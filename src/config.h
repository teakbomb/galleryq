#ifndef CONFIG_H
#define CONFIG_H

#include <QVariantMap>

class Config
{
public:
    Config(QString path);
    static Config* instance(QString path = nullptr)
    {
        static Config i{path};
        return &i;
    }
    QVariant getValue(QString key);
    void putValue(QString key, QVariant type);
private:
    QString configPath;
    QVariantMap config;
    QVariantMap defaultConfig;
    void setDefault();
    void save();
};

#endif // CONFIG_H
