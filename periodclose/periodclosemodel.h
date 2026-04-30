#ifndef PERIODCLOSEMODEL_H
#define PERIODCLOSEMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "table/entry.h"

class PeriodCloseModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit PeriodCloseModel(CSectionInfo& info, QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    void sort(int column, Qt::SortOrder order) override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

public:
    void ResetModel(const QList<Entry*>& list);

private:
    CSectionInfo& info_;
    EntryList list_ {};
};

#endif // PERIODCLOSEMODEL_H
