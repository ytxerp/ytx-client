#include "searchdialog.h"

#include <QHeaderView>

#include "component/enumclass.h"
#include "component/signalblocker.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(
    CTreeModel* tree, SearchNodeModel* search_node, SearchEntryModel* search_entry, CSectionConfig& config, CSectionInfo& info, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SearchDialog)
    , search_node_ { search_node }
    , search_entry_ { search_entry }
    , tree_model_ { tree }
    , config_ { config }
    , info_ { info }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniView(ui->searchViewNode);
    IniView(ui->searchViewEntry);

    ui->searchViewNode->setModel(search_node);
    ui->searchViewEntry->setModel(search_entry);

    ResizeTreeColumn(ui->searchViewNode->horizontalHeader());
    ResizeTableColumn(ui->searchViewEntry->horizontalHeader());

    InitDelegate();
    IniContentGroup();
    IniConnect();
    IniDialog();
    HideTableColumn(ui->searchViewEntry);
    HideTreeColumn(ui->searchViewNode);
}

SearchDialog::~SearchDialog() { delete ui; }

void SearchDialog::IniDialog()
{
    ui->rBtnNode->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);
    ui->pBtnClose->setAutoDefault(false);
    this->setWindowTitle(tr("Search"));
}

void SearchDialog::IniConnect()
{
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &SearchDialog::RSearch);
    connect(ui->searchViewNode, &QTableView::doubleClicked, this, &SearchDialog::RNodeDoubleClicked);
    connect(ui->searchViewEntry, &QTableView::doubleClicked, this, &SearchDialog::REntryDoubleClicked);
    connect(content_group_, &QButtonGroup::idClicked, this, &SearchDialog::RContentGroup);
}

void SearchDialog::IniContentGroup()
{
    content_group_ = new QButtonGroup(this);
    content_group_->addButton(ui->rBtnNode, 0);
    content_group_->addButton(ui->rBtnEntry, 1);
}

void SearchDialog::InitDelegate()
{
    value_ = new DoubleSpinNoneZeroR(config_.amount_decimal, kCoefficient8, this);
    rate_ = new DoubleSpinNoneZeroR(config_.rate_decimal, kCoefficient8, this);
    unit_ = new IntStringR(info_.unit_map, this);
    direction_rule_ = new BoolStringR(info_.rule_map, this);
    kind_ = new IntStringR(info_.kind_map, this);
    tree_path_ = new SearchPathTreeR(tree_model_, std::to_underlying(NodeEnum::kId), this);
    check_ = new StatusR(this);
    color_ = new ColorR(this);
    table_path_ = new SearchPathTableR(tree_model_, this);
}

void SearchDialog::HideTreeColumn(QTableView* view)
{
    view->setColumnHidden(std::to_underlying(NodeEnum::kId), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kUserId), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kCreateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kCreateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateBy), kIsHidden);
}

void SearchDialog::HideTableColumn(QTableView* view)
{
    view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUserId), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kCreateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kCreateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateBy), kIsHidden);
}

void SearchDialog::IniView(QTableView* view)
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->verticalHeader()->setHidden(true);
}

void SearchDialog::ResizeTreeColumn(QHeaderView* header)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(NodeUtils::DescriptionColumn(info_.section), QHeaderView::Stretch);
}

void SearchDialog::ResizeTableColumn(QHeaderView* header)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(std::to_underlying(EntryEnum::kDescription), QHeaderView::Stretch);
}

void SearchDialog::RSearch()
{
    CString text { ui->lineEdit->text() };

    if (ui->rBtnNode->isChecked()) {
        search_node_->Search(text);
        ResizeTreeColumn(ui->searchViewNode->horizontalHeader());
    }

    if (ui->rBtnEntry->isChecked()) {
        search_entry_->Search(text);
        ResizeTableColumn(ui->searchViewEntry->horizontalHeader());
    }
}

void SearchDialog::RNodeDoubleClicked(const QModelIndex& index)
{
    auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    emit SNodeLocation(node_id);
}

void SearchDialog::RContentGroup(int id) { ui->stackedWidget->setCurrentIndex(id); }
