#include "jsongen.h"

#include <QJsonArray>

#include "component/constant.h"
#include "global/logininfo.h"

namespace JsonGen {

QJsonObject InsertNode(CString& section, const Node* node, CUuid& parent_id)
{
    const QJsonObject node_json { node->WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, parent_id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node->id.toString(QUuid::WithoutBraces));

    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kNode, node_json); // Meta info will be appended in service
    message.insert(kPath, path_json); // Meta will be appended in service

    return message;
}

QJsonObject Update(CString& section, CUuid& id, CJsonObject& cache)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kId, id.toString(QUuid::WithoutBraces));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache); // Meta info will be appended in service

    return message;
}

QJsonObject DragNode(CString& section, CUuid& node_id, CUuid& parent_id)
{
    QJsonObject path_json {};
    path_json.insert(kAncestor, parent_id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id.toString(QUuid::WithoutBraces));

    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kPath, path_json); // Meta info will be appended in service
    message.insert(kNode, QJsonObject()); // Meta info will be appended in service

    return message;
}

QJsonObject RemoveLeafNode(CString& section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kLeafEntry, QJsonObject());
    message.insert(kNodeDelta, QJsonArray());

    return message;
}

QJsonObject RemoveBranchNode(CString& section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kId, node_id.toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject Login()
{
    LoginInfo& login_info { LoginInfo::Instance() };

    QJsonObject message {};
    message.insert(kEmail, login_info.Email());
    message.insert(kPassword, login_info.Password());
    message.insert(kWorkspace, login_info.Workspace());

    return message;
}

QJsonObject LeafCheckBeforeRemove(CString& section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kInternalReference, false);
    message.insert(kExternalReference, false);

    return message;
}

QJsonObject ReplaceLeafNode(CString& section, CUuid& old_id, CUuid& new_id, bool external_reference)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kStatus, false);
    message.insert(kExternalReference, external_reference);
    message.insert(kOldId, old_id.toString(QUuid::WithoutBraces));
    message.insert(kNewId, new_id.toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject LeafAcked(CString& section, CUuid& leaf_id, CUuid& entry_id)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kLeafId, leaf_id.toString(QUuid::WithoutBraces));
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));
    message.insert(kEntryArray, QJsonArray());
    return message;
}

QJsonObject CheckAction(CString& section, CUuid& leaf_id, int check)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kLeafId, leaf_id.toString(QUuid::WithoutBraces));
    message.insert(kCheck, check);
    message.insert(kMeta, QJsonObject());

    return message;
}

QJsonObject NodeDataAcked(CString& section, const QDateTime& start, const QDateTime& end)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kStart, start.toString(Qt::ISODate));
    message.insert(kEnd, end.toString(Qt::ISODate));
    return message;
}

QJsonObject UpdateDocumentDir(CString& section, CString& document_dir)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kDocumentDir, document_dir);
    return message;
}

QJsonObject UpdateDefaultUnit(CString& section, int unit)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kDefaultUnit, unit);
    return message;
}

QJsonObject LeafDelta(CUuid& leaf_id, double initial_delta, double final_delta)
{
    QJsonObject delta {};
    delta.insert(kInitialDelta, QString::number(initial_delta, 'f', kMaxNumericScale_4));
    delta.insert(kFinalDelta, QString::number(final_delta, 'f', kMaxNumericScale_4));
    delta.insert(kId, leaf_id.toString(QUuid::WithoutBraces));

    return delta;
}

QJsonObject Register(CString& email, CString& password)
{
    QJsonObject message {};
    message.insert(kEmail, email);
    message.insert(kPassword, password);

    return message;
}

QJsonObject SearchEntry(CString& section, CString& keyword)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kKeyword, keyword);
    message.insert(kEntryArray, QJsonObject());

    return message;
}

QJsonObject OneNode(CString& section, CUuid& node_id)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kNodeId, node_id.toString(QUuid::WithoutBraces));
    message.insert(kNode, QJsonObject());
    message.insert(kAncestor, QUuid().toString(QUuid::WithoutBraces));

    return message;
}

QJsonObject NodeDirectionRule(CString& section, CUuid& id, bool direction_rule)
{
    QJsonObject message {};
    message.insert(kSection, section);
    message.insert(kSessionId, QString());
    message.insert(kId, id.toString(QUuid::WithoutBraces));
    message.insert(kDirectionRule, direction_rule);
    message.insert(kMeta, QJsonObject()); // Meta info will be appended in service

    return message;
}

}
