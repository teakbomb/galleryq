#include "storage.h"

#include <QObject>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDriver>
#include <QThread>
#include <QMutex>
#include <QCollator>

#include "sql.h"

Storage::Storage(QObject *parent)
    : QObject{parent}
{ }

void Storage::setup() {
    if(QSqlDatabase::connectionNames().length() != 0) {
        return;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");

    db.setConnectOptions("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE");
    db.setDatabaseName("file::memory:");
    db.open();

    QSqlQuery q(db);

    q.exec("CREATE TABLE files(file INTEGER PRIMARY KEY, parent INTEGER, children INTEGER, path TEXT, type TEXT, tags TEXT);");
    q.exec("CREATE TABLE metadata(file INTEGER PRIMARY KEY, id INTEGER, parent INTEGER);");
    q.exec("CREATE TABLE tags(tag TEXT, file INTEGER, PRIMARY KEY (file, tag));");
    q.exec("CREATE TABLE sources(path TEXT, mode INTEGER);");
    q.exec("CREATE VIRTUAL TABLE search USING fts5(tags, content=files, content_rowid=file, tokenize=\"unicode61 tokenchars '`~!@#$%^&*_+-=\\;'',./{}|:""<>?'\");");

    q.exec("CREATE TABLE children_count(file INTEGER PRIMARY KEY, children INTEGER);");
    q.exec("CREATE TABLE tag_count(tag TEXT PRIMARY KEY, cnt INTEGER);");

    q.exec("CREATE INDEX idx_files_parent ON files(parent);");
    q.exec("CREATE INDEX idx_tags_file ON tags(file);");
    q.exec("CREATE INDEX idx_tags_tag ON tags(tag);");
    q.exec("CREATE INDEX idx_tag_count ON tag_count(tag);");

    q.exec("CREATE TRIGGER files_ai AFTER INSERT ON files BEGIN\
                INSERT INTO search(rowid, tags) VALUES (new.file, new.tags);\
                UPDATE files SET children = children + 1 WHERE file = new.parent;\
            END;");

    q.exec("CREATE TRIGGER files_ad AFTER DELETE ON files BEGIN\
                INSERT INTO search(search, rowid, tags) VALUES('delete', old.file, old.tags);\
                UPDATE files SET children = children - 1 WHERE file = old.parent;\
            END;");

    q.exec("CREATE TRIGGER files_au AFTER UPDATE ON files BEGIN\
                INSERT INTO search(search, rowid, tags) VALUES('delete', old.file, old.tags);\
                INSERT INTO search(rowid, tags) VALUES (new.file, new.tags);\
            END;");

    q.exec("CREATE TRIGGER files_au_c AFTER UPDATE ON files WHEN old.parent <> new.parent BEGIN\
               UPDATE files SET children = children - 1 WHERE file = old.parent;\
               UPDATE files SET children = children + 1 WHERE file = new.parent;\
            END;");

    q.exec("CREATE TRIGGER tags_ai AFTER INSERT ON tags BEGIN\
                INSERT OR IGNORE INTO tag_count(tag, cnt) VALUES (new.tag, 0);\
                UPDATE tag_count SET cnt = cnt + 1 WHERE tag = new.tag;\
            END;");

    q.exec("CREATE TRIGGER tags_ad AFTER DELETE ON files BEGIN\
                UPDATE tag_count SET cnt = cnt - 1 WHERE tag = old.tag;\
            END;");

     db.driver()->subscribeToNotification("files");
     db.driver()->subscribeToNotification("sources");

     QObject::connect(db.driver(), QOverload<const QString &, QSqlDriver::NotificationSource, const QVariant &>::of(&QSqlDriver::notification), this, &Storage::notification);
}

void Storage::doWrite(QSqlQuery q) {
    while(!q.exec()) {
        if(q.lastError().nativeErrorCode() == "6") {
            QThread::msleep(10);
        } else {
            qInfo() << q.lastQuery() << q.lastError();
            break;
        }
    }
}

void Storage::doWrite(QString query) {
    QSqlQuery q(db);
    q.prepare(query);
    doWrite(q);
}

void Storage::addFile(quint64 id, quint64 parent, QString path, QString type, QString tags) {;
    if(tags != "") {
        tags += " any";
    }
    {
        QSqlQuery q(db);
        q.prepare("INSERT INTO files(file, parent, children, path, type, tags) VALUES (:file, :parent, 0, :path, :type, :tags);");
        q.bindValue(":file", id);
        q.bindValue(":parent", parent);
        q.bindValue(":path", path);
        q.bindValue(":type", type);
        q.bindValue(":tags", tags);
        doWrite(q);
    }
    for(QString tag : tags.split(" ")) {
        if(tag != "" && tag != "any") {
            QSqlQuery q(db);
            q.prepare("INSERT INTO tags(tag, file) VALUES (:tag,:file);");
            q.bindValue(":file", id);
            q.bindValue(":tag", tag);
            doWrite(q);
        }
    }
    emit refresh("files", id);
}
void Storage::deleteFile(quint64 id) {
    auto query = QString("DELETE FROM files WHERE file = %1 OR parent = %1;").arg(id);
    doWrite(query);
    emit refresh("files", id);
}

void Storage::cloneFile(quint64 id, quint64 new_id) {
    auto query = QString("INSERT INTO files(file, parent, path, type, tags) SELECT %1,parent,path,type,tags FROM files WHERE file = %2").arg(new_id).arg(id);
    doWrite(query);
    emit refresh("files", id);
}

void Storage::updateFile(quint64 id, quint64 new_parent, bool remove_tags) {
    doWrite(QString("UPDATE files SET parent=%2 WHERE file=%1;").arg(id).arg(new_parent));

    if(remove_tags) {
        doWrite(QString("UPDATE files SET tags='' WHERE file=%1;").arg(id));
        doWrite(QString("DELETE FROM tags WHERE file=%1;").arg(id));
    }
    emit refresh("files", id);
}

void Storage::addMetadata(quint64 id, Metadata m) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO metadata(file, parent) VALUES (:file, :parent);");
    q.bindValue(":file", id);
    q.bindValue(":parent", m.parent);
    doWrite(q);
    emit refresh("metadata", id);
}

void Storage::deleteMetadata(quint64 id) {
    doWrite(QString("DELETE FROM metadata WHERE file=%1;").arg(id));
    emit refresh("metadata", id);
}

void Storage::addSource(QString path, int mode) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO sources(path, mode) VALUES (:path, :mode);");
    q.bindValue(":path", path);
    q.bindValue(":mode", mode);
    doWrite(q);
    emit refresh("source", path);
}
void Storage::deleteSource(QString path) {
    doWrite(QString("DELETE FROM sources WHERE path='%1';").arg(path));
    emit refresh("source", path);
}
void Storage::updateSource(QString old_path, QString new_path, int new_mode) {
    auto query = QString("UPDATE sources SET path='%1', mode=%2 WHERE path='%3';").arg(new_path).arg(new_mode).arg(old_path);
    doWrite(query);
    emit refresh("source", old_path);
}

void Storage::notification(const QString& name, QSqlDriver::NotificationSource source, const QVariant& payload) {
    //emit refresh(name, payload);
}

Reader::Reader() {
    connect();
}

void Reader::connect() {
    static int id = 0;

    const QString name = "db" + QString::number(id++);
    db = QSqlDatabase::database(name);
    if (!db.isOpen() || !db.isValid()) {
       db = QSqlDatabase::cloneDatabase("qt_sql_default_connection", name);
       if (!db.open()) {
          qCritical() << "failed to open db connection" << name;
       }
    }
}

QSqlQuery Reader::doRead(QString query) {
    QSqlQuery q(db);
    while(!q.exec(query)) {
        if(q.lastError().nativeErrorCode() == "6") {
            QThread::msleep(10);
        } else {
            qInfo() << q.lastQuery() << q.lastError();
            break;
        }
    }
    return q;
}

void Reader::dump() {
    auto start = QDateTime::currentMSecsSinceEpoch();
    auto q = doRead("SELECT * FROM (SELECT tag FROM tags WHERE file = 1000000000007) as 'tags' INNER JOIN tag_count ON tags.tag = tag_count.tag;");
    auto end = QDateTime::currentMSecsSinceEpoch();
    qInfo() << end - start << "SPECIAL";
    while(q.next()) {
        //qInfo() << q.record();
    }
}

void Writer::setup() {
    QMetaObject::invokeMethod(Storage::instance(), "setup", Qt::QueuedConnection);

    while(QSqlDatabase::connectionNames().length() == 0) {
        QThread::msleep(10);
    }
}

void Writer::addFile(quint64 id, quint64 parent, QString path, QString type, QString tags) {
    QMetaObject::invokeMethod(Storage::instance(), "addFile", Qt::QueuedConnection,
                              Q_ARG(quint64, id), Q_ARG(quint64, parent),
                              Q_ARG(QString, path), Q_ARG(QString, type),
                              Q_ARG(QString, tags));
}
void Writer::deleteFile(quint64 id)  {
    QMetaObject::invokeMethod(Storage::instance(), "deleteFile", Qt::QueuedConnection,
                              Q_ARG(quint64, id));
}
void Writer::cloneFile(quint64 id, quint64 new_id) {
    QMetaObject::invokeMethod(Storage::instance(), "cloneFile", Qt::QueuedConnection,
                              Q_ARG(quint64, id), Q_ARG(quint64, new_id));
}
void Writer::updateFile(quint64 id, quint64 new_parent, bool remove_metadata) {
    QMetaObject::invokeMethod(Storage::instance(), "updateFile", Qt::QueuedConnection,
                              Q_ARG(quint64, id), Q_ARG(quint64, new_parent),
                              Q_ARG(bool, remove_metadata));
}
void Writer::addMetadata(quint64 id, Metadata m) {
    QMetaObject::invokeMethod(Storage::instance(), "addMetadata", Qt::QueuedConnection,
                              Q_ARG(quint64, id), Q_ARG(Metadata, m));
}
void Writer::deleteMetadata(quint64 id) {
    QMetaObject::invokeMethod(Storage::instance(), "deleteMetadata", Qt::QueuedConnection,
                              Q_ARG(quint64, id));
}
void Writer::addSource(QString path, int mode) {
    QMetaObject::invokeMethod(Storage::instance(), "addSource", Qt::QueuedConnection,
                              Q_ARG(QString, path), Q_ARG(int, mode));
}
void Writer::deleteSource(QString path) {
    QMetaObject::invokeMethod(Storage::instance(), "deleteSource", Qt::QueuedConnection,
                              Q_ARG(QString, path));
}
void Writer::updateSource(QString old_path, QString new_path, int new_mode) {
    QMetaObject::invokeMethod(Storage::instance(), "updateSource", Qt::QueuedConnection,
                              Q_ARG(QString, old_path), Q_ARG(QString, new_path),
                              Q_ARG(int, new_mode));
}
