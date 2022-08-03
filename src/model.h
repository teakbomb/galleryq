#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QMimeDatabase>
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
    void load();
    bool loadSingle(QString path);
    void traverseFlat(QString path, quint64 parent);
    void traverseCollection(QString path, quint64 parent);
    Metadata parseMetadata(QString path);
    QString getType(QString path);
    void mergeSiblings();
};

#endif // MODEL_H
