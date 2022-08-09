#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QMutex>

struct Metadata {
    quint64 id;
    quint64 parent;
    QString tags;
};
Q_DECLARE_METATYPE(Metadata)

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent = nullptr);

    static Storage* instance()
    {
        static Storage i;
        return &i;
    }
    enum STATUS {
        UNKNOWN,
        UNLOADED,
        LOADING,
        LOADED,
    };
    static qint32 ID(QString data) {
        return (qint32)(qHash(data) >> 1);
    }
    static qint32 UUID() {
        static qint32 static_id = 1000000000;
        static QMutex guard;

        guard.lock();
        auto id = static_id++;
        guard.unlock();

        return id;
    }
private:
    QSqlDatabase db;
    void doWrite(QSqlQuery q);
    void doWrite(QString query);
signals:
    void refresh(QString name, QVariant payload);
public slots:
    void setup();

    void addFile(qint32 file, QString path, QString type);
    void deleteFile(qint32 file);

    void addNode(qint32 node, qint32 parent, qint32 source, qint32 file, QString tags);
    void deleteNode(qint32 node);
    void cloneNode(qint32 node, qint32 new_node);
    void moveNode(qint32 node, qint32 new_parent);
    void stripNode(qint32 node);

    void addMetadata(qint32 node, Metadata m);
    void deleteMetadata(qint32 node);

    void addSource(qint32 source, QString path, int mode);
    void deleteSource(qint32 source);
    void updateSource(qint32 source, QString path, int mode);
    void statusSource(qint32 source, int status);
private slots:
    void notification(const QString& name, QSqlDriver::NotificationSource source, const QVariant& payload);
};


class Reader
{
public:
    Reader();
    QSqlDatabase db;
    void connect();
    QSqlQuery doRead(QString q);
    void dump();
};

class Writer
{
public:
    static void setup();

    static void addFile(qint32 file, QString path, QString type, bool blocking = false);
    static void deleteFile(qint32 file, bool blocking = false);

    static void addNode(qint32 node, qint32 parent, qint32 source, qint32 file, QString tags, bool blocking = false);
    static void deleteNode(qint32 node, bool blocking = false);
    static void cloneNode(qint32 node, qint32 new_node, bool blocking = false);
    static void moveNode(qint32 node, qint32 new_parent, bool blocking = false);
    static void stripNode(qint32 node, bool blocking = false);

    static void addMetadata(qint32 node, Metadata m, bool blocking = false);
    static void deleteMetadata(qint32 node, bool blocking = false);

    static void addSource(qint32 source, QString path, int mode, bool blocking = false);
    static void deleteSource(qint32 source, bool blocking = false);
    static void updateSource(qint32 source, QString path, int mode, bool blocking = false);
    static void statusSource(qint32 source, int status, bool blocking = false);
};


#endif // STORAGE_H
