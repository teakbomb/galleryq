#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include <QtQuick/QQuickFramebufferObject>
#include <QtGui/QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QQueue>
#include <QThread>

#include <mpv/client.h>
#include <mpv/render_gl.h>
#include "mpv_qthelper.h"

static void on_mpv_redraw(void *);
static void *get_proc_address_mpv(void *ctx, const char *name);

class MpvSingleton;

class MpvObject : public QQuickFramebufferObject
{
    Q_OBJECT
    friend class MpvRenderer;

    typedef struct {
        QString name;
        QVariant data;
    } Command;

    QQueue<Command> commands;
public:
    MpvObject(QQuickItem * parent = 0);
    Renderer *createRenderer() const;
public slots:
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    void onSize(int,int);
};

class MpvEventLoop : public QThread {
    Q_OBJECT
    mpv_handle* mpv;
public:
    MpvEventLoop(mpv_handle* mpv_, QObject* parent);
    void run();
signals:
    void gotSize(int,int);
};

class MpvRenderer : public QQuickFramebufferObject::Renderer
{
    int width = 0;
    int height = 0;
public:
    MpvSingleton *mpv;
    MpvRenderer(const MpvObject* parent);
    ~MpvRenderer();
    QOpenGLFramebufferObject * createFramebufferObject(const QSize &size);
    void render();
    void synchronize(QQuickFramebufferObject *item);
};

class MpvSingleton {
public:
   mpv_handle* mpv;
   mpv_render_context *mpv_gl;
   MpvEventLoop* loop;
   MpvSingleton();
};

#endif
