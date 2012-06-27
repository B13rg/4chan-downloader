#include "uiimageviewer.h"
#include "ui_uiimageviewer.h"

UIImageViewer::UIImageViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UIImageViewer)
{
    ui->setupUi(this);

    imagesToDisplay.clear();
    currentImage = -1;
    rotation = 0;
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    loadSettings();
}

UIImageViewer::~UIImageViewer()
{
    delete ui;
}

void UIImageViewer::setImageList(QStringList imageList) {
    if (imageList.count() > 0) {
        imagesToDisplay = imageList;
        if (currentImage == -1) {
            currentImage = 0;
        }

//        loadImage(currentImage);
    }
    else {
        imagesToDisplay.clear();
        currentImage = -1;
    }
}

void UIImageViewer::displayNextImage() {
    if (imagesToDisplay.count() > 0) {
        currentImage++;
        if (currentImage >= imagesToDisplay.count())
            currentImage = 0;
        loadImage(currentImage);
    }
}

void UIImageViewer::displayPrevImage() {
    if (imagesToDisplay.count() > 0) {
        currentImage--;
        if (currentImage < 0)
            currentImage = imagesToDisplay.count()-1;
        loadImage(currentImage);
    }
}

void UIImageViewer::loadImage(int i) {
    QFile f;
    QPixmap p;
    QString filename;

    if (!this->isVisible()) show();

    qDebug() << "Loading image " << currentImage << "from" << imagesToDisplay.count();
    rotation = 0;

    if (imagesToDisplay.count() > i) {
        filename = imagesToDisplay.value(i);

        if (f.exists(filename)) {
            ui->statusbar->showMessage("Working...");
            if (ui->image->movie() != 0) ui->image->movie()->stop();

            if (filename.endsWith(".gif")) {
                // Always assume gifs are animated -> therefore use QMovie for playback
                QMovie *movie = new QMovie(filename);
                ui->image->setMovie(movie);
                movie->start();
                ui->statusbar->showMessage("Loaded image" + filename, 2000);
                ui->lCurrentImage->setText(QString("%1/%2").arg(currentImage+1).arg(imagesToDisplay.count()));
            }
            else {
                if (p.load(filename)) {
                    originalPixmap = p;
                    transformPixmap();
                    fitImage();
                    ui->statusbar->showMessage("Loaded image" + filename, 2000);
                    ui->lCurrentImage->setText(QString("%1/%2").arg(currentImage+1).arg(imagesToDisplay.count()));
                }
                else {
                    qDebug() << "Error loading image" << filename;
                }
            }
        }
    }
}

void UIImageViewer::setCurrentImage(int i) {
    if (i < imagesToDisplay.count() && i >= 0) {
        currentImage = i;
        loadImage(currentImage);
    }
}

void UIImageViewer::setCurrentImage(QString filename) {
    for (int i=0; i<imagesToDisplay.count(); i++) {
        if (imagesToDisplay.value(i) == filename) {
            currentImage = i;
            loadImage(i);
            break;
        }
    }
}

void UIImageViewer::fitImage() {
    QPixmap p;

    p = QPixmap(2,2);
    ui->image->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->image->setPixmap(p);
//    ui->image->repaint();
//    ui->scrollArea->ensureWidgetVisible(ui->image);
    qDebug() << "\nScrollWidgetSize: " << ui->scrollArea->size() << "\npixmapSize: "<<originalPixmap.size();

    if (ui->btnFitImage->isChecked()) {
        //ui->image->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if (transformedPixmap.size().height() <= ui->scrollArea->size().height() &&
            transformedPixmap.size().width() <= ui->scrollArea->size().width()) {
//            qDebug() << "Not scaling image";
            ui->image->setPixmap(transformedPixmap);
        }
        else {
//            ui->image->setPixmap(originalPixmap.scaled(ui->scrollAreaWidgetContents->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
//            qDebug() << "Scaling image to " << ui->scrollArea->size();
            ui->image->setPixmap(transformedPixmap.scaled(ui->scrollArea->size()-QSize(5,5), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    else {
        ui->image->setPixmap(transformedPixmap);
    }

//    qDebug() << "PixmapLabel Size: " << ui->image->pixmap()->size();
}

void UIImageViewer::openWithExternalViewer() {
    QUrl url;
    if (currentImage != -1 && imagesToDisplay.count() > currentImage) {
        url.setUrl(QString("file:///%1").arg(imagesToDisplay.at(currentImage)));
        QDesktopServices::openUrl(url);
    }
}

void UIImageViewer::loadSettings() {
    // Restore window position
    QPoint p;
    QSize s;
    int state;

    settings->beginGroup("imageviewer");
        p = settings->value("position",QPoint(0,0)).toPoint();
        state = settings->value("state",0).toInt();
        s = settings->value("size",QSize(0,0)).toSize();
        ui->btnFitImage->setChecked(settings->value("fit_image", false).toBool());
    settings->endGroup();

    if (p != QPoint(0,0))
        this->move(p);

    if (s != QSize(0,0))
        this->resize(s);

    if (state != Qt::WindowNoState)
        this->setWindowState((Qt::WindowState) state);
}

void UIImageViewer::saveSettings() {
    settings->beginGroup("imageviewer");
        settings->setValue("position", this->pos());
        if (this->windowState() == Qt::WindowNoState)
            settings->setValue("size", this->size());
        settings->setValue("state", QString("%1").arg(this->windowState()));
        settings->setValue("fit_image", ui->btnFitImage->isChecked());
    settings->endGroup();

    settings->sync();
}

void UIImageViewer::transformPixmap() {
    int rot;
    QTransform t;

    rot = rotation % 360;
    if (rot != 0) {
        qDebug() << "Rotating " << (qreal)rot << "deg (rotation:"<<rotation<<")";
        transformedPixmap = originalPixmap.transformed(t.rotate((qreal)rot));
    }
    else {
        transformedPixmap = originalPixmap;
    }
}

void UIImageViewer::rotateCW() {
    rotation += 90;
    transformPixmap();
    fitImage();
}

void UIImageViewer::rotateCCW() {
    rotation -= 90;
    transformPixmap();
    fitImage();
}