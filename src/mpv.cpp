#include "mpv.h"
#include <QObject>

MpvSingleton::MpvSingleton() {
    mpv = mpv_create();
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    loop = new MpvEventLoop(mpv, nullptr);
    loop->start();

    mpv_set_option_string(mpv, "input-default-bindings", "yes");
    mpv_set_option_string(mpv, "scale", "nearest");
    mpv_set_option_string(mpv, "dscale", "mitchell");
    mpv_set_option_string(mpv, "osc", "yes");
    mpv_set_option_string(mpv, "loop", "yes");
    mpv_set_option_string(mpv, "video-timing-offset", "0");
    mpv_set_option_string(mpv, "script-opts", "osc-idlescreen=no,osc-deadzonesize=1");
    mpv_set_option_string(mpv, "hwdec", "auto");

    mpv_observe_property(mpv, 1, "width", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 2, "height", MPV_FORMAT_INT64);

    mpv_set_option_string(mpv, "osc", "yes");

    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");
}


static void *get_proc_address_mpv(void *ctx, const char *name)
{
    Q_UNUSED(ctx)

    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;

    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

MpvObject::MpvObject(QQuickItem * parent) : QQuickFramebufferObject(parent) { }
MpvObject::Renderer *MpvObject::createRenderer() const {
    return new MpvRenderer(this);
}
void MpvObject::command(const QVariant& params) {
    commands.enqueue({"", params});
    update();
}

void MpvObject::setProperty(const QString& name, const QVariant& value)
{
    commands.enqueue({name, value});
    update();
}

void MpvObject::onSize(int width, int height) {
    setImplicitWidth(width);
    setImplicitHeight(height);
}

MpvEventLoop::MpvEventLoop(mpv_handle* mpv_, QObject* parent = nullptr) : QThread(parent), mpv{mpv_} { }
void MpvEventLoop::run() {
    int width = 0;
    int height = 0;
    while (1) {
        auto e = mpv_wait_event(mpv, -1);
        if(e->event_id == MPV_EVENT_PROPERTY_CHANGE && e->error == 0) {
            auto p = *(mpv_event_property*)(e->data);
            if(p.format != MPV_FORMAT_NONE) {
                if(e->reply_userdata == 1) {
                    width = *(int64_t*)p.data;
                }
                if(e->reply_userdata == 2) {
                    height = *(int64_t*)p.data;
                }

                if(width && height) {
                    emit gotSize(width, height);
                }
            }
        } else if (e->event_id == MPV_EVENT_NONE) {
            break;
        }
    }
}

MpvRenderer::MpvRenderer(const MpvObject* parent) {
    static MpvSingleton instance{};
    mpv = &instance;
    QObject::connect(instance.loop, &MpvEventLoop::gotSize, parent, &MpvObject::onSize, Qt::QueuedConnection);
}
MpvRenderer::~MpvRenderer() { }
QOpenGLFramebufferObject * MpvRenderer::createFramebufferObject(const QSize &size) {
    if (!mpv->mpv_gl) {
        mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr};
        mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };

        if (mpv_render_context_create(&(mpv->mpv_gl), mpv->mpv, params) < 0)
            throw std::runtime_error("failed to initialize mpv GL context");
    }

    return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
}
void MpvRenderer::render() {
    update();
    QOpenGLFramebufferObject *fbo = framebufferObject();
    mpv_opengl_fbo mpfbo{.fbo = static_cast<int>(fbo->handle()), .w = fbo->width(), .h = fbo->height(), .internal_format = 0};
    int flip_y{0};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    mpv_render_context_render(mpv->mpv_gl, params);
}

void MpvRenderer::synchronize(QQuickFramebufferObject *item) {
    auto o = (MpvObject*)item;
    while (!o->commands.isEmpty()) {
        auto c = o->commands.dequeue();
        if(c.name == "") {
            mpv::qt::command_variant_async(mpv->mpv, c.data);
        } else {
            mpv::qt::set_property_variant_async(mpv->mpv, c.name, c.data);
        }
    }
}
