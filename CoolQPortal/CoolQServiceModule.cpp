﻿/*!
 * \class CoolQ::ServiceModule
 * \brief 服务模块类
 */

/*!
 * \enum CoolQ::ServiceModule::Result
 * \brief ServiceModule 执行结果
 *
 *  CoolQ  API 密切相关的执行函数都会返回此枚举作为结果表达。
 */

/*!
 * \var CoolQ::ServiceModule::NoError
 * \brief 没有错误（成功）
 *
 * 如果函数执行成功，都会返回当前值。
 */

/*!
 * \var CoolQ::ServiceModule::Unknown
 * \brief 未知错误
 *
 * 不在当前枚举内的错误值，都会返回 Unknown。
 */

#include "CoolQServiceModule.h"
#include "CoolQServiceModule_p.h"

#include "CoolQServiceEngine.h"
#include "CoolQServiceEngine_p.h"

#include <QDir>
#include <QPixmap>
#include <QStringBuilder>
#include <QtDebug>
#include <QUuid>

#include "CoolQApi/CoolQLib.h"
#include "CoolQSqliteService_p.h"

namespace CoolQ {

// class ServiceModule

/*!
 * \brief 构造函数
 *
 * \internal
 */
ServiceModule::ServiceModule(ServiceModulePrivate &dd, ServiceEngine *parent)
    : Interface(dd, parent)
{
    if (auto engine = ServiceEnginePrivate::get(parent))
        engine->installModule(this);

    Q_D(ServiceModule);
    d->engine = parent;

    d->currentId = CQ_getLoginQQ(ServiceEnginePrivate::accessToken);

    QString path = trGbk(CQ_getAppDirectory(ServiceEnginePrivate::accessToken));
    d->resPath = QDir::cleanPath(path % "/../../data");
    d->basePath = QDir::cleanPath(path);
    d->imagePath = QDir::cleanPath(path % "/../../data/image");

    QString sqlitePath = d->basePath % '/' % QString::number(d->currentId);
    SqliteServicePrivate::basePath = QDir::cleanPath(sqlitePath);
    QDir().mkpath(SqliteServicePrivate::basePath);
}

/*!
 * \brief 构造函数
 *
 * \internal
 */
ServiceModule::ServiceModule(ServiceEngine *parent)
    : Interface(*new ServiceModulePrivate(), parent)
{
    if (auto engine = ServiceEnginePrivate::get(parent))
        engine->installModule(this);

    Q_D(ServiceModule);
    d->engine = parent;

    d->currentId = CQ_getLoginQQ(ServiceEnginePrivate::accessToken);

    QString path = trGbk(CQ_getAppDirectory(ServiceEnginePrivate::accessToken));
    d->resPath = QDir::cleanPath(path % "/../../data");
    d->basePath = QDir::cleanPath(path);
    d->imagePath = QDir::cleanPath(path % "/../../data/image");

    QString sqlitePath = d->basePath % '/' % QString::number(d->currentId);
    SqliteServicePrivate::basePath = QDir::cleanPath(sqlitePath);
    QDir().mkpath(SqliteServicePrivate::basePath);
}

/*!
 * \brief 析构函数
 *
 * \internal
 */
ServiceModule::~ServiceModule()
{
}

/*!
 * \brief 返回所属引擎
 *
 * 此函数将返回此模块所属的引擎。
 */
ServiceEngine *ServiceModule::engine() const
{
    Q_D(const ServiceModule);

    return d->engine;
}

/*!
 * \brief 标准初始化接口
 *
 * \internal
 */
bool ServiceModule::initialize()
{
    Q_D(ServiceModule);

    for(MessageFilter *filter : d->filters) {
        if (!filter->initialize())
            return false;
    }

    for(MessageFilter *filter : d->filters) {
        if (filter->filters() & MessageFilter::PrivateFilter) {
            if (filter->keywords().isEmpty()) {
                d->privateFilters.append(filter);
            } else {
                for (const QString &keyword : filter->keywords()) {
                    d->privateKeywordFilters.insert(trGbk(keyword), filter);
                }
            }
        }

        if (filter->filters() & MessageFilter::GroupFilter) {
            if (filter->keywords().isEmpty()) {
                d->groupFilters.append(filter);
            } else {
                for (const QString &keyword : filter->keywords()) {
                    d->groupKeywordFilters.insert(trGbk(keyword), filter);
                }
            }
        }

        if (filter->filters() & MessageFilter::DiscussFilter) {
            if (filter->keywords().isEmpty()) {
                d->discussFilters.append(filter);
            } else {
                for (const QString &keyword : filter->keywords()) {
                    d->discussKeywordFilters.insert(trGbk(keyword), filter);
                }
            }
        }
    }

    return true;
}

/*!
 * \brief 返回 CoolQ 中@他人的字符串
 *
 * 此函数将返回 CoolQ 中@他人的字符串。
 */
QString ServiceModule::at(qint64 uid)
{
    return QString::fromLatin1("[CQ:at,qq=%1]").arg(uid);
}

/*!
 * \brief 返回 CoolQ 中的 emoji 表情的字符串
 *
 * 此函数将返回 CoolQ 中的 emoji 表情的字符串。
 */
QString ServiceModule::emoji(int emojiCode)
{
    return QString::fromLatin1("[CQ:emoji,id=%1]").arg(emojiCode);
}

/*!
 * \brief 返回 CoolQ 中发送图片的字符串
 *
 * 此函数将返回发送图片的字符串。此文件需要保存在 CoolQ 待发送图片目录中。
 * \sa ServiceModule::loadImage ServiceModule::saveImage
 */
QString ServiceModule::image(const QString &name)
{
    return QString::fromLatin1("[CQ:image,file=%1]").arg(name);
}

/*!
 * \brief 返回 CoolQ 中的表情的字符串
 *
 * 此函数将返回 CoolQ 中的表情的字符串。
 */
QString ServiceModule::face(int face)
{
    return QString::fromLatin1("[CQ:face,id=%1]").arg(face);
}

/*!
 * \brief 通过数值返回 QString 格式的 emoji 表情
 *
 * 此函数将通过数值返回 QString 格式的 emoji 表情。
 */
QString ServiceModule::emojiChar(qint32 value)
{
    char utf8Emoji[5] = {0};

    utf8Emoji[0] = char(0xF0);
    utf8Emoji[1] = char(((value >> 0xC) & 0x3f) | 0x80);
    utf8Emoji[2] = char(((value >> 0x6) & 0x3f) | 0x80);
    utf8Emoji[3] = char(((value >> 0x0) & 0x3f) | 0x80);
    utf8Emoji[4] = 0;

    return QString::fromUtf8(utf8Emoji);
}

/*!
 * \brief 返回当前号码
 */
qint64 ServiceModule::currentId() const
{
    Q_D(const ServiceModule);

    return d->currentId;
}

/*!
 * \brief 返回插件路径
 */
QString ServiceModule::basePath() const
{
    Q_D(const ServiceModule);

    return d->basePath;
}

/*!
 * \brief 返回插件目录内的文件
 *
 * 将插件路径与文件名 \a srcName 合并，返回此文件名的绝对路径。
 */
QString ServiceModule::appFilePath(const char *srcName) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->basePath % '/' % QString::fromUtf8(srcName));
}

/*!
 * \brief 返回插件目录内的文件
 *
 * 将插件路径与文件名 \a name 合并，返回此文件名的绝对路径。
 */
QString ServiceModule::appFilePath(const QString &name) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->basePath % '/' % name);
}

/*!
 * \brief 返回插件目录内的文件
 *
 * 将插件路径与当前号码，文件名 \a srcName 合并，返回在此号码下的文件名的绝对路径。
 */
QString ServiceModule::usrFilePath(const char *srcName) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->basePath % '/' % QString::number(d->currentId) % '/' % QString::fromUtf8(srcName));
}

/*!
 * \brief 返回插件目录内的文件
 *
 * 将插件路径与当前号码，文件名 \a name 合并，返回在此号码下的文件名的绝对路径。
 */
QString ServiceModule::usrFilePath(const QString &name) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->basePath % '/' % QString::number(d->currentId) % '/' % name);
}

/*!
 * \brief 返回 CoolQ 资源目录内的文件
 *
 * 将 CoolQ 资源目录与文件名 \a srcName 合并，返回在此资源目录下的文件名的绝对路径。
 */
QString ServiceModule::resFilePath(const char *srcName) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->resPath % '/' % QString::fromUtf8(srcName));
}

/*!
 * \brief 返回 CoolQ 资源目录内的文件
 *
 * 将 CoolQ 资源目录与文件名 \a name 合并，返回在此资源目录下的文件名的绝对路径。
 */
QString ServiceModule::resFilePath(const QString &name) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->resPath % '/' % name);
}

/*!
 * \brief 返回 CoolQ 图片缓存目录内的文件
 *
 * 将 CoolQ 图片缓存目录与文件名 \a srcName 合并，返回在此缓存目录下的文件名的绝对路径。
 */
QString ServiceModule::imgFilePath(const char *srcName) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->imagePath % '/' % QString::number(d->currentId) % '/' % QString::fromUtf8(srcName));
}

/*!
 * \brief 返回 CoolQ 图片缓存目录内的文件
 *
 * 将 CoolQ 图片缓存目录与文件名 \a name 合并，返回在此缓存目录下的文件名的绝对路径。
 */
QString ServiceModule::imgFilePath(const QString &name) const
{
    Q_D(const ServiceModule);

    return QDir::cleanPath(d->imagePath % '/' % QString::number(d->currentId) % '/' % name);
}

int ServiceModule::privateMessageEventPriority() const
{
    Q_D(const ServiceModule);

    return d->privateMessageEventPriority;
}

int ServiceModule::groupMessageEventPriority() const
{
    Q_D(const ServiceModule);

    return d->groupMessageEventPriority;
}

int ServiceModule::discussMessageEventPriority() const
{
    Q_D(const ServiceModule);

    return d->discussMessageEventPriority;
}

int ServiceModule::masterChangeEventPriority() const
{
    Q_D(const ServiceModule);

    return d->masterChangeEventPriority;
}

int ServiceModule::friendRequestEventPriority() const
{
    Q_D(const ServiceModule);

    return d->friendRequestEventPriority;
}

int ServiceModule::groupRequestEventPriority() const
{
    Q_D(const ServiceModule);

    return d->groupRequestEventPriority;
}

int ServiceModule::friendAddEventPriority() const
{
    Q_D(const ServiceModule);

    return d->friendAddEventPriority;
}

int ServiceModule::memberJoinEventPriority() const
{
    Q_D(const ServiceModule);

    return d->memberJoinEventPriority;
}

int ServiceModule::memberLeaveEventPriority() const
{
    Q_D(const ServiceModule);

    return d->memberLeaveEventPriority;
}

/*!
 * \brief 处理个人消息事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::privateMessageEvent(const MessageEvent &ev)
{
    Q_D(ServiceModule);

    if (!d->privateFilters.isEmpty()) {
        for (auto filter : d->privateFilters) {
            if (filter->privateMessageFilter(0, ev)) {
                return true;
            }
        }
    }

    for (int i = 0; i < 33; ++i) {
        if ((ev.gbkMsg[i] == 0) || (ev.gbkMsg[i] == ' ')) {
            if (auto filter = d->privateKeywordFilters.value(QByteArray(ev.gbkMsg, i)))
                return filter->privateMessageFilter(i, ev);
        }
    }

    return false;
}

/*!
 * \brief 处理群组消息事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::groupMessageEvent(const MessageEvent &ev)
{
    Q_D(ServiceModule);

    if (!d->groupFilters.isEmpty()) {
        for (auto filter : d->groupFilters) {
            if (filter->groupMessageFilter(0, ev)) {
                return true;
            }
        }
    }

    for (int i = 0; i < 33; ++i) {
        if ((ev.gbkMsg[i] == 0) || (ev.gbkMsg[i] == ' ')) {
            if (auto filter = d->groupKeywordFilters.value(QByteArray(ev.gbkMsg, i)))
                return filter->groupMessageFilter(i, ev);
        }
    }

    return false;
}

/*!
 * \brief 处理会话消息事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::discussMessageEvent(const MessageEvent &ev)
{
    Q_D(ServiceModule);

    if (!d->discussFilters.isEmpty()) {
        for (auto filter : d->discussFilters) {
            if (filter->discussMessageFilter(0, ev)) {
                return true;
            }
        }
    }

    for (int i = 0; i < 33; ++i) {
        if ((ev.gbkMsg[i] == 0) || (ev.gbkMsg[i] == ' ')) {
            if (auto filter = d->discussKeywordFilters.value(QByteArray(ev.gbkMsg, i)))
                return filter->discussMessageFilter(i, ev);
        }
    }

    return false;
}

/*!
 * \brief 处理管理变更事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::masterChangeEvent(const MasterChangeEvent &ev)
{
    Q_UNUSED(ev);
    return false;
}

/*!
 * \brief 处理好友请求事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::friendRequestEvent(const FriendRequestEvent &ev)
{
    Q_UNUSED(ev);
    return false;
}

/*!
 * \brief 处理群组请求事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::groupRequestEvent(const GroupRequestEvent &ev)
{
    Q_UNUSED(ev);
    return false;
}

/*!
 * \brief 处理好友添加事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::friendAddEvent(const FriendAddEvent &ev)
{
    Q_UNUSED(ev);
    return false;
}

/*!
 * \brief 处理成员加入事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::memberJoinEvent(const MemberJoinEvent &ev)
{
    Q_UNUSED(ev);
    return false;
}

/*!
 * \brief 处理成员离开事件的过滤器
 *
 * 默认的事件过滤器什么也不做，只返回 false。
 * \return true 表示拦截此次事件；false 表示不拦截。
 */
bool ServiceModule::memberLeaveEvent(const MemberLeaveEvent &ev)
{
    Q_UNUSED(ev);
    return false;
}

/*!
 * \brief 发送个人消息
 *
 * 向个人 \a uid 发送消息 \a gbkMsg。
 * \note gbkMsg 是指使用 GBK 编码的文本，而本项目源代码是 UTF-8 编码的。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::sendPrivateMessage(qint64 uid, const char *gbkMsg) const
{
    return ServiceModulePrivate::result(CQ_sendPrivateMsg(ServiceEnginePrivate::accessToken, uid, gbkMsg));
}

/*!
 * \brief 发送群组消息
 *
 * 向群组 \a gid 发送消息 \a gbkMsg。
 * \note gbkMsg 是指使用 GBK 编码的文本，而本项目源代码是 UTF-8 编码的。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::sendGroupMessage(qint64 gid, const char *gbkMsg) const
{
    return ServiceModulePrivate::result(CQ_sendGroupMsg(ServiceEnginePrivate::accessToken, gid, gbkMsg));
}

/*!
 * \brief 发送会话消息
 *
 * 向会话 \a did 发送消息 \a gbkMsg。
 * \note gbkMsg 是指使用 GBK 编码的文本，而本项目源代码是 UTF-8 编码的。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::sendDiscussMessage(qint64 did, const char *gbkMsg) const
{
    return ServiceModulePrivate::result(CQ_sendDiscussMsg(ServiceEnginePrivate::accessToken, did, gbkMsg));
}

/*!
 * \brief 发送个人消息
 *
 * 向个人 \a uid 发送消息 \a gbkMsg。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::sendPrivateMessage(qint64 uid, const QString &msg) const
{
    return sendPrivateMessage(uid, trGbk(msg).constData());
}

/*!
 * \brief 发送群组消息
 *
 * 向群组 \a gid 发送消息 \a gbkMsg。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::sendGroupMessage(qint64 gid, const QString &msg) const
{
    return sendGroupMessage(gid, trGbk(msg).constData());
}

/*!
 * \brief 发送会话消息
 *
 * 向会话 \a did 发送消息 \a gbkMsg。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::sendDiscussMessage(qint64 did, const QString &msg) const
{
    return sendDiscussMessage(did, trGbk(msg).constData());
}

/*!
 * \brief 禁言群组成员
 *
 * 禁言群组 \a gid 中的成员 \a uid。\a duration 最短 60 秒，最长 30 天；0 表示解除禁言。（单位：秒）
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::banGroupMember(qint64 gid, qint64 uid, int duration)
{
    return ServiceModulePrivate::result(CQ_setGroupBan(ServiceEnginePrivate::accessToken, gid, uid, duration));
}

/*!
 * \brief 踢出群组成员
 *
 * 踢出群组 \a gid 中的成员 \a uid。\a lasting 为 true 表示永久禁言；false 表示只是踢出，不加入黑名单。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::kickGroupMember(qint64 gid, qint64 uid, bool lasting)
{
    return ServiceModulePrivate::result(CQ_setGroupKick(ServiceEnginePrivate::accessToken, gid, uid, lasting));
}

/*!
 * \brief 设置群组管理
 *
 * 设置群组 \a gid 中的管理 \a uid。\a enable 为 true 表示设置为管理；false 表示回复为成员。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::adminGroupMember(qint64 gid, qint64 uid, bool enabled)
{
    return ServiceModulePrivate::result(CQ_setGroupAdmin(ServiceEnginePrivate::accessToken, gid, uid, enabled));
}

/*!
 * \brief 修改成员名片
 *
 * 设置群组 \a gid 中的成员 \a uid 新的名片 \a gbkNewNameCard。
 * \note gbkNewNameCard 是指使用 GBK 编码的文本，而本项目源代码是 UTF-8 编码的。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::renameGroupMember(qint64 gid, qint64 uid, const char *gbkNewNameCard)
{
    return ServiceModulePrivate::result(CQ_setGroupCard(ServiceEnginePrivate::accessToken, gid, uid, gbkNewNameCard));
}

/*!
 * \brief 修改成员名片
 *
 * 设置群组 \a gid 中的成员 \a uid 新的名片 \a newNameCard
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::renameGroupMember(qint64 gid, qint64 uid, const QString &newNameCard)
{
    return ServiceModulePrivate::result(CQ_setGroupCard(ServiceEnginePrivate::accessToken, gid, uid, trGbk(newNameCard).constData()));
}

/*!
 * \brief 接受好友添加请求
 *
 * 接受添加为好友的请求。需要将 FriendRequestEvent::gbkTag 直接传递给 \a gbkTag。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::acceptRequest(const char *gbkTag)
{
    return ServiceModulePrivate::result(CQ_setFriendAddRequest(ServiceEnginePrivate::accessToken, gbkTag, REQUEST_ALLOW, ""));
}

/*!
 * \brief 拒绝好友添加请求
 *
 * 拒绝添加为好友的请求。需要将 FriendRequestEvent::gbkTag 直接传递给 \a gbkTag。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::rejectRequest(const char *gbkTag)
{
    return ServiceModulePrivate::result(CQ_setFriendAddRequest(ServiceEnginePrivate::accessToken, gbkTag, REQUEST_DENY, ""));
}

/*!
 * \brief 接受成员添加请求
 *
 * 接受个人加入群组的请求。需要将 GroupRequestEvent::type 直接传递给 \a type，GroupRequestEvent::gbkTag 直接传递给 \a gbkTag。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::acceptRequest(qint32 type, const char *gbkTag)
{
    if (1 == type) {
        return ServiceModulePrivate::result(CQ_setGroupAddRequestV2(ServiceEnginePrivate::accessToken, gbkTag, REQUEST_GROUPADD, REQUEST_ALLOW, ""));
    } else if (2 == type) {
        return ServiceModulePrivate::result(CQ_setGroupAddRequestV2(ServiceEnginePrivate::accessToken, gbkTag, REQUEST_GROUPINVITE, REQUEST_ALLOW, ""));
    }

    return Result::Unknown;
}

/*!
 * \brief 拒绝成员添加请求
 *
 * 拒绝个人加入群组的请求。需要将 GroupRequestEvent::type 直接传递给 \a type，GroupRequestEvent::gbkTag 直接传递给 \a gbkTag。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::rejectRequest(qint32 type, const char *gbkTag)
{
    if (1 == type) {
        return ServiceModulePrivate::result(CQ_setGroupAddRequestV2(ServiceEnginePrivate::accessToken, gbkTag, REQUEST_GROUPADD, REQUEST_DENY, ""));
    } else if (2 == type) {
        return ServiceModulePrivate::result(CQ_setGroupAddRequestV2(ServiceEnginePrivate::accessToken, gbkTag, REQUEST_GROUPINVITE, REQUEST_DENY, ""));
    }

    return Result::Unknown;
}

/*!
 * \brief 退出群组
 *
 * 这个帐号将退出群组 \a gid，注意安全！
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::leaveGroup(qint64 gid)
{
    return ServiceModulePrivate::result(CQ_setGroupLeave(ServiceEnginePrivate::accessToken, gid, false));
}

/*!
 * \brief 退出会话
 *
 * 这个帐号将退出会话 \a did，注意安全！
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::leaveDiscuss(qint64 did)
{
    return ServiceModulePrivate::result(CQ_setDiscussLeave(ServiceEnginePrivate::accessToken, did));
}

/*!
 * \brief 全员禁言
 *
 * 是否开启群组的全员禁言。\a muted 为 true 表示开启；否则表示关闭。
 * \return ServiceModule::Result
 */
ServiceModule::Result ServiceModule::mute(qint64 gid, bool muted)
{
    return ServiceModulePrivate::result(CQ_setGroupWholeBan(ServiceEnginePrivate::accessToken, gid, muted));
}

/*!
 * \brief 返回个人信息
 *
 * 此方法会返回个人 \a uid 的个人信息，默认会使用缓存数据。如果不想使用缓存，可以设置 \a cache 为 false，此时将同步获取个人信息。
 * \return 获取到的信息是否有效，可以通过返回对象的 CqPersonInfo::isValid() 函数进行验证。
 */
PersonInfo ServiceModule::personInfo(qint64 uid, bool cached)
{
    return PersonInfo(CQ_getStrangerInfo(ServiceEnginePrivate::accessToken, uid, !cached));
}

/*!
 * \brief 返回成员信息
 *
 * 此方法会返回在群组 \a gid 中的成员 \a uid 的成员信息，默认会使用缓存数据。如果不想使用缓存，可以设置 \a cache 为 false，此时将同步获取成员信息。
 * \return 获取到的信息是否有效，可以通过返回对象的 CqMemberInfo::isValid() 函数进行验证。
 */
MemberInfo ServiceModule::memberInfo(qint64 gid, qint64 uid, bool cached)
{
    return MemberInfo(CQ_getGroupMemberInfoV2(ServiceEnginePrivate::accessToken, gid, uid, !cached));
}

/*!
 * \brief 保存图片
 *
 * 将图片 \a data 保存到 CoolQ 待发送图片目录。如果成功，返回自动生成的文件名。
 */
QString ServiceModule::saveImage(const QPixmap &data) const
{
    Q_D(const ServiceModule);

    QString uuid = QString::fromLatin1(QUuid::createUuid().toRfc4122().toHex());
    QString name = QString("%1/%2.png").arg(d->imagePath, uuid);
    if (data.save(name, "PNG", 0)) {
        return uuid % ".png";
    }

    return QString();
}

/*!
 * \brief 载入图片
 *
 * 将载入 CoolQ 待发送图片目录内文件名为 \a name 的图片，如果成功，返回此图片对象。
 */
QPixmap ServiceModule::loadImage(const QString &name) const
{
    Q_UNUSED(name);
    return QPixmap();
}

// class ServiceModulePrivate

/*!
 * \internal
 */
ServiceModulePrivate::ServiceModulePrivate()
    : engine(nullptr)
    , currentId(0)
    //
    , privateMessageEventPriority(1)
    , groupMessageEventPriority(1)
    , discussMessageEventPriority(1)
    //
    , masterChangeEventPriority(1)
    , friendRequestEventPriority(1)
    , groupRequestEventPriority(1)
    //
    , friendAddEventPriority(1)
    , memberJoinEventPriority(1)
    , memberLeaveEventPriority(1)
{
}

/*!
 * \internal
 */
ServiceModulePrivate::~ServiceModulePrivate()
{
}

/*!
 * \internal
 */
void ServiceModulePrivate::installFilter(MessageFilter *filter)
{
    Q_CHECK_PTR(filter);

    filters.append(filter);
}

/*!
 * \internal
 */
ServiceModule::Result ServiceModulePrivate::result(qint32 r)
{
    switch (r) {
    case 0:
        return ServiceModule::NoError;
    case 1:
        break;
    default:
        return ServiceModule::Unknown;
    }

    return ServiceModule::Unknown;
}

} // namespace CoolQ
