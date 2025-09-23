# Struct Map

This document provides a reference for mapping **Qt data structures** to their corresponding **PostgreSQL fields** used in the application.

- The **..** symbol indicates that the current cell or field is the same as the one in the first group (i.e., it inherits or repeats the same value from above).
- Rows labeled with **NodeEnumX** or **TransEnumX** indicate the enumeration for different **sections** of the system, corresponding to the table columns in that module.
- The **Qt** row shows the type used in the client application.
- The **PgSql** row shows the database type in PostgreSQL.
- Additional notes and special fields, such as `unit_price`, `is_finished`, etc., are listed in the corresponding table cells for clarity.

## Node

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

## Trans

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
