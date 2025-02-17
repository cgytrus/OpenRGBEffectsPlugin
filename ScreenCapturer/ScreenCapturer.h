#ifndef SCREENCAPTURER_H
#define SCREENCAPTURER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QRect>

enum ScreenCapturerError {
    Cancelled,
    Other
};

class ScreenCapturer : public QObject
{
    Q_OBJECT

public:
    ScreenCapturer(): QObject(){};
    ~ScreenCapturer() {};

    void SetFrameRate(unsigned int value) {framerate = value;};

    virtual void Init(const QString& restore_token = "") {};
    virtual void Start() {};
    virtual void Stop() {};
    virtual void SetScreen(int) {};

protected:
    unsigned int framerate = 60;

signals:
    void OnStarted();
    void OnStopped();

    void OnRestoreTokenProvided(const QString& token);
    void OnImage(const QImage& img);
    void OnError(const ScreenCapturerError& err, const QString& message);

};

#endif // SCREENCAPTURER_H
