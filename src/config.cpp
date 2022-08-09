#include "config.h"

#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

Config::Config(QString path) : configPath(path) {
    setDefault();
    config = QVariantMap();

    QFile in(path);

    if(!in.exists())
        return;

    in.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray data = in.readAll();
    in.close();

    auto doc = QJsonDocument::fromJson(data);
    if(!doc.isObject())
        return;

    config = doc.object().toVariantMap();
}
QVariant Config::getValue(QString key) {
    if(config.contains(key)) {
        return config[key];
    }
    if(defaultConfig.contains(key)) {
        return defaultConfig[key];
    }
    return QVariant();
}
void Config::putValue(QString key, QVariant type) {
    config[key] = type;
    save();
}

void Config::save() {
    auto doc = QJsonDocument();
    doc.setObject(QJsonObject::fromVariantMap(config));

    auto data = doc.toJson();

    auto configDir = QFileInfo(configPath).absoluteDir().absolutePath();
    QDir().mkpath(configDir);

    QFile out(configPath);
    out.open(QIODevice::WriteOnly|QIODevice::Text);
    out.write(data);
    out.close();
}

void Config::setDefault() {
    defaultConfig = QVariantMap();
    defaultConfig["thumbnailStore"] = "./thumbs/";
}
















