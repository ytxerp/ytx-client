#include "plaintextguard.h"

#include <QApplication>
#include <QRect>

#include "widget/plaintextedit.h"

PlainTextGuard::PlainTextGuard(QObject* parent)
    : StyledItemDelegate { parent }
{
}

QWidget* PlainTextGuard::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto* editor { new PlainTextEdit(parent) };
    return editor;
}

void PlainTextGuard::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<PlainTextEdit*>(editor) };
    if (cast_editor->hasFocus())
        return;

    cast_editor->setPlainText(index.data().toString());
}

void PlainTextGuard::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QSize mainwindow_size { qApp->activeWindow()->size() };
    int width { mainwindow_size.width() * 150 / 1920 };
    int height { mainwindow_size.height() * 200 / 1080 };

    editor->setGeometry(QRect(option.rect.bottomLeft(), QSize(width, height)));
}

void PlainTextGuard::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<PlainTextEdit*>(editor) };
    model->setData(index, cast_editor->toPlainText());
}
