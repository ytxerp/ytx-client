#include "jsongen.h"

#include <QJsonArray>

#include "component/constant.h"

namespace JsonGen {

QJsonObject NodeInsert(Section section, const Node* node, CUuid& parent_id)
{
    Q_ASSERT(node != nullptr);

    const QJsonObject node_json { node->WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, parent_id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node->id.toString(QUuid::WithoutBraces));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNode, node_json);
    message.insert(kPath, path_json);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject NodeDrag(Section section, CUuid& node_id, CUuid& parent_id)
{
    QJsonObject path_json {};
    path_json.insert(kAncestor, parent_id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id.toString(QUuid::WithoutBraces));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kPath, path_json);

    return message;
}

QJsonObject LeafRemove(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kLinkedEntry, QJsonObject());
    message.insert(kTotalArray, QJsonArray());

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
    message.insert(kInsideRef, false);
    message.insert(kInventoryIntRef, false);
    message.insert(kInventoryExtRef, false);
    message.insert(kPartnerRef, false);
    message.insert(kEmployeeRef, false);
    message.insert(kSettlementRef, false);

    return message;
}

QJsonObject LeafReplace(Section section, CUuid& old_id, CUuid& new_id, bool inventory_external_ref, NodeUnit unit)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kResult, false);
    message.insert(kInventoryOutsideRef, inventory_external_ref);
    message.insert(kOldNodeId, old_id.toString(QUuid::WithoutBraces));
    message.insert(kNewNodeId, new_id.toString(QUuid::WithoutBraces));
    message.insert(kUnit, std::to_underlying(unit));

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

QJsonObject NodeStatus(Section section, CUuid& node_id, enum NodeStatus status)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kStatus, std::to_underlying(status));
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject EntryUpdate(Section section, CUuid& entry_id, CJsonObject& update)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kSessionId, QString());
    message.insert(kUpdate, update);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject NodeUpdate(Section section, CUuid& node_id, CJsonObject& update)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kSessionId, QString());
    message.insert(kUpdate, update);
    message.insert(kMeta, QJsonObject());

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

QJsonObject OrderRecalled(Section section, CUuid& node_id, CJsonObject& update)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kMeta, QJsonObject());
    message.insert(kNodeUpdate, update);
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    return message;
}

QJsonObject EntryValue(Section section, CUuid& entry_id, CJsonObject& update, bool is_parallel)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kUpdate, update);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject EntryRemove(Section section, CUuid& entry_id)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject MetaMessage(Section section)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject EntryLinkedNode(Section section, CUuid& entry_id)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kMeta, QJsonObject());
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject SaleReferenceAcked(Section section, CUuid& widget_id, CUuid& node_id, int unit, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kUnit, unit);
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject StatementAcked(Section section, CUuid& widget_id, int unit, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kUnit, unit);
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject StatementNodeAcked(Section section, CUuid& widget_id, CUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kPartnerId, partner_id.toString(QUuid::WithoutBraces));
    message.insert(kUnit, unit);
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject StatementEntryAcked(Section section, CUuid& widget_id, CUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kPartnerId, partner_id.toString(QUuid::WithoutBraces));
    message.insert(kUnit, unit);
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kArray, QJsonArray());
    message.insert(kTotal, QJsonObject());

    return message;
}

QJsonObject SettlementAcked(Section section, CUuid& widget_id, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject SettlementNodeAcked(Section section, CUuid& widget_id, CUuid& partner_id, CUuid& settlement_id)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kPartnerId, partner_id.toString(QUuid::WithoutBraces));
    message.insert(kSettlementId, settlement_id.toString(QUuid::WithoutBraces));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject SettlementRemoved(Section section, CUuid& settlement_id)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSettlementId, settlement_id.toString(QUuid::WithoutBraces));

    return message;
}

}
