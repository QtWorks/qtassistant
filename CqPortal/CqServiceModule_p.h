﻿#ifndef CQSERVICEMODULE_P_H
#define CQSERVICEMODULE_P_H

#include "CqInterface_p.h"
#include "CqServiceModule.h"
#include "CqMessageFilter.h"

namespace CoolQ {

class ServiceModulePrivate : public InterfacePrivate
{
    Q_DECLARE_PUBLIC(ServiceModule)

public:
    ServiceModulePrivate();
    virtual ~ServiceModulePrivate();

protected:
    ServicePortal *portal;

public:
    static ServiceModulePrivate *get(ServiceModule *o) { return o ? o->d_func() : nullptr; }
    static const ServiceModulePrivate *get(const ServiceModule *o) { return o ? o->d_func() : nullptr; }

public:
    void installFilter(MessageFilter *filter);
protected:
    QList<MessageFilter *> filters;

protected:
    QHash<QByteArray, MessageFilter *> privateFilters;
    QHash<QByteArray, MessageFilter *> groupFilters;
    QHash<QByteArray, MessageFilter *> discussFilters;

public:
    static ServiceModule::Result result(qint32 r);

protected:
    int privateMessageEventPriority;
    int groupMessageEventPriority;
    int discussMessageEventPriority;

    int masterChangeEventPriority;
    int friendRequestEventPriority;
    int groupRequestEventPriority;

    int friendAddEventPriority;
    int memberJoinEventPriority;
    int memberLeaveEventPriority;

private:
    qint64  currentId;

protected:
    QString resPath;
    QString basePath;
    QString imagePath;
};

} // namespace CoolQ

#endif // CQSERVICEMODULE_P_H