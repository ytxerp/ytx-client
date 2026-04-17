#include "audittextdelegate.h"

#include <QEvent>

#include "audithub/auditenum.h"
#include "audittextdialog.h"

AuditTextDelegate::AuditTextDelegate(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

bool AuditTextDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model);
    Q_UNUSED(option);

    if (event->type() == QEvent::MouseButtonDblClick) {
        const QString text { index.data().toString().trimmed() };
        const QString title { index.column() == std::to_underlying(audit_hub::AuditField::kBefore) ? QObject::tr("Audit Detail - Before")
                                                                                                   : QObject::tr("Audit Detail - After") };

        if ((text.startsWith('{') || text.startsWith('[')) && text != "{}" && text != "[]") {
            auto* dialog = new AuditTextDialog(text, title, const_cast<QWidget*>(option.widget));
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();
            return true; // handled
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
