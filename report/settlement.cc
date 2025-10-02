#include "settlement.h"

void Settlement::ResetState()
{
    id = QUuid();
    partner = QUuid();
    employee = QUuid();
    issued_time = {};
    description.clear();
    status = false;
    initial_total = 0.0;

    user_id = QUuid();
    created_time = {};
    created_by = QUuid();
    updated_time = {};
    updated_by = QUuid();
    is_valid = true;
}

const QStringList& Settlement::SqlField() const
{
    static const QStringList field {
        QStringLiteral("id"),
        QStringLiteral("partner"),
        QStringLiteral("issued_time"),
        QStringLiteral("description"),
        QStringLiteral("status"),
        QStringLiteral("initial_total"),
        QStringLiteral("user_id"),
        QStringLiteral("created_time"),
        QStringLiteral("created_by"),
        QStringLiteral("updated_time"),
        QStringLiteral("updated_by"),
        QStringLiteral("version"),
        QStringLiteral("is_valid"),
    };

    return field;
}

void Settlement::ReadJson(const QJsonObject& object)
{
    id = QUuid(object.value(QStringLiteral("id")).toString());
    partner = QUuid(object.value(QStringLiteral("partner")).toString());
    issued_time = QDateTime::fromString(object[QStringLiteral("issued_time")].toString(), Qt::ISODate);
    description = object.value(QStringLiteral("description")).toString();
    status = object.value(QStringLiteral("status")).toBool();
    initial_total = object.value(QStringLiteral("initial_total")).toDouble();

    user_id = QUuid(object.value(QStringLiteral("user_id")).toString());
    created_time = QDateTime::fromString(object[QStringLiteral("created_time")].toString(), Qt::ISODate);
    created_by = QUuid(object.value(QStringLiteral("created_by")).toString());
    updated_time = QDateTime::fromString(object[QStringLiteral("updated_time")].toString(), Qt::ISODate);
    updated_by = QUuid(object.value(QStringLiteral("updated_by")).toString());
    is_valid = object.value(QStringLiteral("is_valid")).toBool();
}

QJsonObject Settlement::WriteJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("id"), id.toString());
    obj.insert(QStringLiteral("partner"), partner.toString());
    obj.insert(QStringLiteral("issued_time"), issued_time.toString(Qt::ISODate));
    obj.insert(QStringLiteral("description"), description);
    obj.insert(QStringLiteral("status"), status);
    obj.insert(QStringLiteral("initial_total"), initial_total);

    obj.insert(QStringLiteral("user_id"), user_id.toString());
    obj.insert(QStringLiteral("created_time"), created_time.toString(Qt::ISODate));
    obj.insert(QStringLiteral("created_by"), created_by.toString());
    obj.insert(QStringLiteral("updated_time"), updated_time.toString(Qt::ISODate));
    obj.insert(QStringLiteral("updated_by"), updated_by.toString());
    obj.insert(QStringLiteral("is_valid"), is_valid);
    return obj;
}
