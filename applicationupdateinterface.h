#ifndef APPLICATIONUPDATEINTERFACE_H
#define APPLICATIONUPDATEINTERFACE_H

#include <QObject>
#include <QByteArray>
#include <QtNetwork/QUdpSocket>
#include <QDir>
#include <QMessageBox>
#include "updater/commands.h"
#include "defines.h"

class ApplicationUpdateInterface : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationUpdateInterface(QObject *parent = 0);
    void startUpdate(QString);
private:
    QUdpSocket* udpSocket;
    bool pinging;
    bool answerPing;
    bool connected;
    bool startRequest;
    QString version;

    void processCommand(QByteArray a);
    void writeCommand(int c, QByteArray a);
    void writeCommand(int c, QString s);
    void writeCommand(int c);

private slots:
    void readPendingDatagrams(void);
    void init();

signals:
    void connectionEstablished();

public slots:

};

#endif // APPLICATIONUPDATEINTERFACE_H
