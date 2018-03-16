﻿#ifndef MEMBERDEATHHOUSE_H
#define MEMBERDEATHHOUSE_H

#if _MSC_VER >= 1600
#  pragma execution_character_set("utf-8")
#endif

#include <QHash>

#include "CqSqliteService.h"

class MemberDeathHousePrivate;
class MemberDeathHouse : public CoolQ::SqliteService
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MemberDeathHouse)

public:
    explicit MemberDeathHouse(QObject *parent = Q_NULLPTR);
    virtual ~MemberDeathHouse();

public:
    Result addMember(qint64 gid, qint64 uid);
    Result removeMember(qint64 gid, qint64 uid);

public:
    QHash<CoolQ::Member, qint64> members() const;
    void expiredMembers(CoolQ::MemberList &members);
};

#endif // MEMBERDEATHHOUSE_H
