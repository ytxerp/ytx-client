#include "financeroledelegate.h"

#include <QStandardItem>

#include "widget/combobox.h"

FinanceRoleDelegate::FinanceRoleDelegate(std::span<const finance::RoleItem> roles, QObject* parent)
    : StyledItemDelegate { parent }
    , roles_ { roles }
{
}

QWidget* FinanceRoleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };

    auto* model { new QStandardItemModel(editor) };
    editor->setModel(model);

    for (const auto& item : roles_) {
        auto* model_item { new QStandardItem(item.text) };

        model_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        model_item->setData(static_cast<int>(item.role), Qt::UserRole);
        model_item->setCheckState(Qt::Unchecked);

        model->appendRow(model_item);
    }

    return editor;
}

void FinanceRoleDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int value { index.data().toInt() };
    const finance::Roles roles(value);

    auto* model { qobject_cast<QStandardItemModel*>(cast_editor->model()) };

    for (int i = 0; i != model->rowCount(); ++i) {
        auto* item { model->item(i) };

        const int role { item->data(Qt::UserRole).toInt() };
        const bool checked { static_cast<int>(roles & role) == role };
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }

    // Set line edit text to show all selected roles
    cast_editor->setEditText(finance::RolesDisplay(roles));
}

void FinanceRoleDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
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

void FinanceRoleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    if (value == 0)
        return PaintEmpty(painter, option, index);

    const QString text { finance::RolesDisplay(finance::Role(value)) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize FinanceRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    const QString text { finance::RolesDisplay(finance::Role(value)) };

    return CalculateTextSize(text, option);
}

void FinanceRoleDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };
    const int value { index.data().toInt() };
    const QString display_text { finance::RolesDisplay(finance::Role(value)) };

    const QSize text_size { CalculateTextSize(display_text, option) };

    const int width { std::max(option.rect.width(), text_size.width() + bar_width) };
    const int height { std::max(option.rect.height(), text_size.height()) };

    QRect geom { option.rect };
    geom.setWidth(width);
    geom.setHeight(height);

    editor->setGeometry(geom);
}
