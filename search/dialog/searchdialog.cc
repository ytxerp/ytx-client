#include "searchdialog.h"

#include <QHeaderView>

#include "component/signalblocker.h"
#include "enum/entryenum.h"
#include "ui_searchdialog.h"
#include "utils/entryutils.h"
#include "utils/templateutils.h"

SearchDialog::SearchDialog(SectionContext* sc, SearchNodeModel* search_node, SearchEntryModel* search_entry, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SearchDialog)
    , search_node_ { search_node }
    , search_entry_ { search_entry }
    , tree_model_ { sc->tree_model }
    , config_ { sc->section_config }
    , info_ { sc->info }
    , tag_hash_ { sc->tag_hash }
    , tag_pixmap_ { sc->tag_pixmap }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniView(ui->searchViewNode);
    IniView(ui->searchViewEntry);

    ui->searchViewNode->setModel(search_node);
    search_node->setParent(ui->searchViewNode);

    ui->searchViewEntry->setModel(search_entry);
    search_entry->setParent(ui->searchViewEntry);

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
    ui->lineEditEntry->setVisible(false);

    ui->stackedWidget->setCurrentIndex(0);
    ui->pBtnClose->setAutoDefault(false);
    this->setWindowTitle(tr("Search"));
}

void SearchDialog::IniConnect()
{
    connect(ui->lineEditNode, &QLineEdit::returnPressed, this, &SearchDialog::RSearchNode);
    connect(ui->lineEditEntry, &QLineEdit::returnPressed, this, &SearchDialog::RSearchEntry);
    connect(ui->searchViewNode, &QTableView::doubleClicked, this, &SearchDialog::RNodeDoubleClicked);
    connect(ui->searchViewEntry, &QTableView::doubleClicked, this, &SearchDialog::REntryDoubleClicked);
    connect(content_group_, &QButtonGroup::idClicked, this, &SearchDialog::RContentGroup);
}

void SearchDialog::TableViewDelegate(QTableView* view)
{
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kLhsDebit), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kRhsDebit), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kLhsCredit), quantity_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kRhsCredit), quantity_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kLhsRate), rate_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kRhsRate), rate_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kLhsNode), table_path_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kRhsNode), table_path_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kStatus), check_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kIssuedTime), issued_time_);

    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kDocument), document_);
    view->setItemDelegateForColumn(std::to_underlying(FullEntryEnum::kTag), tag_);
}

void SearchDialog::IniContentGroup()
{
    content_group_ = new QButtonGroup(this);
    content_group_->addButton(ui->rBtnNode, 0);
    content_group_->addButton(ui->rBtnEntry, 1);
}

void SearchDialog::InitDelegate()
{
    amount_ = new DoubleSpinNoneZeroR(config_.amount_decimal, kCoefficient8, this);
    rate_ = new DoubleSpinNoneZeroR(config_.rate_decimal, kCoefficient8, this);
    quantity_ = new DoubleSpinNoneZeroR(config_.quantity_decimal, kCoefficient8, this);
    unit_ = new IntStringR(info_.unit_map, this);
    direction_rule_ = new BoolStringR(info_.rule_map, this);
    kind_ = new IntStringR(info_.kind_map, this);
    tree_path_ = new SearchPathTreeR(tree_model_, std::to_underlying(NodeEnum::kId), this);
    check_ = new StatusR(this);
    color_ = new ColorR(this);
    table_path_ = new SearchPathTableR(tree_model_, this);
    issued_time_ = new IssuedTimeR(config_.date_format, this);
    document_ = new DocumentR(this);
    int_ = new Int(0, 36500, this);
    tag_ = new TagDelegate(tag_hash_, tag_pixmap_, this);
}

void SearchDialog::HideTreeColumn(QTableView* view)
{
    view->setColumnHidden(std::to_underlying(NodeEnum::kId), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kUserId), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kCreateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kCreateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kUpdateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(NodeEnum::kVersion), kIsHidden);

    if (info_.section == Section::kSale || info_.section == Section::kPurchase) {
        view->setColumnHidden(std::to_underlying(NodeEnumO::kIsSettled), kIsHidden);
        view->setColumnHidden(std::to_underlying(NodeEnumO::kSettlementId), kIsHidden);
    }
}

void SearchDialog::HideTableColumn(QTableView* view)
{
    view->setColumnHidden(std::to_underlying(EntryEnum::kId), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUserId), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kCreateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kCreateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateTime), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kUpdateBy), kIsHidden);
    view->setColumnHidden(std::to_underlying(EntryEnum::kVersion), kIsHidden);

    if (info_.section == Section::kSale || info_.section == Section::kPurchase) {
        view->setColumnHidden(std::to_underlying(EntryEnumO::kExternalSku), kIsHidden);
    }
}

void SearchDialog::IniView(QTableView* view)
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    Utils::SetupVerticalHeader(view, kRowHeight);
}

void SearchDialog::ResizeTreeColumn(QHeaderView* header)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(Utils::NodeDescriptionColumn(info_.section), QHeaderView::Stretch);
}

void SearchDialog::ResizeTableColumn(QHeaderView* header)
{
    const int description_column { Utils::SearchEntryDescriptionColumn(info_.section) };
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(description_column, QHeaderView::Stretch);
}

void SearchDialog::RSearchNode()
{
    search_node_->Search(ui->lineEditNode->text().trimmed());
    ResizeTreeColumn(ui->searchViewNode->horizontalHeader());
}

void SearchDialog::RSearchEntry()
{
    search_entry_->Search(ui->lineEditEntry->text().trimmed());
    ResizeTableColumn(ui->searchViewEntry->horizontalHeader());
}

void SearchDialog::RNodeDoubleClicked(const QModelIndex& index)
{
    auto node_id { index.siblingAtColumn(std::to_underlying(NodeEnum::kId)).data().toUuid() };
    if (node_id.isNull())
        return;

    emit SNodeLocation(info_.section, node_id);
}

void SearchDialog::RContentGroup(int id)
{
    {
        const bool is_node { id == 0 };
        ui->lineEditNode->setVisible(is_node);
        ui->lineEditEntry->setVisible(!is_node);
    }

    ui->stackedWidget->setCurrentIndex(id);
}
