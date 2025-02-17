#ifndef QTSCREENCAPTURER_H
#define QTSCREENCAPTURER_H

#include "ScreenCapturer.h"
#include <thread>
#include <QScreen>

class QtScreenCapturer : public ScreenCapturer
{
public:
    QtScreenCapturer();
    ~QtScreenCapturer();

    void Start() override;
    void Stop() override;
    void SetScreen(int) override;

private:
    void CaptureThreadFunction();
    std::thread* capture_thread = nullptr;
    bool continue_capture = false;
    QScreen* screen = nullptr;
};

#endif // QTSCREENCAPTURER_H
