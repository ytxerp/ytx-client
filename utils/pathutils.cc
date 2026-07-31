#include "pathutils.h"

#include "component/constant.h"

QList<path::Dto> path::Parse(const QJsonArray& array)
{
    QList<Dto> paths {};
    paths.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << Q_FUNC_INFO << "Invalid path, expected object:" << value;
            continue;
        }

        Dto path {};
        path.ReadJson(value.toObject());

        paths.emplaceBack(path);
    }

    return paths;
}

void path::Dto::ReadJson(const QJsonObject& object)
{
    Q_ASSERT_X(object.contains(kAncestor), Q_FUNC_INFO, "Missing ancestor field");
    Q_ASSERT_X(object.contains(kDescendant), Q_FUNC_INFO, "Missing descendant field");

    ancestor_id = QUuid(object.value(kAncestor).toString());
    descendant_id = QUuid(object.value(kDescendant).toString());

    Q_ASSERT_X(!ancestor_id.isNull(), Q_FUNC_INFO, "Invalid ancestor id");
    Q_ASSERT_X(!descendant_id.isNull(), Q_FUNC_INFO, "Invalid descendant id");
}
