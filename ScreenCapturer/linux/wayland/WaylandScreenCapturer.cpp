#include "WaylandScreenCapturer.h"

WaylandScreenCapturer::WaylandScreenCapturer(): ScreenCapturer()
{
    dbus_manager = new DBusScreenCastManager();
    capturer = new PipeWireCapturer();

    connect(capturer, &PipeWireCapturer::NewImage, this, [&](const QImage& img){
        emit OnImage(img);
    });

    connect(dbus_manager, &DBusScreenCastManager::OnRestoreTokenAcquired,[&](const QString& token){
        emit OnRestoreTokenProvided(token);
    });

    connect(dbus_manager, &DBusScreenCastManager::OnPipeWireStreamOpened,[&](quint32 node_id, int fd, unsigned int width, unsigned int height){
        capturer->ReadStream(node_id, fd, width, height);        
    });    

    connect(dbus_manager, &DBusScreenCastManager::OnError,[=](const QDBusError& err, const QString& message){
        emit OnError(Other, message);
    });
}

WaylandScreenCapturer::~WaylandScreenCapturer()
{
    delete capturer;
    delete dbus_manager;
}

void WaylandScreenCapturer::Start()
{
    if(!dbus_manager->Started())
    {
        dbus_manager->Start();
    }
    else
    {
        dbus_manager->ReOpen();
    }
}

void WaylandScreenCapturer::Stop()
{
    capturer->Stop();
}

void WaylandScreenCapturer::Init(const QString& restore_token)
{
    dbus_manager->CreateSession(restore_token);

}

void WaylandScreenCapturer::SetScreen(int)
{
    // not supported
}
