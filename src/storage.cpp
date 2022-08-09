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
#include <QFileInfo>
#include <QDateTime>

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

    q.exec("CREATE TABLE files(file INT, path TEXT UNIQUE, type TEXT);");
    q.exec("CREATE TABLE sources(source INT, path TEXT, mode INT, status INT, saved INT);");

    q.exec("CREATE TABLE nodes(node INT, parent INT, children INT, source INT, file INT, tags TEXT);");
    q.exec("CREATE TABLE metadata(node INT, id INT, parent INT);");
    q.exec("CREATE TABLE tags(node INT, tag TEXT, PRIMARY KEY (node, tag));");
    q.exec("CREATE TABLE tag_count(tag TEXT PRIMARY KEY, cnt INT);");

    q.exec("CREATE VIRTUAL TABLE search USING fts5(tags, content=nodes, content_rowid=node, tokenize=\"unicode61 tokenchars '`~!@#$%^&*_+-=\\;'',./{}|:""<>?'\");");

    /*q.exec("CREATE INDEX idx_files_parent ON files(parent);");
    q.exec("CREATE INDEX idx_tags_file ON tags(file);");
    q.exec("CREATE INDEX idx_tags_tag ON tags(tag);");
    q.exec("CREATE INDEX idx_tag_count ON tag_count(tag);");*/

    q.exec("CREATE TRIGGER nodes_ai AFTER INSERT ON nodes BEGIN\
                INSERT INTO search(rowid, tags) VALUES (new.node, new.tags);\
                UPDATE nodes SET children = children + 1 WHERE node = new.parent;\
            END;");

    q.exec("CREATE TRIGGER nodes_ad AFTER DELETE ON nodes BEGIN\
                INSERT INTO search(search, rowid, tags) VALUES('delete', old.node, old.tags);\
                UPDATE nodes SET children = children - 1 WHERE node = old.parent;\
            END;");

    q.exec("CREATE TRIGGER nodes_au AFTER UPDATE ON nodes BEGIN\
                INSERT INTO search(search, rowid, tags) VALUES('delete', old.node, old.tags);\
                INSERT INTO search(rowid, tags) VALUES (new.node, new.tags);\
            END;");

    q.exec("CREATE TRIGGER nodes_au_children AFTER UPDATE OF parent ON nodes BEGIN\
               UPDATE nodes SET children = children - 1 WHERE node = old.parent;\
               UPDATE nodes SET children = children + 1 WHERE node = new.parent;\
            END;");

    q.exec("CREATE TRIGGER tags_ai AFTER INSERT ON tags BEGIN\
                INSERT OR IGNORE INTO tag_count(tag, cnt) VALUES (new.tag, 0);\
                UPDATE tag_count SET cnt = cnt + 1 WHERE tag = new.tag;\
            END;");

    q.exec("CREATE TRIGGER tags_ad AFTER DELETE ON tags BEGIN\
                UPDATE tag_count SET cnt = cnt - 1 WHERE tag = old.tag;\
            END;");

     //db.driver()->subscribeToNotification("files");
     //db.driver()->subscribeToNotification("sources");

     QObject::connect(db.driver(), QOverload<const QString &, QSqlDriver::NotificationSource, const QVariant &>::of(&QSqlDriver::notification), this, &Storage::notification);
}

void Storage::doWrite(QSqlQuery q) {
    static QMutex guard;
    guard.lock();
    while(!q.exec()) {
        if(q.lastError().nativeErrorCode() == "6") {
            QThread::msleep(10);
        } else {
            qInfo() << q.lastQuery() << q.boundValues() << q.lastError();
            break;
        }
    }
    guard.unlock();
}

void Storage::doWrite(QString query) {
    QSqlQuery q(db);
    q.prepare(query);
    doWrite(q);
}

void Storage::addFile(qint32 file, QString path, QString type) {
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO files(file, path, type) VALUES (:file, :path, :type);");
    q.bindValue(":file", file);
    q.bindValue(":path", path);
    q.bindValue(":type", type);
    doWrite(q);
    emit refresh("files", file);
}
void Storage::deleteFile(qint32 file) {
    auto query = QString("DELETE FROM files WHERE file = %1;").arg(file);
    doWrite(query);
    emit refresh("files", file);
}
void Storage::addNode(qint32 node, qint32 parent, qint32 source, qint32 file, QString tags) {
    if(tags != "") {
        tags += " any";
    }
    {
        QSqlQuery q(db);
        q.prepare("INSERT OR REPLACE INTO nodes(node, parent, children, source, file, tags) VALUES (:node, :parent, 0, :source, :file, :tags);");
        q.bindValue(":node", node);
        q.bindValue(":parent", parent);
        q.bindValue(":source", source);
        q.bindValue(":file", file);
        q.bindValue(":tags", tags);
        doWrite(q);
    }
    for(QString tag : tags.split(" ")) {
        if(tag != "" && tag != "any") {
            QSqlQuery q(db);
            q.prepare("INSERT OR IGNORE INTO tags(node, tag) VALUES (:node, :tag);");
            q.bindValue(":node", node);
            q.bindValue(":tag", tag);
            doWrite(q);
        }
    }
    emit refresh("nodes", node);
}
void Storage::deleteNode(qint32 node) {
    auto query = QString("DELETE FROM nodes WHERE node = %1;").arg(node);
    doWrite(query);
    emit refresh("nodes", node);
}
void Storage::cloneNode(qint32 node, qint32 new_node) {
    auto query = QString("INSERT INTO nodes(node, parent, children, source, file, tags) SELECT %1, parent, 0, source, file, tags FROM nodes WHERE node = %2;").arg(new_node).arg(node);
    doWrite(query);
    emit refresh("nodes", node);
}
void Storage::moveNode(qint32 node, qint32 new_parent) {
    doWrite(QString("UPDATE nodes SET parent=%2 WHERE node=%1;").arg(node).arg(new_parent));
    emit refresh("nodes", node);
}
void Storage::stripNode(qint32 node) {
    doWrite(QString("UPDATE nodes SET tags='' WHERE node=%1;").arg(node));
    doWrite(QString("DELETE FROM tags WHERE node=%1;").arg(node));
    emit refresh("nodes", node);
}
void Storage::addMetadata(qint32 node, Metadata m) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO metadata(node, id, parent) VALUES (:node, :id, :parent);");
    q.bindValue(":node", node);
    q.bindValue(":id", m.id);
    q.bindValue(":parent", m.parent);
    doWrite(q);
    emit refresh("metadata", node);
}
void Storage::deleteMetadata(qint32 node) {
    doWrite(QString("DELETE FROM metadata WHERE node=%1;").arg(node));
    emit refresh("metadata", node);
}

void Storage::addSource(qint32 source, QString path, int mode) {
    QSqlQuery q(db);
    q.prepare("INSERT INTO sources(source, path, mode, status) VALUES (:source, :path, :mode, 1);");
    q.bindValue(":source", source);
    q.bindValue(":path", path);
    q.bindValue(":mode", mode);
    doWrite(q);
    emit refresh("source", source);
}
void Storage::deleteSource(qint32 source) {
    doWrite(QString("DELETE FROM nodes WHERE source=%1;").arg(source));
    emit refresh("nodes", source);
    doWrite(QString("DELETE FROM sources WHERE source=%1;").arg(source));
    emit refresh("source", source);
}
void Storage::updateSource(qint32 source, QString path, int mode) {
    doWrite(QString("DELETE FROM nodes WHERE source=%1;").arg(source));
    auto query = QString("UPDATE sources SET path='%1', mode=%2, status=1 WHERE source=%3;").arg(path).arg(mode).arg(source);
    doWrite(query);
    emit refresh("source", source);
}
void Storage::statusSource(qint32 source, int status) {
    auto query = QString("UPDATE sources SET status=%1 WHERE source=%2;").arg(status).arg(source);
    doWrite(query);
    emit refresh("source", source);
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
    if(query == "")
        return q;
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
    auto query = "SELECT * FROM nodes INNER JOIN files ON nodes.file = files.file WHERE parent = 0 ORDER BY node;";
    auto q = doRead(query);
    if(q.next()) {
        qInfo() << q.record();
    }
    query = "SELECT nodes.*, files.* FROM (SELECT rowid FROM search WHERE tags MATCH '\"loli\" ') as 't' JOIN nodes ON t.rowid = nodes.node JOIN files ON nodes.file = files.file";
    q = doRead(query);
    if(q.next()) {
        qInfo() << q.record();
    }
}

void Writer::setup() {
    QMetaObject::invokeMethod(Storage::instance(), "setup", Qt::BlockingQueuedConnection);

    while(QSqlDatabase::connectionNames().length() == 0) {
        QThread::msleep(10);
    }
}

void Writer::addFile(qint32 file, QString path, QString type, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "addFile", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, file), Q_ARG(QString, path),
                              Q_ARG(QString, type));
}
void Writer::deleteFile(qint32 file, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "deleteFile", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, file));
}
void Writer::addNode(qint32 node, qint32 parent, qint32 source, qint32 file, QString tags, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "addNode", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node), Q_ARG(qint32, parent),
                              Q_ARG(qint32, source), Q_ARG(qint32, file),
                              Q_ARG(QString, tags));
}
void Writer::deleteNode(qint32 node, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "deleteNode", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node));
}
void Writer::cloneNode(qint32 node, qint32 new_node, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "cloneNode", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node), Q_ARG(qint32, new_node));
}
void Writer::moveNode(qint32 node, qint32 new_parent, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "moveNode", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node), Q_ARG(qint32, new_parent));
}
void Writer::stripNode(qint32 node, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "stripNode", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node));
}
void Writer::addMetadata(qint32 node, Metadata m, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "addMetadata", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node), Q_ARG(Metadata, m));
}
void Writer::deleteMetadata(qint32 node, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "deleteMetadata", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, node));
}
void Writer::addSource(qint32 source, QString path, int mode, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "addSource", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, source), Q_ARG(QString, path),
                              Q_ARG(int, mode));
}
void Writer::deleteSource(qint32 source, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "deleteSource", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, source));
}
void Writer::updateSource(qint32 source, QString path, int mode, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "updateSource", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, source), Q_ARG(QString, path),
                              Q_ARG(int, mode));
}
void Writer::statusSource(qint32 source, int status, bool blocking) {
    QMetaObject::invokeMethod(Storage::instance(), "statusSource", blocking ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
                              Q_ARG(qint32, source), Q_ARG(int, status));
}
