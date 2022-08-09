#include "thumbnail.h"
#include <QDebug>
#include <QImage>
#include <QFile>
#include <QCryptographicHash>
#include <QProcess>
#include <QDir>

#include "config.h"

ThumbnailResponseRunnable::ThumbnailResponseRunnable(const QString &path, const QString &cached, const QSize &size) : m_path(QUrl(path).path()), m_cached(cached), m_size(size) {}

void ThumbnailResponseRunnable::run() {
    createThumbnail(m_path, m_cached, m_size);
    auto image = QImage(m_cached);
    emit done(image);
}

void ThumbnailResponseRunnable::createThumbnail(const QString &path, const QString &thumbnail, const QSize &size) {
    QDir().mkdir(QFileInfo(thumbnail).absoluteDir().absolutePath());

    if(QFile::exists(thumbnail)) {
        return;
    } else {
        auto image = QImage();
        auto valid = image.load(path);
        if(valid) {
            if((image.size().width() > size.width() || image.size().height() > size.height())) {
                image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            image.save(thumbnail);
            return;
        } else {
            QProcess *p = new QProcess();
            QStringList args;
            args << "-ss" << "00:00:00.00" << "-i" << path << "-vf" << QString("scale=%1:%2:force_original_aspect_ratio=decrease").arg(size.width()).arg(size.height()) << "-vframes" << "1" << thumbnail;
            p->start("ffmpeg", args);
            p->waitForFinished();
            return;
        }
    }
}

ThumbnailResponse::ThumbnailResponse(const QString &path, const QString &cached, const QSize &size, QThreadPool *pool) {
    auto runnable = new ThumbnailResponseRunnable(path, cached, size);
    connect(runnable, &ThumbnailResponseRunnable::done, this, &ThumbnailResponse::handleDone);
    pool->start(runnable);
}

void ThumbnailResponse::handleDone(QImage image) {
    m_image = image;
    emit finished();
}

QQuickTextureFactory* ThumbnailResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

ThumbnailProvider::ThumbnailProvider() { }

QQuickImageResponse* ThumbnailProvider::requestImageResponse(const QString &path, const QSize &size) {
    QString cached = getThumbnailPath(path, size);
    ThumbnailResponse *response = new ThumbnailResponse(path, cached, size, &pool);
    return response;
}

QString ThumbnailProvider::getThumbnailPath(QString path, QSize size) {
    return QString("%1%2_%3x%4.jpg").arg(Config::instance()->getValue("thumbnailStore").toString()).arg(checksum(path)).arg(size.width()).arg(size.height());
}

QString ThumbnailProvider::checksum(QString s) {
    return QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Md5).toHex();
}
