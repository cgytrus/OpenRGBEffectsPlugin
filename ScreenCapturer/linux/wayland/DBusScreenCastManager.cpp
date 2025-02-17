#include "DBusScreenCastManager.h"
#include <stdio.h>
#include <math.h>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QDBusArgument>
#include <QDBusUnixFileDescriptor>
#include <QDBusMetaType>
#include <QMetaType>
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#include <spa/debug/types.h>
#include <spa/param/video/type-info.h>
#include <fcntl.h>
#include <spa/debug/format.h>
#include <spa/debug/types.h>
#include <spa/utils/result.h>
#include <spa/pod/dynamic.h>

// docs https://github.com/flatpak/xdg-desktop-portal/blob/main/data/org.freedesktop.portal.ScreenCast.xml

unsigned long DBusScreenCastManager::RequestTokenCounter = 0;
unsigned long DBusScreenCastManager::SessionTokenCounter = 0;

DBusScreenCastManager::DBusScreenCastManager(QObject* parent) : QObject(parent){}

DBusScreenCastManager::~DBusScreenCastManager(){}

DBusPathToken DBusScreenCastManager::NewPath(QString type)
{
    QString token = "u" + QString::number(++RequestTokenCounter);
    QString path = "/org/freedesktop/portal/desktop/" + type + "/" + i.objectName() + "/" + token;
    return {token, path};
}

DBusPathToken DBusScreenCastManager::NewRequestPath()
{
    return NewPath("request");
}

DBusPathToken DBusScreenCastManager::NewSessionPath()
{
    return NewPath("session");
}

void DBusScreenCastManager::CreateSession(const QString& restore_token)
{
    qDebug() << "CreateSession with token" << (restore_token.isEmpty() ? "NOT PROVIDED" : restore_token);

    this->restore_token = restore_token;

    DBusPathToken new_session_path = NewSessionPath();
    DBusPathToken new_request_path = NewRequestPath();

    QVariantMap options;
    options["handle_token"] = new_request_path.token;
    options["session_handle_token"] = new_session_path.token;

    qDebug() << "session options " << options;

    QDBusReply<QDBusObjectPath> repl = i.call("CreateSession", options);

    if(repl.isValid())
    {
        qDebug() << "CreateSession() valid : " << repl.value().path();
        bool con = bus.connect("", repl.value().path(), "org.freedesktop.portal.Request", "Response", this, SLOT(OnSessionCreated(uint, QVariantMap)));
        qDebug() << "session con" << con;
    }
    else
    {
        emit OnError(repl.error(), "Cannot create session");
    }
}

void DBusScreenCastManager::OnSessionCreated(uint responseCode, QVariantMap results)
{
    if(responseCode == 0)
    {
        qDebug() << "OnSessionCreated() " << results;
        session_handle = results["session_handle"].toString();
        qDebug() << "session handle = " << session_handle;
        SelectSources();
    }
    else
    {
        emit OnError(bus.lastError(), "Cannot create session");
    }
}

void DBusScreenCastManager::SelectSources()
{
    QVariantMap options;
    DBusPathToken new_request_path = NewRequestPath();
    options["handle_token"] = new_request_path.token;
    options["multiple"] = false;
    options["types"] = quint32(1|2);
    options["persist_mode"] = quint32(2);

    if(!restore_token.isEmpty())
    {
        options["restore_token"] = restore_token;
    }

    qDebug() << options;

    QDBusReply<QDBusObjectPath> repl = i.call("SelectSources", QDBusObjectPath(session_handle), options);

    if(repl.isValid())
    {
        qDebug() << "SelectSources() valid " << repl.value().path();
        bus.connect("", repl.value().path(), "org.freedesktop.portal.Request", "Response", this, SLOT(OnSourceSelected(uint, QVariantMap)));
    }
    else
    {
        qDebug() << "SelectSources() Something is wrong: " << repl.error();
    }
}

void DBusScreenCastManager::OnSourceSelected(uint responseCode, QVariantMap results) {
    if(responseCode == 0)
    {
        qDebug() << "OnSourceSelected() " << results;

        if(!restore_token.isEmpty())
        {
            Start();
        }
    }
    else
    {
        qDebug() << "OnSourceSelected() response code " << responseCode;
    }
}

void DBusScreenCastManager::Start()
{
    QVariantMap options;
    DBusPathToken new_request_path = NewRequestPath();
    options["handle_token"] = new_request_path.token;

    qDebug() << options;

    QDBusReply<QDBusObjectPath> repl = i.call("Start", QDBusObjectPath(session_handle), "", options);

    if(repl.isValid())
    {
        qDebug() << "Start() valid " << repl.value().path();
        bool con=bus.connect("", repl.value().path(), "org.freedesktop.portal.Request", "Response", this, SLOT(OnStarted(uint, QVariantMap)));
        qDebug() << con;
    }
    else
    {
        qDebug() << "Start() Something is wrong: " << repl.error();
    }
}

void DBusScreenCastManager::ReOpen()
{
    if(streams.size() == 1)
    {
        OpenPipeWireRemote(streams[0]);
    }
}

bool DBusScreenCastManager::Started()
{
    return started;
}

void DBusScreenCastManager::OnStarted(uint responseCode, QVariantMap results) {
    streams.clear();

    if(responseCode == 0)
    {
        started = true;

        restore_token = results["restore_token"].toString();
        emit OnRestoreTokenAcquired(restore_token);

        qDebug() << "OnStarted()";
        qDebug() << "restore_token" << restore_token;

        streams = qdbus_cast<PipeWireStreamInfoList>(results["streams"].value<QDBusArgument>()) ;

        qDebug() << "Stream infos:";

        for(PipeWireStreamInfo stream: streams)
        {
            qDebug() << "----------";
            qDebug() << "Node id" << stream.node_id;
            qDebug() << "Width" << stream.width << "Height" << stream.height;
            qDebug() << "X" << stream.x << "y" << stream.y;
            qDebug() << "Source type " << stream.source_type;
            qDebug() << "----------";
        }

    }
    else
    {
        qDebug() << "OnStarted() response code " << responseCode;
    }

    if(streams.size() == 1)
    {
        OpenPipeWireRemote(streams[0]);
    }
}

void DBusScreenCastManager::OpenPipeWireRemote(const PipeWireStreamInfo& stream_info)
{
    QVariantMap options;

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                                          QStringLiteral("org.freedesktop.portal.ScreenCast"),
                                                          QStringLiteral("OpenPipeWireRemote"));

    message <<  QDBusObjectPath(session_handle) << options;

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);

    connect(watcher, &QDBusPendingCallWatcher::finished, [&] (QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusUnixFileDescriptor> reply = *watcher;

        if (reply.isError())
        {
            qDebug() << "Couldn't get reply";
            qDebug() << "Error: " << reply.error().message();
        }
        else
        {
            qDebug() << "no errors";
            qDebug() << reply.isValid();
            qDebug() << reply.isFinished();
            int fd = reply.value().fileDescriptor();
            qDebug() << "fd " << fd;

            emit OnPipeWireStreamOpened(stream_info.node_id, fd, stream_info.width , stream_info.height);
        }
    });
}


