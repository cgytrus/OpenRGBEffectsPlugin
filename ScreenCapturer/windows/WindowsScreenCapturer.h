#ifndef WINDOWSSCREENCAPTURER_H
#define WINDOWSSCREENCAPTURER_H

#include "ScreenCapturer.h"
#include <thread>
#include <QScreen>
#include <QPixmap>

class WindowsScreenCapturer : public ScreenCapturer
{
public:
    WindowsScreenCapturer();
    ~WindowsScreenCapturer();

    void Start() override;
    void Stop() override;
    void SetScreen(int) override;

private:
    void CaptureThreadFunction();
    std::thread* capture_thread = nullptr;
    bool continue_capture = false;
    QScreen* screen = nullptr;

    QPixmap grabWindow(quintptr window) const;
};

#endif // WINDOWSSCREENCAPTURER_H
