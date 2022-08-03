#ifndef UTIL_H
#define UTIL_H

#include <QObject>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>
#include "model.h"
#include "thumbnail.h"


class LoaderWorker : public QObject {
    Q_OBJECT
public:
    LoaderWorker(QString p) {
        path = p;
        model = new Model{};
    }
    ~LoaderWorker() {}
public slots:
    void process() {
        auto start = QDateTime::currentMSecsSinceEpoch();
        if(!model->loadSingle(path)) {
            emit error("no such source");
        }
        auto end = QDateTime::currentMSecsSinceEpoch();
        qInfo() << end-start << path;
        emit finished();
    }
signals:
    void finished();
    void error(QString err);
private:
    QString path;
    Model* model;
};

class Util : public QObject {
    Q_OBJECT
public:
    Util(Model* m, ThumbnailProvider *t) {
        model = m;
        thumbs = t;
    }
    Q_INVOKABLE void openFolder(QString file) {
        QDir d = QFileInfo(file).absoluteDir();
        QString absolute=d.absolutePath();
        QDesktopServices::openUrl(absolute);
    }

    Q_INVOKABLE void deleteFolder(QString file) {
        QDir d = QFileInfo(file).absoluteDir();
        d.removeRecursively();
    }
    Q_INVOKABLE void deleteFile(QVariant file) {
        Writer::deleteFile(file.toULongLong());
    }
    Q_INVOKABLE QString processQuery(QString query) {
        static auto token_reg = QRegularExpression("([^\\s\\[\\]]+)");
        query.replace(token_reg, "\"\\1\"");
        query.replace("\"OR\"", "OR", Qt::CaseInsensitive);
        query.replace("\"AND\"", "AND", Qt::CaseInsensitive);
        query.replace("\"NOT\"", "NOT", Qt::CaseInsensitive);

        static auto multispace_reg = QRegularExpression("\\s+");
        query.replace(multispace_reg, " ");

        static auto and_reg = QRegularExpression("([\"\\]\\[])\\s([\"\\]\\[])");
        query.replace(and_reg, "\\1 AND \\2");

        query.replace("[", "(").replace("]", ")");

        static auto not_reg = QRegularExpression("\"-([\\w_]+)\"");
        query.replace(not_reg, "(any NOT \"\\1\")");

        return query;
    }

    Q_INVOKABLE void newSource() {
        Writer::addSource(QDir::currentPath(), Model::COLLECTION);
    }

    Q_INVOKABLE bool updateSource(QString oldPath, QString newPath, int mode) {
        QDir dir(newPath);
        if (!dir.exists()) {
            return false;
        }

        qInfo() << oldPath << newPath << mode;

        Writer::updateSource(oldPath, newPath, (Model::MODE)mode);
        return true;
    }

    Q_INVOKABLE void startLoad(QString path) {
        QThread* thread = new QThread();
        LoaderWorker* worker = new LoaderWorker(path);
        worker->moveToThread(thread);
        //connect( worker, &LoaderWorker::error, this, &LoaderWorker::errorString);
        connect( thread, &QThread::started, worker, &LoaderWorker::process);
        connect( worker, &LoaderWorker::finished, thread, &QThread::quit);
        connect( worker, &LoaderWorker::finished, worker, &LoaderWorker::deleteLater);
        connect( thread, &QThread::finished, thread, &QThread::deleteLater);
        thread->start();
    }

    Q_INVOKABLE void startFullLoad() {
        qInfo() << "full load";
        //Reader r{};
        auto q = model->reader.doRead("SELECT path FROM sources;");
        qInfo() << q.lastError();
        while (q.next()) {
            auto path = q.value(0).toString();
            qInfo() << "load" << path;
            startLoad(path);
        }
    }
    Q_INVOKABLE void dump() {
        model->reader.dump();
    }

private:
    Model* model;
    ThumbnailProvider* thumbs;
};

#endif // UTIL_H
