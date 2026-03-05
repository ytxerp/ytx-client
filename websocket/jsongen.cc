#include "jsongen.h"

#include <QJsonArray>

#include "component/constant.h"
#include "component/constantstring.h"
#include "component/constantwebsocket.h"

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

QJsonObject LeafDelete(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kLinkedEntry, QJsonObject());
    message.insert(kTotalArray, QJsonArray());

    return message;
}

QJsonObject LeafDeleteP(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kLinkedEntry, QJsonArray());

    return message;
}

QJsonObject LeafDeleteO(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kPartnerId, QUuid().toString(QUuid::WithoutBraces));
    message.insert(kUnit, int {});
    message.insert(kStatus, int {});
    message.insert(kInitialTotal, QString());

    return message;
}

QJsonObject BranchDelete(Section section, CUuid& node_id, CUuid& parent_id)
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

QJsonObject LeafDeleteCheck(Section section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(NodeRef::kWithin, false);
    message.insert(NodeRef::kInventoryInt, false);
    message.insert(NodeRef::kInventoryExt, false);
    message.insert(NodeRef::kPartnerCV, false);
    message.insert(NodeRef::kPartnerEmp, false);
    message.insert(NodeRef::kOrder, false);
    return message;
}

QJsonObject LeafReplace(Section section, CUuid& old_id, CUuid& new_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kResult, false);
    message.insert(kOldNodeId, old_id.toString(QUuid::WithoutBraces));
    message.insert(kNewNodeId, new_id.toString(QUuid::WithoutBraces));
    message.insert(NodeRef::kInventoryInt, false);
    message.insert(NodeRef::kInventoryExt, false);

    return message;
}

QJsonObject TableAck(Section section, CUuid& node_id, CUuid& entry_id)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kEntryArray, QJsonArray());
    return message;
}

QJsonObject BatchMark(Section section, CUuid& node_id, int mark)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(WsField::kMark, mark);

    return message;
}

QJsonObject TreeAck(Section section, const QDateTime& start, const QDateTime& end)
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
    message.insert(WsField::kDocumentDir, document_dir);
    return message;
}

QJsonObject DefaultUnit(Section section, int unit)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(WsField::kDefaultUnit, unit);
    return message;
}

QJsonObject Register(CString& email, CString& password)
{
    QJsonObject message {};
    message.insert(kEmail, email);
    message.insert(kPassword, password);

    return message;
}

QJsonObject EntryDescriptionSearch(Section section, CString& keyword)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kKeyword, keyword);
    message.insert(kEntryArray, QJsonArray());

    return message;
}

QJsonObject EntryTagSearch(Section section, const QSet<QString>& tags)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kEntryArray, QJsonArray());

    // Convert QSet<QString> to QJsonArray
    QJsonArray tag_array {};
    for (const QString& tag_id : tags) {
        tag_array.append(tag_id);
    }
    message.insert(kTagArray, tag_array);

    return message;
}

QJsonObject NodeAck(Section section, CUuid& node_id)
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

QJsonObject OrderRecall(Section section, CUuid& node_id, CJsonObject& update)
{
    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kMeta, QJsonObject());
    message.insert(WsField::kNodeUpdate, update);
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

QJsonObject EntryDelete(Section section, CUuid& entry_id)
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

QJsonObject OrderReferenceAck(Section section, CUuid& widget_id, CUuid& node_id, int unit, const QDateTime& start, const QDateTime& end)
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

QJsonObject StatementAck(Section section, CUuid& widget_id, int unit, const QDateTime& start, const QDateTime& end)
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

QJsonObject StatementNodeAck(Section section, CUuid& widget_id, CUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end)
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

QJsonObject StatementEntryAck(Section section, CUuid& widget_id, CUuid& partner_id, int unit, const QDateTime& start, const QDateTime& end)
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

QJsonObject SettlementAck(Section section, CUuid& widget_id, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject SettlementNodeAck(Section section, CUuid& widget_id, CUuid& partner_id, CUuid& settlement_id)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kWidgetId, widget_id.toString(QUuid::WithoutBraces));
    message.insert(kPartnerId, partner_id.toString(QUuid::WithoutBraces));
    message.insert(kSettlementId, settlement_id.toString(QUuid::WithoutBraces));
    message.insert(kArray, QJsonArray());

    return message;
}

QJsonObject SettlementDelete(Section section, CUuid& settlement_id, int version)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSettlementId, settlement_id.toString(QUuid::WithoutBraces));
    message.insert(kVersion, version);

    return message;
}

QJsonObject TagUpdate(Section section, const Tag* tag)
{
    QJsonObject update {};

    update.insert(kName, tag->name);
    update.insert(kColor, tag->color);

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kMeta, QJsonObject());
    message.insert(kId, tag->id.toString(QUuid::WithoutBraces));
    message.insert(kUpdate, update);
    message.insert(kVersion, tag->version);

    return message;
}

QJsonObject TagInsert(Section section, const Tag* tag)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kTag, tag->WriteJson());
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject TagDelete(Section section, CUuid& tag_id)
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section));
    message.insert(kSessionId, QString());
    message.insert(kId, tag_id.toString(QUuid::WithoutBraces));

    return message;
}

}
