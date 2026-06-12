#include "audittextdelegate.h"

#include <QEvent>

#include "audithub/auditenum.h"
#include "audittextdialog.h"

AuditTextDelegate::AuditTextDelegate(QObject* parent)
    : StyledItemDelegate { parent }
{
}

bool AuditTextDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model);

    if (event->type() == QEvent::MouseButtonDblClick) {
        const QString text { index.data().toString().trimmed() };

        const QString title { index.column() == std::to_underlying(audit_hub::AuditField::kBefore) ? QObject::tr("Before Change")
                                                                                                   : QObject::tr("After Change") };

        if ((text.startsWith('{') || text.startsWith('[')) && text != "{}" && text != "[]") {
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
    const QString text { index.data(Qt::DisplayRole).toString().trimmed() };

    if (text.isEmpty() || text == "{}" || text == "[]")
        return PaintEmpty(painter, option, index);

    StyledItemDelegate::paint(painter, option, index);
}
