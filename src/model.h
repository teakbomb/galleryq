#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QMimeDatabase>
#include <QHash>
#include <QDebug>
#include <QMutex>
#include "storage.h"

class Model
{
public:
    enum MODE {
        NONE,
        COLLECTION,
        FLAT,
    };
    Model();
    QMimeDatabase mime_db;
    Reader reader;
    bool load(qint32 source);
    void mergeSiblings(qint32 source);
    void traverseFlat(qint32 id, QString path, qint32 parent);
    void traverseCollection(qint32 id, QString path, qint32 parent);
    Metadata parseMetadata(QString path);
    QString getType(QString path);
};

#endif // MODEL_H
