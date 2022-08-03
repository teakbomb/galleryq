#include "thumbnail.h"
#include <QDebug>
#include <QImage>
#include <QFile>
#include <QCryptographicHash>
#include <QProcess>
#include <QDir>

ThumbnailResponseRunnable::ThumbnailResponseRunnable(const QString &path, const QString &cached, const QSize &size) : m_path(QUrl(path).path()), m_cached(cached), m_size(size) {}

void ThumbnailResponseRunnable::run() {
    static ThumbnailStore store;

    /*if(store.have(m_cached)) {
        auto image = store.get(m_cached);
        emit done(image);
        return;
    }*/

    //qInfo() << m_cached;

    if(QFile::exists(m_cached)) {
        auto image = QImage(m_cached);
        //store.put(m_cached, image);
        emit done(image);
    } else {
        auto image = QImage();
        auto valid = image.load(m_path);
        if(valid) {
            if((image.size().width() > m_size.width() || image.size().height() > m_size.height())) {
                image = image.scaled(m_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
            emit done(image);
            image.save(m_cached);
            //store.put(m_cached, image);
        } else {
            QProcess *p = new QProcess();
            QStringList args;
            args << "-ss" << "00:00:00.00" << "-i" << m_path << "-vf" << QString("scale=%1:%2:force_original_aspect_ratio=decrease").arg(m_size.width()).arg(m_size.height()) << "-vframes" << "1" << m_cached;
            p->start("ffmpeg", args);
            p->waitForFinished();

            auto image = QImage(m_cached);
            emit done(image);
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

ThumbnailProvider::ThumbnailProvider(const QString &thumbnail_store) : m_store(thumbnail_store) {
    QDir(".").mkdir(thumbnail_store);
}

QQuickImageResponse* ThumbnailProvider::requestImageResponse(const QString &path, const QSize &size) {
    QString cached = QString("%1%2_%3x%4.jpg").arg(m_store).arg(checksum(path)).arg(size.width()).arg(size.height());
    ThumbnailResponse *response = new ThumbnailResponse(path, cached, size, &pool);
    return response;
}


QString ThumbnailProvider::checksum(QString s) {
    return QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Md5).toHex();
}
