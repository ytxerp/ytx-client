#include "audittextdelegate.h"

#include <QEvent>

#include "dialog/audittextdialog.h"

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

        if ((text.startsWith('{') || text.startsWith('[')) && text != "{}" && text != "[]") {
            auto* dialog = new AuditTextDialog(text, const_cast<QWidget*>(option.widget));
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            dialog->show();
            return true; // handled
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
