#pragma once

#include <QLineEdit>

#include "styleditemdelegate.h"

class FilterLineEditDelegate final : public StyledItemDelegate {
    Q_OBJECT

public:
    explicit FilterLineEditDelegate(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const override;
};