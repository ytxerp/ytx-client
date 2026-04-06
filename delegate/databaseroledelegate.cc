#include "databaseroledelegate.h"

#include "widget/combobox.h"

DatabaseRoleDelegate::DatabaseRoleDelegate(
    const QHash<QString, QString>& database_role_hash, const QList<QPair<QString, QString>>& database_role_list, QObject* parent)
    : StyledItemDelegate { parent }
    , database_role_hash_ { database_role_hash }
    , database_role_list_ { database_role_list }
{
}

QWidget* DatabaseRoleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };

    for (const auto& [key, display] : database_role_list_)
        editor->addItem(display, key); // key stored as UserData

    return editor;
}

void DatabaseRoleDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const QString key { index.data().toString() };
    const int idx { cast_editor->findData(key) };

    if (idx != -1)
        cast_editor->setCurrentIndex(idx);
}

void DatabaseRoleDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    const QString key { cast_editor->currentData().toString() };
    model->setData(index, key);
}

void DatabaseRoleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString key { index.data().toString() };
    if (!database_role_hash_.contains(key))
        return PaintEmpty(painter, option, index);

    const QString text { database_role_hash_.value(key) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize DatabaseRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QString key { index.data().toString() };
    const QString text { database_role_hash_.value(key) };
    return CalculateTextSize(text, option);
}

void DatabaseRoleDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };

    const QString key { index.data().toString() };
    const QString text { database_role_hash_.value(key) };

    const QSize text_size { CalculateTextSize(text, option) };

    const int width { std::max(option.rect.width(), text_size.width() + bar_width) };
    const int height { std::max(option.rect.height(), text_size.height()) };

    QRect geom { option.rect };
    geom.setWidth(width);
    geom.setHeight(height);

    editor->setGeometry(geom);
}
