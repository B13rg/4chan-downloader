#ifndef THUMBNAILREMOVERTHREAD_H
#define THUMBNAILREMOVERTHREAD_H

#include <QThread>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QString>
#include <QMutex>
#include <QCoreApplication>
#include <QFileInfoList>
#include <QtDebug>
#include <QDateTime>

class ThumbnailRemoverThread : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailRemoverThread(QObject *parent = 0);
private:
    QSettings* settings;
    int ttl;
    QString dirName;
    QDir dir;
    QMutex mutex;
    QFileInfoList fileInfoList;
protected:
    void run();

signals:

public slots:
    void removeFiles(QStringList fileList);
    void removeAll();

};

#endif // THUMBNAILREMOVERTHREAD_H