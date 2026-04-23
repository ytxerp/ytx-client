#include "databaseroledelegate.h"

#include <QStandardItem>

#include "widget/combobox.h"

DatabaseRoleDelegate::DatabaseRoleDelegate(const QList<PermissionItem>& database_role_list, QObject* parent)
    : StyledItemDelegate { parent }
    , database_role_list_ { database_role_list }
{
}

QWidget* DatabaseRoleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* editor { new ComboBox(parent) };

    auto* model { new QStandardItemModel(editor) };
    editor->setModel(model);

    for (const auto& item : database_role_list_) {
        auto* model_item { new QStandardItem(item.text) };

        model_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        model_item->setData(static_cast<int>(item.flag), Qt::UserRole);
        model_item->setCheckState(Qt::Unchecked);

        model->appendRow(model_item);
    }

    return editor;
}

void DatabaseRoleDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int value { index.data().toInt() };
    const PermissionBits::Flags flags(value);

    auto* model { qobject_cast<QStandardItemModel*>(cast_editor->model()) };

    for (int i = 0; i != model->rowCount(); ++i) {
        auto* item { model->item(i) };

        const int flag { item->data(Qt::UserRole).toInt() };
        const bool checked { static_cast<int>(flags & flag) == flag };
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }

    // Set line edit text to show all selected roles
    cast_editor->setEditText(FlagsToDisplay(flags));
}

void DatabaseRoleDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    auto* cast_model { qobject_cast<QStandardItemModel*>(cast_editor->model()) };

    int result { 0 };

    for (int i = 0; i != cast_model->rowCount(); ++i) {
        auto* item { cast_model->item(i) };

        if (item->checkState() == Qt::Checked) {
            result |= item->data(Qt::UserRole).toInt();
        }
    }

    model->setData(index, result);
}

void DatabaseRoleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    const QString text { FlagsToDisplay(PermissionBits::Flags(value)) };

    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize DatabaseRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    const QString text { FlagsToDisplay(PermissionBits::Flags(value)) };

    return CalculateTextSize(text, option);
}

void DatabaseRoleDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };
    const int value { index.data().toInt() };
    const QString display_text { FlagsToDisplay(PermissionBits::Flags(value)) };

    const QSize text_size { CalculateTextSize(display_text, option) };

    const int width { std::max(option.rect.width(), text_size.width() + bar_width) };
    const int height { std::max(option.rect.height(), text_size.height()) };

    QRect geom { option.rect };
    geom.setWidth(width);
    geom.setHeight(height);

    editor->setGeometry(geom);
}

QString DatabaseRoleDelegate::FlagsToDisplay(PermissionBits::Flags flags) const
{
    QString result {};

    for (const auto& item : database_role_list_) {
        if ((flags & item.flag) == item.flag) { // exact match
            if (!result.isEmpty())
                result += " | ";
            result += item.text;
        }
    }

    return result;
}
