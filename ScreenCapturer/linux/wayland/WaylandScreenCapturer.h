#ifndef WAYLANDSCREENCAPTURER_H
#define WAYLANDSCREENCAPTURER_H

#include "ScreenCapturer.h"
#include "DBusScreenCastManager.h"
#include "PipeWireCapturer.h"

class WaylandScreenCapturer : public ScreenCapturer
{
public:
    WaylandScreenCapturer(QObject* parent = nullptr);
    ~WaylandScreenCapturer();

    void Init(const QString& restore_token = "",  bool auto_start = false) override;
    void SetToken(const QString& restore_token = "") override;
    void Start() override;
    void Stop() override;
    void SetScreen(int) override;

private:
    DBusScreenCastManager *dbus_manager = nullptr;
    PipeWireCapturer* capturer = nullptr;
};

#endif // WAYLANDSCREENCAPTURER_H
