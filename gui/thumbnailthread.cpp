#include "thumbnailthread.h"

ThumbnailThread::ThumbnailThread(QObject *parent) :
    QThread(parent)
{
    iconSize = new QSize(100,100);
    hq = true;
    newImages = false;
    settings = new QSettings("settings.ini", QSettings::IniFormat);
}

void ThumbnailThread::createThumbnails() {
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        start(LowPriority);
    } else {
        condition.wakeOne();
    }
}

void ThumbnailThread::run() {
    QImage original, tn;
    QString cacheFile;
    QString currentFilename;
    QDir dir;
    QList<UIImageOverview*> uis;
    int iconWidth;
    int iconHeight;
    bool enlargeThumbnails;
    bool hqRendering;

    forever {
        mutex.lock();
            if (list.count()>0) {
                currentFilename = list.front();
                list.pop_front();
                newImages = true;
                canceled = false;
            }
        mutex.unlock();
        iconWidth = iconSize->width();
        iconHeight = iconSize->height();
        enlargeThumbnails = settings->value("options/enlarge_thumbnails", false).toBool();
        hqRendering = settings->value("options/hq_thumbnails", false).toBool();
//        useCache = settings->value("options/use_thumbnail_cache", true).toBool();
        useCache = true;
        cacheFolder = settings->value("options/thumbnail_cache_folder", QString("%1/%2").arg(QCoreApplication::applicationDirPath())
                                      .arg("tncache")).toString();

//        QLOG_ALWAYS() << "ThumbnailThread :: Using thumbnail folder " << cacheFolder;
        if (useCache && !(dir.exists(cacheFolder))) {
            QLOG_TRACE() << "ThumbnailThread :: Creating thumbnail cache folder " << cacheFolder;
            dir.mkpath(cacheFolder);
        }

        if (newImages) {
            bool useCachedThumbnail;

            useCachedThumbnail = false;
            cacheFile = getCacheFile(currentFilename);
            // Check if thumbnail exists
            if (useCache && QFile::exists(cacheFile)) {
                QLOG_TRACE() << "ThumbnailThread :: Cached thumbnail available for " << currentFilename;
                tn.load(cacheFile);

                if (tn.width() == iconWidth || tn.height() == iconHeight) {
                    useCachedThumbnail = true;
                }
            }

            if (!useCachedThumbnail){
                QLOG_TRACE() << "ThumbnailThread :: Creating new thumbnail for " << currentFilename;
                original.load(currentFilename);

                if (original.width()<iconWidth
                    && original.height()<iconHeight
                    && !(enlargeThumbnails)) {
                    tn = original;
                } else {
                    QLOG_TRACE() << "ThumbnailThread :: Rendering thumbnail";
                    if (hqRendering) {
                        tn = original.scaled(*iconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
                    }
                    else {
                        tn = original.scaled(*iconSize,Qt::KeepAspectRatio,Qt::FastTransformation);
                    }
                }

                tn.save(cacheFile, "PNG");
            }

            uis = callingUIs.values(currentFilename);

            foreach(UIImageOverview* ui, uis) {
                if (!canceled) {
                    ui->addThumbnail(currentFilename, cacheFile);
                }
                callingUIs.remove(currentFilename, ui);
            }
//            emit thumbnailCreated(currentFilename, tn);

            mutex.lock();
                emit pendingThumbnails(list.count());
                newImages = false;

                if (list.count() == 0)
                    condition.wait(&mutex);
            mutex.unlock();
        }
        else {
            mutex.lock();
                condition.wait(&mutex);
            mutex.unlock();
        }
    }
}

void ThumbnailThread::setIconSize(QSize s) {
    mutex.lock();
    iconSize->setWidth(s.width());
    iconSize->setHeight(s.height());
    mutex.unlock();
}

void ThumbnailThread::addToList(UIImageOverview* caller, QString s) {
    mutex.lock();
    list.append(s);
    callingUIs.insertMulti(s, caller);
    newImages = true;
    mutex.unlock();
}

QString ThumbnailThread::getCacheFile(QString filename) {
    QString tmp, ret;

    tmp = filename;
    tmp.replace( QRegExp( "[" + QRegExp::escape( "\\/:*?\"<>|" ) + "]" ), QString( "_" ) );
    ret= QString("%1/%2.tn").arg(cacheFolder).arg(tmp);

    return ret;
}

bool ThumbnailThread::cancelAll(UIImageOverview *caller) {
    bool ret;
    QList<QString> filenames;

    mutex.lock();
    ret = true;
    filenames = callingUIs.keys(caller);
    foreach(QString key, filenames)
        callingUIs.remove(key, caller);

    mutex.unlock();

    return ret;
}
