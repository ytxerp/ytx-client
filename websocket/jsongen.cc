#include "jsongen.h"

#include <QJsonArray>

#include "component/constant.h"

namespace JsonGen {

QJsonObject NodeInsert(Section section, const Node* node, CUuid& parent_id)
{
    const QJsonObject node_json { node->WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, parent_id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node->id.toString(QUuid::WithoutBraces));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNode, node_json); // Meta info will be appended in service
    message.insert(kPath, path_json);

    return message;
}

QJsonObject DragNode(Section section, CUuid& node_id, CUuid& parent_id)
{
    QJsonObject path_json {};
    path_json.insert(kAncestor, parent_id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id.toString(QUuid::WithoutBraces));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kPath, path_json);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject LeafRemove(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kLinkedEntry, QJsonObject());
    message.insert(kDeltaArray, QJsonArray());

    return message;
}

QJsonObject BranchRemove(Section section, CUuid& node_id, CUuid& parent_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kParentId, parent_id.toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject Login(CString& email, CString& password, CString& workspace)
{
    QJsonObject message {};
    message.insert(kEmail, email);
    message.insert(kPassword, password);
    message.insert(kWorkspace, workspace);

    return message;
}

QJsonObject LeafRemoveCheck(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kInternallyRef, false);
    message.insert(kInventoryInternalRef, false);
    message.insert(kInventoryExternalRef, false);
    message.insert(kPartnerRef, false);
    message.insert(kEmployeeRef, false);
    message.insert(kSettlementRef, false);

    return message;
}

QJsonObject LeafReplace(Section section, CUuid& old_id, CUuid& new_id, bool inventory_external_ref)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kResult, false);
    message.insert(kInventoryExternalRef, inventory_external_ref);
    message.insert(kOldNodeId, old_id.toString(QUuid::WithoutBraces));
    message.insert(kNewNodeId, new_id.toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject LeafEntry(Section section, CUuid& node_id, CUuid& entry_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kEntryArray, QJsonArray());
    return message;
}

QJsonObject EntryAction(Section section, CUuid& node_id, int action)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kAction, action);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject TreeAcked(Section section, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kNodeArray, QJsonArray());
    message.insert(kPathArray, QJsonArray());
    return message;
}

QJsonObject DocumentDir(Section section, CString& document_dir)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kDocumentDir, document_dir);
    return message;
}

QJsonObject DefaultUnit(Section section, int unit)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kDefaultUnit, unit);
    return message;
}

QJsonObject NodeDelta(CUuid& node_id, double initial_delta, double final_delta)
{
    QJsonObject message {};
    message.insert(kInitialDelta, QString::number(initial_delta, 'f', kMaxNumericScale_4));
    message.insert(kFinalDelta, QString::number(final_delta, 'f', kMaxNumericScale_4));
    message.insert(kId, node_id.toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject Register(CString& email, CString& password)
{
    QJsonObject message {};
    message.insert(kEmail, email);
    message.insert(kPassword, password);

    return message;
}

QJsonObject EntrySearch(Section section, CString& keyword)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kKeyword, keyword);
    message.insert(kEntryArray, QJsonArray());

    return message;
}

QJsonObject NodeAcked(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kNode, QJsonObject());
    message.insert(kAncestor, QUuid().toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject NodeDirectionRule(Section section, CUuid& node_id, bool direction_rule)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kDirectionRule, direction_rule);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject NodeStatus(Section section, CUuid& node_id, int status)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kStatus, status);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject EntryUpdate(Section section, CUuid& entry_id, CJsonObject& cache)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache); // Meta info will be appended to cache in service

    return message;
}

QJsonObject NodeUpdate(Section section, CUuid& node_id, CJsonObject& cache)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache); // Meta info will be appended to cache in service

    return message;
}

QJsonObject NodeName(Section section, CUuid& node_id, CString& name)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kSessionId, QString());
    message.insert(kName, name);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject NodeSearch(Section section, CString& keyword)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kKeyword, keyword);
    message.insert(kNodeArray, QJsonArray());

    return message;
}

QJsonObject OrderRecalled(Section section, const NodeO* node)
{
    QJsonObject node_cache {};
    node_cache.insert(kStatus, std::to_underlying(NodeStatus::kRecalled));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node->id.toString(QUuid::WithoutBraces));
    message.insert(kNodeCache, node_cache);

    if (node->unit == std::to_underlying(UnitO::kMonthly) && FloatChanged(-node->initial_total, 0.0)) {
        QJsonObject partner_delta {};

        partner_delta.insert(kInitialDelta, QString::number(-node->initial_total, 'f', kMaxNumericScale_4));
        partner_delta.insert(kId, node->partner.toString(QUuid::WithoutBraces));

        message.insert(kPartnerDelta, partner_delta);
    }

    return message;
}

}
