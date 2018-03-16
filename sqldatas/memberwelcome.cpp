﻿#include "memberwelcome.h"
#include "memberwelcome_p.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QReadWriteLock>

Q_LOGGING_CATEGORY(qlcMemberWelcome, "Welcome")

// class MemberWelcome

MemberWelcome::MemberWelcome(QObject *parent)
    : CoolQ::SqliteService(*new MemberWelcomePrivate(), parent)
{
    Q_D(MemberWelcome);

    setFileName(QStringLiteral("Welcome.db"));

    do {
        const char sql[] = "CREATE TABLE IF NOT EXISTS [Welcome] ("
                           "[gid] INT8 NOT NULL, "
                           "[uid] INT8 NOT NULL, "
                           "[stamp] INT8 NOT NULL, "
                           "PRIMARY KEY ([gid], [uid]));";
        prepare(QString::fromLatin1(sql));
    } while (false);

    if (openDatabase()) {
        do {
            const char sql[] = "SELECT * FROM [Welcome];";
            QSqlQuery query = this->query(sql);
            while (query.next()) {
                qint64 gid = query.value(0).toLongLong();
                qint64 uid = query.value(1).toLongLong();
                qint64 stamp = query.value(2).toLongLong();
                d->welcome.insert(CoolQ::Member(gid, uid), stamp);
            }
        } while (false);
    }
}

MemberWelcome::~MemberWelcome()
{
}

CoolQ::SqliteService::Result MemberWelcome::addMember(qint64 gid, qint64 uid)
{
    Q_D(MemberWelcome);
    QWriteLocker locker(&d->guard);

    CoolQ::Member member(gid, uid);
    if (!d->welcome.contains(member)) {
        qint64 stamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
        const char sql[] = "REPLACE INTO [Welcome] VALUES(%1, %2, %3);";
        QString qtSql = QString::fromLatin1(sql).arg(gid).arg(uid).arg(stamp);
        QSqlQuery query = this->query(qtSql);
        if (query.lastError().isValid()) {
            qCCritical(qlcMemberWelcome, "Update error: %s",
                       qPrintable(query.lastError().text()));
            return SqlError;
        }
        d->welcome.insert(member, stamp);
        qCInfo(qlcMemberWelcome, "Update: gid: %lld, uid: %lld.", gid, uid);

        return Done;
    }

    return NoChange;
}

CoolQ::SqliteService::Result MemberWelcome::removeMember(qint64 gid, qint64 uid)
{
    Q_D(MemberWelcome);
    QWriteLocker locker(&d->guard);

    CoolQ::Member member(gid, uid);
    if (d->welcome.contains(member)) {
        const char sql[] = "DELETE FROM [Welcome] WHERE [gid] = %1 AND [uid] = %2;";
        QString qtSql = QString::fromLatin1(sql).arg(gid).arg(uid);
        QSqlQuery query = this->query(qtSql);
        if (query.lastError().isValid()) {
            qCCritical(qlcMemberWelcome, "Delete error: %s",
                       qPrintable(query.lastError().text()));
            return SqlError;
        }
        d->welcome.remove(member);
        qCInfo(qlcMemberWelcome, "Delete: gid: %lld, uid: %lld.", gid, uid);

        return Done;
    }

    return NoChange;
}

QHash<CoolQ::Member, qint64> MemberWelcome::members() const
{
    Q_D(const MemberWelcome);

    return d->welcome;
}

void MemberWelcome::expiredMembers(CoolQ::MemberList &members)
{
    Q_D(MemberWelcome);
    QWriteLocker locker(&d->guard);

    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QMutableHashIterator<CoolQ::Member, qint64> iter(d->welcome);
    while (iter.hasNext()) {
        iter.next();
        if ((iter.value() + 1800000) < now) {
            members << iter.key();
            iter.remove();
        }
    }
}

// class MemberWelcomePrivate

MemberWelcomePrivate::MemberWelcomePrivate()
{
}

MemberWelcomePrivate::~MemberWelcomePrivate()
{
}
