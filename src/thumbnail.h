#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QQuickImageProvider>
#include <QThreadPool>
#include <QHash>
#include <QMutex>

class ThumbnailStore {
public:
    void put(QString path, QImage img) {
        guard.lock();
        cache[path] = img;
        guard.unlock();
    }
    bool have(QString path) {
        guard.lock();
        auto i = cache.contains(path);
        guard.unlock();
        return i;
    }
    QImage get(QString path) {
        guard.lock();
        auto i = cache[path];
        guard.unlock();
        return i;
    }
private:
    QMutex guard;
    QHash<QString, QImage> cache;
};

class ThumbnailResponseRunnable : public QObject, public QRunnable
{
    Q_OBJECT
signals:
    void done(QImage image);

public:
    ThumbnailResponseRunnable(const QString &path, const QString &cached, const QSize &size);
    void run() override;

private:
    QString m_path;
    QString m_cached;
    QSize m_size;
};

class ThumbnailResponse : public QQuickImageResponse
{
public:
    ThumbnailResponse(const QString &path, const QString &cached, const QSize &size, QThreadPool *pool);
    void handleDone(QImage image);
    QQuickTextureFactory *textureFactory() const override;
    QImage m_image;
};

class ThumbnailProvider : public QQuickAsyncImageProvider
{
public:
    ThumbnailProvider(const QString &thumbnail_store);
    QQuickImageResponse *requestImageResponse(const QString &path, const QSize &size) override;

private:
    QString m_store;
    QThreadPool pool;
    QString checksum(QString s);
};

#endif // THUMBNAIL_H
