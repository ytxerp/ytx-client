#include "audittextdelegate.h"

#include <QEvent>

#include "audit/auditrow.h"
#include "audittextdialog.h"

AuditTextDelegate::AuditTextDelegate(QObject* parent)
    : StyledItemDelegate { parent }
{
}

bool AuditTextDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        const QJsonValue value { index.data(audit::kRawJsonRole).toJsonValue() };
        const QString text { audit::JsonValueToString(value, QJsonDocument::Indented) };

        if (!text.isEmpty()) {
            const QString title { index.column() == std::to_underlying(audit::RowField::kBefore) ? QObject::tr("Before Change") : QObject::tr("After Change") };

            auto* dialog = new AuditTextDialog(text, title, const_cast<QWidget*>(option.widget));
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();

            return true; // handled
        }
    }

    return StyledItemDelegate::editorEvent(event, model, option, index);
}

void AuditTextDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(Qt::DisplayRole).toString().isEmpty())
        return PaintEmpty(painter, option, index);

    StyledItemDelegate::paint(painter, option, index);
}
