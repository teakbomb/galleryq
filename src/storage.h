#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlDriver>

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

private:
    QSqlDatabase db;
    void doWrite(QSqlQuery q);
    void doWrite(QString query);
signals:
    void refresh(QString name, QVariant payload);
public slots:
    void setup();

    void addFile(quint64 id, quint64 parent, QString path, QString type, QString tags);
    void deleteFile(quint64 id);
    void cloneFile(quint64 id, quint64 new_id);
    void updateFile(quint64 id, quint64 new_parent, bool remove_metadata);

    void addMetadata(quint64 id, Metadata m);
    void deleteMetadata(quint64 id);

    void addSource(QString path, int mode);
    void deleteSource(QString path);
    void updateSource(QString old_path, QString new_path, int new_mode);
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

    static void addFile(quint64 id, quint64 parent, QString path, QString type, QString tags);
    static void deleteFile(quint64 id);
    static void cloneFile(quint64 id, quint64 new_id);
    static void updateFile(quint64 id, quint64 new_parent, bool remove_metadata);

    static void addMetadata(quint64 id, Metadata m);
    static void deleteMetadata(quint64 id);

    static void addSource(QString path, int mode);
    static void deleteSource(QString path);
    static void updateSource(QString old_path, QString new_path, int new_mode);
};


#endif // STORAGE_H
