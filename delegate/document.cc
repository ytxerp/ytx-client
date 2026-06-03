#include "document.h"

#include <QMouseEvent>

#include "dialog/editdocument.h"

Document::Document(CString& document_path, QObject* parent)
    : StyledItemDelegate { parent }
    , document_path_ { document_path }
{
}

void Document::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList list { index.data().toStringList() };
    const auto size { list.size() };

    if (size == 0) {
        return PaintEmpty(painter, option, index);
    }

    const QString text { QString::number(size) };
    PaintText(text, painter, option, index, Qt::AlignCenter);
}

bool Document::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() != QEvent::MouseButtonDblClick)
        return StyledItemDelegate::editorEvent(event, model, option, index);

    auto* mouse_event { static_cast<QMouseEvent*>(event) };
    if (!mouse_event || mouse_event->button() != Qt::LeftButton || !option.rect.contains(mouse_event->pos()))
        return false;

    auto list { index.data().toStringList() };

    auto dialog = std::make_unique<EditDocument>(list, document_path_);
    if (dialog->exec() == QDialog::Accepted)
        model->setData(index, list);

    return true;
}

QSize Document::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QStringList list { index.data().toStringList() };
    const auto size { list.size() };

    const QString text { size == 0 ? QString() : QString::number(size) };
    return CalculateTextSize(text, option);
}
