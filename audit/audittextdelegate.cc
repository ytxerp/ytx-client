#include "audittextdelegate.h"

#include <QEvent>

#include "audit/auditrow.h"
#include "audittextdialog.h"

namespace {
bool IsEmptyJson(QStringView text) { return text == QLatin1StringView("{}") || text == QLatin1StringView("[]"); }
} // namespace

AuditTextDelegate::AuditTextDelegate(QObject* parent)
    : StyledItemDelegate { parent }
{
}

bool AuditTextDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model);

    if (event->type() == QEvent::MouseButtonDblClick) {
        const QString text { index.data().toString().trimmed() };

        const QString title { index.column() == std::to_underlying(audit::RowField::kBefore) ? QObject::tr("Before Change") : QObject::tr("After Change") };

        if ((text.startsWith('{') || text.startsWith('[')) && !IsEmptyJson(text)) {
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
    const QString raw { index.data(Qt::DisplayRole).toString() };
    const QStringView text { QStringView(raw).trimmed() };

    if (text.isEmpty() || IsEmptyJson(text))
        return PaintEmpty(painter, option, index);

    StyledItemDelegate::paint(painter, option, index);
}
