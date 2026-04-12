#include "auditentry.h"

#include "component/constant.h"

namespace audit_hub::keys {
const QString kTargetId = QStringLiteral("target_id");
const QString kTargetType = QStringLiteral("target_type");
const QString kWsKey = QStringLiteral("ws_key");
const QString kLevel = QStringLiteral("level");
const QString kTargetCode = QStringLiteral("target_code");
const QString kBefore = QStringLiteral("before");
const QString kAfter = QStringLiteral("after");
}

void audit_hub::AuditEntry::Reset() { *this = AuditEntry {}; }

void audit_hub::AuditEntry::ReadJson(const QJsonObject& object)
{
    using namespace audit_hub::keys;

    if (const auto val = object.value(kId); val.isString())
        id = QUuid(val.toString());
    if (const auto val = object.value(kTargetId); val.isString())
        target_id = QUuid(val.toString());
    if (const auto val = object.value(kUserId); val.isString())
        user_id = QUuid(val.toString());
    if (const auto val = object.value(kLhsNode); val.isString())
        lhs_node = QUuid(val.toString());
    if (const auto val = object.value(kRhsNode); val.isString())
        rhs_node = QUuid(val.toString());
    if (const auto val = object.value(kCreatedTime); val.isString())
        created_time = QDateTime::fromString(val.toString(), Qt::ISODateWithMs);
    if (const auto val = object.value(kSection); val.isDouble())
        section = val.toInt();
    if (const auto val = object.value(kWsKey); val.isDouble())
        ws_key = val.toInt();
    if (const auto val = object.value(kTargetType); val.isDouble())
        target_type = val.toInt();
    if (const auto val = object.value(kLevel); val.isDouble())
        level = val.toInt();
    if (const auto val = object.value(kTargetCode); val.isString())
        target_code = val.toString();
    if (const auto val = object.value(kBefore); !val.isUndefined())
        before = val;
    if (const auto val = object.value(kAfter); !val.isUndefined())
        after = val;
}
