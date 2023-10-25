# YTX Client

- [YTX Client](#ytx-client)
  - [Introduction](#introduction)
  - [Developer](#developer)
    - [Node](#node)
    - [Trans](#trans)
    - [Build](#build)
  - [User](#user)
    - [Description](#description)
    - [Action](#action)
  - [Support Me](#support-me)

## Introduction

**YTX** is a lightweight ERP software designed for small businesses, with full support for **multi-user online operation**. It provides a complete solution for managing daily operations across key modules such as **Finance**, **Item**, **Task**, **Sale**, **Purchase**, and **Stakeholder**.

Originally built as a desktop application, YTX has evolved into a **fully-featured online ERP system**. With the support of the companion **ytx-server backend service**, it can be deployed on a **local server** using PostgreSQL to enable **real-time collaboration** within teams, without relying on third-party cloud services.

With its **user-friendly interface** and **modular design**, YTX is an efficient and flexible tool for small enterprises seeking modern business management.

## Developer

- The **..** symbol indicates that current text matches the text in first group.
- **Note: The Sale and Purchase modules are currently a work in progress (WIP).**

### Node

| Node        |  name   |  id   |  code   | description  |  note   |  kind   | direction_rule |  unit   | initial_total | final_total |         |             |             |              |                |        |           |                |             |              |
| ----------- | :-----: | :---: | :-----: | :----------: | :-----: | :-----: | :------------: | :-----: | :-----------: | :---------: | :-----: | :---------: | :---------: | :----------: | :------------: | :----: | :-------: | :------------: | :---------: | :----------: |
| NodeEnum    |  kName  |  kId  |  kCode  | kDescription |  kNote  |  kKind  | kDirectionRule |  kUnit  | kInitialTotal | kFinalTotal |         |             |             |              |                |        |           |                |             |              |
|             |         |       |         |              |         |         |                |         |               |             |         |             |             |              |                |        |           |                |             |              |
| Qt          | QString | QUuid | QString |   QString    | QString |   int   |      bool      |   int   |    double     |   double    | QString |    bool     |   double    |    double    |     double     | QUuid  |   QUuid   |   QDateTime    | QStringList |     int      |
| PgSql       |  TEXT   | UUID  |  TEXT   |     TEXT     |  TEXT   | INTEGER |    BOOLEAN     | INTEGER |    NUMERIC    |   NUMERIC   |  TEXT   |   BOOLEAN   |   NUMERIC   |   NUMERIC    |    NUMERIC     |  UUID  |   UUID    | TIMESTAMPTZ(0) |    TEXT     |   INTEGER    |
|             |         |       |         |              |         |         |                |         |               |             |         |             |             |              |                |        |           |                |             |              |
| finance     |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |       ..       |   ..    |      ..       |     ..      |         |             |             |              |                |        |           |                |             |              |
| NodeEnumF   |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |       ..       |   ..    |      ..       |     ..      |         |             |             |              |                |        |           |                |             |              |
|             |         |       |         |              |         |         |                |         |               |             |         |             |             |              |                |        |           |                |             |              |
| item        |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |       ..       |   ..    |      ..       |     ..      |  color  |             | unit_price  |  commission  |                |        |           |                |             |              |
| NodeEnumI   |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |       ..       |   ..    |      ..       |     ..      | KColor  |             | kUnitPrice  | kCommission  |                |        |           |                |             |              |
|             |         |       |         |              |         |         |                |         |               |             |         |             |             |              |                |        |           |                |             |              |
| task        |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |       ..       |   ..    |      ..       |     ..      |  color  | is_finished |             |              |                |        |           |  issued_time   |  document   |              |
| NodeEnumT   |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |       ..       |   ..    |      ..       |     ..      | KColor  | kIsFinished |             |              |                |        |           |  kIssuedTime   |  kDocument  |              |
|             |         |       |         |              |         |         |                |         |               |             |         |             |             |              |                |        |           |                |             |              |
| stakeholder |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |                |   ..    |      ..       |     ..      |         |             |             |              |                |        |           |                |             | payment_term |
| NodeEnumS   |   ..    |  ..   |   ..    |      ..      |   ..    |   ..    |                |   ..    |      ..       |     ..      |         |             |             |              |                |        |           |                |             | kPaymentTerm |
|             |         |       |         |              |         |         |                |         |               |             |         |             |             |              |                |        |           |                |             |              |
| order       |   ..    |  ..   |         |      ..      |         |   ..    |       ..       |   ..    |      ..       |     ..      |         | is_finished | first_total | second_total | discount_total | party  | employee  |  issued_time   |             |              |
| NodeEnumO   |   ..    |  ..   |         |      ..      |         |   ..    |       ..       |   ..    |      ..       |     ..      |         | kIsFinished | kFirstTotal | kSecondTotal | kDiscountTotal | kParty | kEmployee |  kIssuedTime   |             |              |

| Settlement        |      |             |          |             |             |               |
| :---------------- | :--: | :---------: | :------: | :---------: | :---------: | :-----------: |
| Settlement        |  id  | issued_time |  party   | description | is_finished | initial_total |
| SettlementPrimary |  id  | issued_time | employee | description | is_finished | initial_total |

| Statement          |             |             |        |         |               |            |                |                 |            |
| :----------------- | :---------: | :---------: | :----: | :-----: | :-----------: | :--------: | :------------: | :-------------: | ---------- |
| Statement          |    party    |  pbalance   | cfirst | csecond | cgross_amount |  cbalance  |  csettlement   |                 |            |
| StatementPrimary   | issued_time | description | first  | second  | initial_total | is_checked |    employee    |   final_total   |            |
| StatementSecondary | issued_time | description | first  | second  |    initial    | is_checked | inside_product | outside_product | unit_price |

| TransRef |             |          |         |       |                 |       |        |            |                |             |         |
| -------- | :---------: | :------: | :-----: | :---: | :-------------: | :---: | :----: | :--------: | -------------- | ----------- | ------- |
| TransRef | issued_time | order_id | section | pp_id | outside_product | first | second | unit_price | discount_price | description | initial |

| TransSupport  |             |      |          |           |           |            |      |             |          |            |            |            |           |          |          |
| :------------ | :---------: | :--: | :------: | :-------: | :-------: | :--------: | :--: | :---------: | :------: | ---------- | :--------: | :--------: | :-------: | :------: | :------: |
| TransSupportF | issued_time |  id  | lhs_node | lhs_rate  | lhs_debit | lhs_credit | code | description | document | support_id | is_checked | rhs_credit | rhs_debit | rhs_rate | rhs_node |
| TransSupportI | issued_time |  id  | lhs_node | unit_cost | lhs_debit | lhs_credit | code | description | document | support_id | is_checked | rhs_credit | rhs_debit |          | rhs_node |
| TransSupportT | issued_time |  id  | lhs_node | unit_cost | lhs_debit | lhs_credit | code | description | document | support_id | is_checked | rhs_credit | rhs_debit |          | rhs_node |

### Trans

| Trans       |  issued_time   |  id   | lhs_node |  code   | description  |  document   | is_checked | support_node | rhs_node | lhs_debit | lhs_credit | rhs_credit | rhs_debit |            |                |          |
| ----------- | :------------: | :---: | :------: | :-----: | :----------: | :---------: | :--------: | :----------: | :------: | :-------: | :--------: | :--------: | :-------: | :--------: | :------------: | :------: |
| TransEnum   |  kIssuedTime   |  kId  | kLhsNode |  KCode  | kDescription |  kDocument  | kIsChecked | kSupportNode | kRhsNode | kLhsDebit | kLhsCredit | kRhsCredit | kRhsDebit |            |                |          |
|             |                |       |          |         |              |             |            |              |          |           |            |            |           |            |                |          |
| Qt          |   QDateTime    | QUuid |  QUuid   | QString |   QString    | QStringList |    bool    |    QUuid     |  QUuid   |  double   |   double   |   double   |  double   |   double   |     double     |  double  |
| PgSql       | TIMESTAMPTZ(0) | UUID  |   UUID   |  TEXT   |     TEXT     |    TEXT     |  BOOLEAN   |     UUID     |   UUID   |  NUMERIC  |  NUMERIC   |  NUMERIC   |  NUMERIC  |  NUMERIC   |    NUMERIC     | NUMERIC  |
|             |                |       |          |         |              |             |            |              |          |           |            |            |           |            |                |          |
| finance     |       ..       |  ..   |    ..    |   ..    |      ..      |     ..      |     ..     |      ..      |    ..    |    ..     |     ..     |     ..     |    ..     |  lhs_rate  |    rhs_rate    |          |
| TransEnumF  |       ..       |  ..   |    ..    |   ..    |      ..      |     ..      |     ..     |      ..      |    ..    |  kDebit   |  kCredit   |            |           |  kLhsRate  |                |          |
|             |                |       |          |         |              |             |            |              |          |           |            |            |           |            |                |          |
| item_task   |       ..       |  ..   |    ..    |   ..    |      ..      |     ..      |     ..     |      ..      |    ..    |    ..     |     ..     |     ..     |    ..     | unit_cost  |                |          |
| TransEnumIT |       ..       |  ..   |    ..    |   ..    |      ..      |     ..      |     ..     |      ..      |    ..    |  kDebit   |  kCredit   |            |           | kUnitCost  |                |          |
|             |                |       |          |         |              |             |            |              |          |           |            |            |           |            |                |          |
| stakeholder |       ..       |  ..   |    ..    |   ..    |      ..      |     ..      |     ..     |      ..      |    ..    |           |            |            |           | unit_price |                |          |
| TransEnumS  |       ..       |  ..   |    ..    |   ..    |      ..      |     ..      |     ..     |      ..      |    ..    |           |            |            |           | kUnitPrice |                |          |
|             |                |       |          |         |              |             |            |              |          |           |            |            |           |            |                |          |
| order       |                |  ..   |    ..    |         |      ..      |             |            |      ..      |    ..    |   first   |   second   |  initial   | discount  | unit_price | discount_price | balance  |
| TransEnumO  |                |  ..   |    ..    |         |      ..      |             |            |      ..      |    ..    |  kFirst   |  kSecond   |  kInitial  | kDiscount | kUnitPrice | kDiscountPrice | kBalance |

### Build

This is a pure Qt project. Only a compatible version of Qt and a compiler are required.

- Qt: 6.9+
    1. Desktop
    2. Additional Libraries
        - Qt Charts
        - Qt Image Formats
        - Developer and Designer Tools
    3. Qt Creator 15.X.X
- CMake: 3.19+
- Compiler: GCC 12+ or LLVM/Clang 14+

## User

### Description

1. Configuration Directory
    - QStandardPaths::AppConfigLocation
    - Mac: `~/Library/Preferences/YTX` (Show hidden folders: **`cmd + shift + .`**)
    - Win: `C:/Users/<USER>/AppData/Local/YTX`
    - Linux: `~/.config/YTX`

### Action

1. Preferences
    - Default Unit: Set the default unit.
2. Node
    - Insert: **`Alt + N`**, Append: **`Alt + P`**
    - DirectionRule(**R**)
        1. Define how balances are calculated in the entry table. New nodes inherit rules from their parent nodes by default.
        2. When properly configured, the total of all nodes is typically positive.
        3. Two common rules:
            - **DICD**: _Debit Increase, Credit Decrease_. Used for assets and expenses, calculated as "left side minus right side".
            - **DDCI**: _Debit Decrease, Credit Increase_. Used for liabilities, income, and owner’s equity, calculated as "right side minus left side".
    - node_type
        1. **B**: Branch nodes (grouping nodes; cannot record entry).
        2. **L**: Leaf nodes (used to record entry).
        3. **S**: Support nodes (easy viewing; cannot record entry).
    - Unit(**U**)
        - Inherits its parent node’s unit by default.
3. Entry
    - Append: **`Ctrl + N`**
    - Reference date and related node.
    - Date
        1. By default, the time is displayed and stored in the database with second-level precision.
        2. Shortcut keys (for English input method):
            - T: Now
            - J: Yesterday
            - K: Tomorrow
            - H: End of last month
            - L: First of next month
            - W: Last week
            - B: Next week
            - E: Last year
            - N: next year
    - Rate
      - Finance: Represents the conversion rate between the node’s unit and the base unit (e.g., 1 USD = 7.2 RMB).
    - Document(**D**)
        1. No restrictions on node_type and quantity.
        2. Local only, but files can be backed up via cloud services like Dropbox or Google Drive.
    - IsChecked(**C**)
        1. Used for reconciliation (sort by date, then reconcile).
    - Related Node
        1. If no related node is specified, the row will not be saved when the table is closed.
    - Debit, Credit, Balance
        1. Display values in the node’s current unit, where the base unit value = rate × node value.
4. Status bar
    - The middle section shows the node’s total value in the current unit.
    - The right section shows the result of operations between two nodes.

## Support Me

If YTX has been helpful to you, I’d be truly grateful for your support. Your encouragement helps me keep improving and creating!

Also may the force be with you!

[<img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" width="160" height="40">](https://buymeacoffee.com/ytx.cash)
