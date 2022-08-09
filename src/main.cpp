#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "model.h"
#include "thumbnail.h"
#include "mpv.h"
#include "util.h"
#include "sql.h"
#include "sql.h"
#include "storage.h"

#include <QDateTime>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                        if (!obj && url == objUrl)
                            QCoreApplication::exit(-1);
                    }, Qt::QueuedConnection);
    QQmlContext *ctx = engine.rootContext();
    qmlRegisterSingletonType(QUrl("qrc:///Constants.qml"), "gallery.constants", 1, 0, "Constants");
    std::setlocale(LC_NUMERIC, "C");
    qmlRegisterType<MpvObject>("gallery.mpv", 1, 0, "MpvObject");

    QThread *thread = new QThread();
    Storage::instance()->moveToThread(thread);
    thread->start();
    Writer::setup();

    Config::instance(QDir::homePath() + QFileInfo("/.config/galleryq/config.json").filePath());
    for(int i = 1; i < argc; i++) {
        qInfo() << argv[i];
        Writer::addSource(Storage::UUID(), argv[i], Model::COLLECTION);
    }

    Model m{};
    ThumbnailProvider* thumbs = new ThumbnailProvider();
    Util util{&m, thumbs};
    Util::instance(&util);

    ctx->setContextProperty("util", &util);
    engine.addImageProvider("thumbnail", thumbs);

    engine.load(url);
    return app.exec();
}
