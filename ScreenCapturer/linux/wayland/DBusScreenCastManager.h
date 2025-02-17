#ifndef DBUSSCREENCASTMANAGER_H
#define DBUSSCREENCASTMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QDBusArgument>
#include "PipeWireStreamInfo.h"

#define PORTAL_SERVICE_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_SERVICE_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_SERVICE_IFACE "org.freedesktop.portal.ScreenCast"
#define PORTAL_REQUEST_IFACE "org.freedesktop.portal.Request"

struct DBusPathToken {
    QString token;
    QString path;
};

class DBusScreenCastManager : public QObject
{
    Q_OBJECT

public:
    DBusScreenCastManager(QObject *parent = nullptr);
    ~DBusScreenCastManager();

    void Start();
    void CreateSession(const QString& restore_token);
    bool Started();
    void ReOpen();


public slots:
    void OnSessionCreated(uint responseCode, QVariantMap results);
    void OnSourceSelected(uint responseCode, QVariantMap results);
    void OnStarted(uint responseCode, QVariantMap results);

signals:
    void OnRestoreTokenAcquired(const QString& restore_token);
    void OnPipeWireStreamOpened(quint32 node_id, int fd, unsigned int width, unsigned int height);
    void OnError(const QDBusError& err, const QString& message);

private:

    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface i = QDBusInterface("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop", "org.freedesktop.portal.ScreenCast", bus, nullptr);

    void SelectSources();
    void OpenPipeWireRemote(const PipeWireStreamInfo&);

    QString session_handle;
    PipeWireStreamInfoList streams;

    DBusPathToken NewRequestPath();
    DBusPathToken NewSessionPath();
    DBusPathToken NewPath(QString type);

    static unsigned long RequestTokenCounter;
    static unsigned long SessionTokenCounter;

    bool started = false;
    QString restore_token;
};

#endif // DBUSSCREENCASTMANAGER_H
