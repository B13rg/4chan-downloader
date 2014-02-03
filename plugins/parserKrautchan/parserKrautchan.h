﻿#ifndef PARSERKRAUTCHAN_H
#define PARSERKRAUTCHAN_H

#include <QStringList>
#include <QList>
#include <QRegExp>
#include <QtDebug>
#include "../../gui/ParserPluginInterface.h"
#include "../../gui/structs.h"

class ParserKrautchan : public QObject, public ParserPluginInterface {
    Q_OBJECT
    Q_INTERFACES(ParserPluginInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "ParserKrautchan" FILE "parserKrautchan.json")
#endif

public:
    ParserKrautchan();
    QString getAuthor(void);
    QString getPluginName(void);
    QString getVersion();
    QString getDomain();
    QString getInterfaceRevision() {return _PARSER_PLUGIN_INTERFACE_REVISION;}
    ParsingStatus parseHTML(QString html);
    void setURL(QUrl url);
    QString getThreadTitle();
    QList<_IMAGE> getImageList();
    QList<QUrl> getUrlList();
    QUrl getRedirectURL();
    int getErrorCode();
    ParsingStatus getStatusCode();
    QString parseSavepath(QString s);
    QMap<QString, QString> getSupportedReplaceCharacters();

    void initPlugin();
    QList<QUrl> initialRequests();
    QUrl alterUrl(QUrl u) {return u;}

    QObject* createInstance() {return new ParserKrautchan();}

private:
    QString _html;
    QString boardName;
    QString threadNumber;
    QList<_IMAGE> _images;
    QList<QUrl> _urlList;
    QUrl _redirect;
    QUrl _url;
    ParsingStatus _statusCode;
    int _errorCode;
    QString _threadTitle;
};

#endif // PARSERKRAUTCHAN_H
