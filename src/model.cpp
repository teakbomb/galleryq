#include "model.h"
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDir>
#include <QDirIterator>
#include <QCollator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QThread>
#include <QMutex>
#include <QSqlDriver>
#include <QtGui>

#include <QMetaObject>
#include "sql.h"
#include "storage.h"

quint64 get_id() {
    static QMutex guard;
    static quint64 static_id = 1000000000000ULL;

    guard.lock();
    auto id = static_id++;
    guard.unlock();

    return id;
}

Model::Model()
{
    reader = Reader{};
}

QString Model::getType(QString file) {
    QMimeType mime = mime_db.mimeTypeForFile(file);
    auto name = mime.name();

    if(name.contains("svg")) {
        return "";
    }
    if(name.contains("image")) {
        return name.contains("gif") ? "video" : "image";
    } else if (name.contains("video")) {
        return "video";
    } else if(name.contains("json")) {
        return "json";
    }

    return "";
}

Metadata Model::parseMetadata(QString path) {
    static QStringList tagsAlias = {"tags", "tag_string"};
    static QStringList idAlias = {"id", "gid"};
    static QStringList parentAlias = {"parent_id"};

    Metadata m{.id = 0, .parent=0, .tags=""};

    QFile in(path);

    if(!in.exists())
        return m;

    in.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray data = in.readAll();
    in.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
        return m;

    auto o = doc.object();

    QStringList tags;



    for(QString alias : tagsAlias) {
        if(o.contains(alias)) {
            auto v = o.value(alias);
            if(v.isString()) {
                auto t = v.toString().toLower().replace(",", " ");
                tags.append(t.split(" "));
            }
            if(v.isArray()) {
                foreach(QJsonValue tag, v.toArray()){
                    tags.append(QUrl::fromPercentEncoding(tag.toString().toLatin1()).toLower().replace(" ", "_"));
                }
            }
        }
    }

    tags.removeDuplicates();
    tags.removeAll("");
    tags.removeAll(" ");
    m.tags = tags.join(" ").replace("[", "(").replace("]", ")").replace("-", "_");

    QTextDocument text;
    text.setHtml(m.tags);
    m.tags = text.toPlainText().replace("'", "");

    for(QString alias : idAlias) {
        if(o.contains(alias) && o.value(alias).isDouble()) {
            m.id = o.value(alias).toDouble();
        }
    }

    for(QString alias : parentAlias) {
        if(o.contains(alias) && o.value(alias).isDouble()) {
            m.parent = o.value(alias).toDouble();
        }
    }

    return m;
}

void Model::traverseFlat(QString path, quint64 parent = 0) {
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Time);
    auto list = dir.entryInfoList();

    QFileInfoList jsonFiles;
    QFileInfoList mediaFiles;
    QHash<QString, QFileInfo> namesMap;

    for(QFileInfo item : list) {
        namesMap[item.fileName()] = item;
        if(item.isFile()) {
            auto path = item.absoluteFilePath();
            auto type = getType(path);
            if(type == "json") {
                jsonFiles.append(item);
            } else if(type != "") {
                mediaFiles.append(item);
            }
        }
    }

    QFileInfoList looseMediaFiles;

    for(QFileInfo item : mediaFiles) {
        auto path = item.absoluteFilePath();
        auto json = item.fileName() + ".json";

        if(!namesMap.contains(json)) {
            looseMediaFiles.append(item);
            continue;
        } else {
            auto jsonItem = namesMap[json];

            Metadata m = parseMetadata(jsonItem.absoluteFilePath());
            jsonFiles.removeAll(jsonItem);
            namesMap.remove(json);

            auto id = get_id();

            Writer::addFile(id, parent, path, getType(path), m.tags);
            Writer::addMetadata(id, m);
        }
    }

    for(QFileInfo item : looseMediaFiles) {
        Metadata m = parseMetadata("");
        auto f = item.absoluteFilePath();
        auto id = get_id();
        Writer::addFile(id, parent, f, getType(f), m.tags);
    }
}

void Model::traverseCollection(QString path, quint64 parent = 0) {
    //any media file named like a sub-directory will become the parent of that sub-directory
    //  ex: /site1.png, /site1/
    //      becomes a file with site1.png as cover, and all files found in /site1/ as children
    //any json file named like a media file will be parsed for metadata and added to that file
    //  ex: /site1.png.json, /site1.png
    //      becomes a file with site1.png as cover, site1.png.json as metadata
    //all loose media files (without an associated json or a directory) will be combined into a file and added to the parent
    //  the first loose media file will be the cover of this collection
    //  the first loose json file will be parsed for metadata and added to the new collection
    //  ex: /0.png, /1.png, /2.png, /metadata.json
    //      becomes a file with 0.png as cover, metadata.json as metadata, and 0.png, 1.png, 2.png as children
    //  ex: /0.png, /tags.json
    //      becomes a file with 0.png as cover, tags.json as metadata

    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Time);
    auto list = dir.entryInfoList();

    QCollator collator;
    collator.setNumericMode(true);

    std::sort(
        list.begin(), list.end(), [&](const QFileInfo &file1, const QFileInfo &file2) {
            return collator.compare(file1.fileName(), file2.fileName()) < 0;
        }
    );

    QFileInfoList jsonFiles;
    QFileInfoList mediaFiles;
    QFileInfoList subDirectories;
    QHash<QString, QFileInfo> namesMap;

    for(QFileInfo item : list) {
        namesMap[item.fileName()] = item;
        if(item.isFile()) {
            auto path = item.absoluteFilePath();
            auto type = getType(path);
            if(type == "json") {
                jsonFiles.append(item);
            } else if(type != "") {
                mediaFiles.append(item);
            }
        }
        if(item.isDir()) {
            subDirectories.append(item);
        }
    }

    QFileInfoList looseMediaFiles;

    for(QFileInfo item : mediaFiles) {
        auto path = item.absoluteFilePath();
        auto json = item.fileName() + ".json";
        auto dir = item.baseName();

        if(!namesMap.contains(json) && !namesMap.contains(dir)) {
            looseMediaFiles.append(item);
            continue;
        }

        Metadata m = parseMetadata("");
        auto id = get_id();

        if(namesMap.contains(json)) {
            auto jsonItem = namesMap[json];

            m = parseMetadata(jsonItem.absoluteFilePath());

            jsonFiles.removeAll(jsonItem);
            namesMap.remove(json);
        }

        Writer::addFile(id, parent, path, getType(path), m.tags);
        Writer::addMetadata(id, m);

        if(namesMap.contains(dir)) {
            auto dirItem = namesMap[dir];
            traverseCollection(dirItem.absoluteFilePath(), id);
            subDirectories.removeAll(dirItem);
            namesMap.remove(dir);
        }
    }

    if(looseMediaFiles.length() > 0) {
        Metadata m = parseMetadata(jsonFiles.length() ? jsonFiles[0].absoluteFilePath() : "");
        auto id = get_id();

        auto f = looseMediaFiles[0].absoluteFilePath();
        Writer::addFile(id, parent, f, getType(f), m.tags);
        Writer::addMetadata(id, m);

        if(looseMediaFiles.length() > 1) {
            for(QFileInfo item : looseMediaFiles) {
                auto f = item.absoluteFilePath();
                auto child = get_id();
                Writer::addFile(child, id, f, getType(f), "");
            }
        }
    }

    for(QFileInfo item : subDirectories) {
        traverseCollection(item.absoluteFilePath(), parent);
    }
}

bool Model::loadSingle(QString path) {
    QSqlQuery q(QString("SELECT mode FROM sources WHERE path = '%1';").arg(path), reader.db);
    if(!q.next()) {
        return false;
    }
    auto mode = (MODE)q.value(0).toInt();

    switch(mode) {
    case FLAT:
        traverseFlat(path, 0);
        break;
    case COLLECTION:
        traverseCollection(path, 0);
        break;
    default:
        break;
    }

    mergeSiblings();

    return true;
}

void Model::mergeSiblings() {
    auto q = reader.doRead("SELECT parent, COUNT(*) as 'occurances' FROM metadata WHERE parent != 0 GROUP BY parent ORDER BY occurances;");
    while (q.next()) {
        auto parent = q.value(0).toULongLong();
        auto count = q.value(1).toUInt();
        if(count == 1)
            continue;
        quint64 new_root = 0;

        auto p = reader.doRead(QString("SELECT * FROM files WHERE file = %1;").arg(parent));
        if(p.next()) { //parent already exists
            new_root = parent;
        }

        auto s = reader.doRead(QString("SELECT * FROM metadata WHERE parent = %1 ORDER BY file;").arg(parent));

        while (s.next()) {
            auto f = s.value(0).toULongLong();
            if(!new_root) {
                new_root = get_id();
                Writer::cloneFile(f, new_root);
            }
            Writer::updateFile(f, new_root, true);
        }
    }
}
