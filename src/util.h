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
#include "config.h"

class LoaderWorker : public QObject {
    Q_OBJECT
public:
    LoaderWorker(qint32 s) {
        source = s;
        model = new Model{};
    }
    ~LoaderWorker() {}
public slots:
    void process() {
        auto start = QDateTime::currentMSecsSinceEpoch();
        if(!model->load(source)) {
            emit error("no such source");
        }
        auto end = QDateTime::currentMSecsSinceEpoch();
        qInfo() << end-start << source;

        Writer::statusSource(source, Storage::LOADED);

        emit loadFinished(source);
        emit finished();
    }
signals:
    void finished();
    void error(QString err);
    void loadFinished(qint32);
private:
    qint32 source;
    Model* model;
};

class ThumbnailWorker : public QObject {
    Q_OBJECT
public:
    ThumbnailWorker() {
        reader = new Reader{};
    }
    ~ThumbnailWorker() {}
public slots:
    void process() {
        emit progress(0.0);
        auto q = reader->doRead("SELECT COUNT(*) FROM files;");
        q.next();
        auto count = (double)q.value(0).toInt();

        q = reader->doRead("SELECT path FROM files;");

        double i = 0;
        while (q.next()) {
            auto path = q.value(0).toString();
            auto size = QSize(512,512);
            auto thumb = ThumbnailProvider::getThumbnailPath(path, size);
            ThumbnailResponseRunnable::createThumbnail(path, thumb, size);
            emit progress(++i/count);
        }

        emit progress(0.0);
    }
signals:
    void progress(float progress);
    void finished();
    void error(QString err);
private:
    Reader* reader;
};

class Util : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString thumbnailPath READ getThumbnailPath NOTIFY configChanged)
    Q_PROPERTY(double thumbnailProgress MEMBER thumbnailProgress NOTIFY progressChanged)
    //Q_PROPERTY(bool bilinear MEMBER bi NOTIFY configChanged)
public:
    Util(Model* m, ThumbnailProvider *t) {
        model = m;
        thumbs = t;
    }

    static Util* instance(Util* u = nullptr)
    {
        static Util* i = u;
        return i;
    }

    Q_INVOKABLE void onDrop(QUrl url) {
        if(!url.isLocalFile())
            return;

        auto file = QFileInfo(url.toLocalFile());
        if(!file.isDir())
            return;
        auto path = file.absoluteFilePath();
        auto source = Storage::UUID();
        Writer::addSource(source, path, Model::COLLECTION);
        startLoad(source);
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
        Writer::addSource(Storage::UUID(), QDir::currentPath(), Model::COLLECTION, true);
    }

    Q_INVOKABLE bool updateSource(qint32 source, QString path, int mode) {
        auto q = model->reader.doRead(QString("SELECT path FROM sources WHERE source = %1").arg(source));

        if(!q.next()) {
            return false; //updating a source that doesnt exist?
        }

        auto old = q.value(0).toString();
        q.finish();



        QDir dir(path);
        if (!dir.exists()) {
            return false;
        }

        if(isSourceSaved(old)) {
            toggleSourceSaved(old);
            if(!isSourceSaved(path)) {
                toggleSourceSaved(path);
            }
        }

        Writer::updateSource(source, path, (Model::MODE)mode, true);

        emit configChanged();

        startLoad(source);

        return true;
    }

    Q_INVOKABLE bool isSourceSaved(QString path) {
        qInfo() << path;
        auto v = Config::instance()->getValue("sources");
        auto l = v.toStringList();
        return l.contains(path);
    }

    Q_INVOKABLE void toggleSourceSaved(QString path) {
        auto v = Config::instance()->getValue("sources");
        auto l = v.toStringList();
        if(l.contains(path)) {
            l.removeAll(path);
        } else {
            l.append(path);
        }
        Config::instance()->putValue("sources", l);
        emit configChanged();
    }

    Q_INVOKABLE void deleteSource(qint32 source) {
        auto q = model->reader.doRead(QString("SELECT path FROM sources WHERE source = %1").arg(source));
        if(!q.next()) return;
        auto path = q.value(0).toString();
        q.finish();

        auto v = Config::instance()->getValue("sources");
        auto l = v.toStringList();
        if(l.contains(path)) {
            l.removeAll(path);
            Config::instance()->putValue("sources", l);
        }

        Writer::deleteSource(source);

        emit configChanged();
    }

    Q_INVOKABLE void startLoad(qint32 source) {
        Writer::statusSource(source, Storage::LOADING);
        emit configChanged();

        QThread* thread = new QThread();
        LoaderWorker* worker = new LoaderWorker(source);
        worker->moveToThread(thread);
        connect(thread, &QThread::started, worker, &LoaderWorker::process);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(worker, &LoaderWorker::finished, thread, &QThread::quit);
        connect(worker, &LoaderWorker::finished, worker, &LoaderWorker::deleteLater);
        connect(worker, &LoaderWorker::loadFinished, this, &Util::onLoadFinished);

        thread->start();
    }

    Q_INVOKABLE bool isLoading(qint32 source) {
        auto b = sourceStatus(source) == Storage::LOADING;
        qInfo() << "loading" << b;
        return b;
    }

    Q_INVOKABLE int sourceStatus(qint32 source) {
        auto q = model->reader.doRead(QString("SELECT status FROM sources WHERE source=%1;").arg(source));
        if(!q.next())
            return 0;
        auto status = q.value(0).toInt();
        q.finish();
        return status;
    }

    Q_INVOKABLE void startFullLoad() {
        auto v = Config::instance()->getValue("sources");
        auto l = v.toStringList();
        for(QString path : l) {
            Writer::addSource(Storage::UUID(), path, Model::COLLECTION, true);
        }

        auto q = model->reader.doRead("SELECT source FROM sources;");
        auto s = QList<int>();
        while (q.next()) {
            s.append(q.value(0).toInt());
        }
        q.finish();
        for(int source : s) {
            startLoad(source);
        }
    }
    Q_INVOKABLE void dump() {
        model->reader.dump();
    }

    double thumbnailProgress = 0.0;
    Q_INVOKABLE void generateThumbnails() {
        thumbnailProgress = 0.0;
        emit progressChanged();

        QThread* thread = new QThread();
        ThumbnailWorker* worker = new ThumbnailWorker();
        worker->moveToThread(thread);
        connect(thread, &QThread::started, worker, &ThumbnailWorker::process);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        connect(worker, &ThumbnailWorker::finished, thread, &QThread::quit);
        connect(worker, &ThumbnailWorker::finished, worker, &ThumbnailWorker::deleteLater);
        connect(worker, &ThumbnailWorker::progress, this, &Util::updateThumbnailProgress);        
        thread->start();
    }

    Q_INVOKABLE void deleteThumbnails() {
        auto path = getThumbnailPath();
        QDir dir(path);
        qInfo() << dir.absolutePath();
        dir.removeRecursively();
    }

     Q_INVOKABLE bool setThumbnailPath(QString path) {
        QDir dir;

        if(!dir.mkpath(path))
            return false;

        Config::instance()->putValue("thumbnailStore", path);
        emit configChanged();
        return true;
    }

    QString getThumbnailPath() {
        return Config::instance()->getValue("thumbnailStore").toString();
    }
private:
    Model* model;
    ThumbnailProvider* thumbs;
    QString configFile;
    QList<qint32> loading;
private slots:
    void updateThumbnailProgress(double progress) {
        thumbnailProgress = progress;
        emit progressChanged();
    }
    void onLoadFinished(qint32 source) {
        emit configChanged();
    }
signals:
    void configChanged();
    void progressChanged();
};

#endif // UTIL_H
