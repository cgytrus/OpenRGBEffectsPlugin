#ifndef PIPEWIRESTREAMINFO_H
#define PIPEWIRESTREAMINFO_H

#include <QString>
#include <QList>
#include <QMetaType>
#include <QtDBus>
#include <QDebug>

typedef struct PipeWireStreamInfo {
    quint32 node_id = 0;
    quint32 width = 0;
    quint32 height = 0;
    quint32 x = 0;
    quint32 y = 0;
    quint32 source_type = 0;
} PipeWireStreamInfo;

typedef QList<PipeWireStreamInfo> PipeWireStreamInfoList;

Q_DECLARE_METATYPE(PipeWireStreamInfoList)

inline void registerCommType()
{
    qDebug() << "registering";
    qDBusRegisterMetaType<PipeWireStreamInfoList>();
}

QDBusArgument &operator<<(QDBusArgument &arg, const PipeWireStreamInfoList& reply);
const QDBusArgument &operator>>(const QDBusArgument &arg, PipeWireStreamInfoList& reply);

#endif // PIPEWIRESTREAMINFO_H
