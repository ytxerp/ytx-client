#include "sqlite.h"

#include <QSqlError>
#include <QSqlQuery>

#include "component/constvalue.h"

bool Sqlite::CreateDatabase(QSqlDatabase& db, CString database, CString owner)
{
    if (!IsValidPgIdentifier(database)) {
        qDebug() << "Invalid database name:" << database;
        return false;
    }

    if (!IsValidPgIdentifier(owner)) {
        qDebug() << "Invalid owner name:" << owner;
        return false;
    }

    QString sql = QString("CREATE DATABASE %1 OWNER %2;").arg(database, owner);

    QSqlQuery query(db);

    if (!query.exec(QString("SELECT 1 FROM pg_database WHERE datname = '%1';").arg(database))) {
        qDebug() << "Check database existence failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return true;
    }

    if (!query.exec(sql)) {
        qDebug() << "Error creating database:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Sqlite::NewFile(const QString& file_path)
{
    QSqlDatabase db { QSqlDatabase::addDatabase(kQSQLITE) };
    db.setDatabaseName(file_path);
    if (!db.open())
        return false;

    const std::vector<QString> tables { NodeFinance(), Path(kFinance), TransFinance(), NodeProduct(), Path(kProduct), TransProduct(), NodeStakeholder(),
        Path(kStakeholder), TransStakeholder(), NodeTask(), Path(kTask), TransTask(), NodeOrder(kPurchase), Path(kPurchase), TransOrder(kPurchase),
        SettlementOrder(kPurchase), NodeOrder(kSales), Path(kSales), TransOrder(kSales), SettlementOrder(kSales) };

    QSqlQuery query { db };

    if (!db.transaction()) {
        qDebug() << "Error starting transaction:" << db.lastError().text();
        return false;
    }

    for (const auto& table : tables) {
        if (!query.exec(table)) {
            qDebug() << "Error executing query:" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        qDebug() << "Error committing transaction:" << db.lastError().text();
        db.rollback();
        return false;
    }

    db.close();
    return true;
}

/**
 * Server-Generated Fields Notice
 *
 * The following fields are strictly generated and managed by the server:
 *
 * - created_time, updated_time
 *   Ensures consistent timestamps across clients and enables proper auditing.
 *
 * - created_by, updated_by
 *   Prevents identity spoofing; reflects authenticated user context on the server.
 *
 * - user_id
 *   Tied to the authenticated session; recommended to be consistent with created_by.
 *
 * Clients MUST NOT generate or override these fields.
 * They are returned by the server and should only be stored or displayed locally.
 */

QString Sqlite::NodeFinance()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS finance (
        id               BLOB PRIMARY KEY,
        name             TEXT ,
        code             TEXT,
        description      TEXT,
        note             TEXT,
        node_type        INTEGER ,
        direction_rule   BOOLEAN  DEFAULT FALSE,
        unit             INTEGER ,
        foreign_total    NUMERIC,
        local_total      NUMERIC,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version          INTEGER  DEFAULT 1,
        is_valid         BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeProduct()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS product (
        id               BLOB PRIMARY KEY,
        name             TEXT ,
        code             TEXT,
        description      TEXT,
        note             TEXT,
        node_type        INTEGER ,
        direction_rule   BOOLEAN  DEFAULT FALSE,
        unit             INTEGER ,
        color            TEXT,
        unit_price       NUMERIC,
        commission       NUMERIC,
        quantity         NUMERIC,
        amount           NUMERIC,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version          INTEGER  DEFAULT 1,
        is_valid         BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeTask()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS task (
        id               BLOB PRIMARY KEY,
        name             TEXT ,
        code             TEXT,
        description      TEXT,
        note             TEXT,
        node_type        INTEGER ,
        direction_rule   BOOLEAN  DEFAULT FALSE,
        unit             INTEGER ,
        issued_time      TEXT ,
        color            TEXT,
        document         TEXT,
        is_finished      BOOLEAN  DEFAULT FALSE,
        unit_cost        NUMERIC,
        quantity         NUMERIC,
        amount           NUMERIC,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version          INTEGER  DEFAULT 1,
        is_valid         BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeStakeholder()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS stakeholder (
        id                BLOB PRIMARY KEY,
        name              TEXT ,
        code              TEXT,
        description       TEXT,
        note              TEXT,
        node_type         INTEGER ,
        payment_term      INTEGER ,
        unit              INTEGER ,
        deadline          INTEGER CHECK (deadline BETWEEN 1 AND 31),
        employee          BLOB,
        tax_rate          NUMERIC,
        amount            NUMERIC,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version           INTEGER  DEFAULT 1,
        is_valid          BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeOrder(const QString& order)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1 (
        id                BLOB PRIMARY KEY,
        name              TEXT ,
        party             BLOB ,
        description       TEXT,
        employee          BLOB,
        node_type         INTEGER ,
        direction_rule    BOOLEAN  DEFAULT FALSE,
        unit              INTEGER ,
        issued_time       TEXT ,
        first             NUMERIC,
        second            NUMERIC,
        is_finished       BOOLEAN  DEFAULT FALSE,
        gross_amount      NUMERIC,
        discount          NUMERIC,
        settlement        NUMERIC,
        settlement_id     BLOB ,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version           INTEGER  DEFAULT 1,
        is_valid          BOOLEAN  DEFAULT TRUE
    );
    )")
        .arg(order);
}

QString Sqlite::Path(const QString& table_name)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1_path (
        ancestor      BLOB ,
        descendant    BLOB ,
        distance      INTEGER  CHECK (distance >= 0),
        user_id       BLOB ,
        version       INTEGER  DEFAULT 1
    );
)")
        .arg(table_name);
}

QString Sqlite::TransFinance()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS finance_transaction (
        id             BLOB PRIMARY KEY,
        issued_time    TEXT ,
        code           TEXT,
        lhs_node       BLOB ,
        lhs_ratio      NUMERIC CHECK (lhs_ratio   > 0),
        lhs_debit      NUMERIC CHECK (lhs_debit  >= 0),
        lhs_credit     NUMERIC CHECK (lhs_credit >= 0),
        description    TEXT,
        support_id     BLOB,
        document       TEXT,
        is_checked     BOOLEAN  DEFAULT FALSE,
        rhs_credit     NUMERIC CHECK (rhs_credit >= 0),
        rhs_debit      NUMERIC CHECK (rhs_debit  >= 0),
        rhs_ratio      NUMERIC CHECK (rhs_ratio   > 0),
        rhs_node       BLOB ,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version        INTEGER  DEFAULT 1,
        is_valid       BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::TransProduct()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS product_transaction (
        id             BLOB PRIMARY KEY,
        issued_time    TEXT ,
        code           TEXT,
        lhs_node       BLOB ,
        unit_cost      NUMERIC,
        lhs_debit      NUMERIC CHECK (lhs_debit  >= 0),
        lhs_credit     NUMERIC CHECK (lhs_credit >= 0),
        description    TEXT,
        support_id     BLOB,
        document       TEXT,
        is_checked     BOOLEAN  DEFAULT FALSE,
        rhs_credit     NUMERIC CHECK (rhs_credit >= 0),
        rhs_debit      NUMERIC CHECK (rhs_debit  >= 0),
        rhs_node       BLOB ,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version        INTEGER  DEFAULT 1,
        is_valid       BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::TransTask()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS task_transaction (
        id             BLOB PRIMARY KEY,
        issued_time    TEXT ,
        code           TEXT,
        lhs_node       BLOB ,
        unit_cost      NUMERIC,
        lhs_debit      NUMERIC CHECK (lhs_debit  >= 0),
        lhs_credit     NUMERIC CHECK (lhs_credit >= 0),
        description    TEXT,
        support_id     BLOB,
        document       TEXT,
        is_checked     BOOLEAN  DEFAULT FALSE,
        rhs_credit     NUMERIC CHECK (rhs_credit >= 0),
        rhs_debit      NUMERIC CHECK (rhs_debit  >= 0),
        rhs_node       BLOB ,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version        INTEGER  DEFAULT 1,
        is_valid       BOOLEAN  DEFAULT TRUE
    );
    )");
}

QString Sqlite::TransStakeholder()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS stakeholder_transaction (
        id                 BLOB PRIMARY KEY,
        issued_time        TEXT ,
        code               TEXT,
        lhs_node           BLOB ,
        unit_price         NUMERIC,
        description        TEXT,
        outside_product    BLOB,
        document           TEXT,
        is_checked         BOOLEAN  DEFAULT FALSE,
        inside_product     BLOB ,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version            INTEGER  DEFAULT 1,
        UNIQUE(lhs_node, inside_product)
    );
    )");
}

QString Sqlite::TransOrder(const QString& order)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1_transaction (
        id                  BLOB PRIMARY KEY,
        code                TEXT,
        lhs_node            BLOB ,
        unit_price          NUMERIC,
        first               NUMERIC,
        second              NUMERIC,
        description         TEXT,
        outside_product     BLOB,
        discount            NUMERIC,
        net_amount          NUMERIC,
        gross_amount        NUMERIC,
        discount_price      NUMERIC,
        inside_product      BLOB ,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version             INTEGER  DEFAULT 1,
        is_valid            BOOLEAN  DEFAULT TRUE
    );
    )")
        .arg(order);
}

QString Sqlite::SettlementOrder(const QString& order)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1_settlement (
        id               BLOB PRIMARY KEY,
        party            BLOB ,
        issued_time      TEXT ,
        description      TEXT,
        is_finished      BOOLEAN  DEFAULT FALSE,
        gross_amount     NUMERIC,
        user_id          BLOB ,
        created_time     TEXT ,
        created_by       BLOB ,
        updated_time     TEXT ,
        updated_by       BLOB ,
        version          INTEGER  DEFAULT 1,
        is_valid         BOOLEAN  DEFAULT TRUE
    );
    )")
        .arg(order);
}

#if 0
QString Sqlite::NodeFinance()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS finance (
        id               BLOB PRIMARY KEY,
        name             TEXT NOT NULL,
        code             TEXT,
        description      TEXT,
        note             TEXT,
        node_type        INTEGER NOT NULL,
        direction_rule   BOOLEAN NOT NULL DEFAULT FALSE,
        unit             INTEGER NOT NULL,
        foreign_total    NUMERIC,
        local_total      NUMERIC,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeProduct()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS product (
        id               BLOB PRIMARY KEY,
        name             TEXT NOT NULL,
        code             TEXT,
        description      TEXT,
        note             TEXT,
        node_type        INTEGER NOT NULL,
        direction_rule   BOOLEAN NOT NULL DEFAULT FALSE,
        unit             INTEGER NOT NULL,
        color            TEXT,
        unit_price       NUMERIC,
        commission       NUMERIC,
        quantity         NUMERIC,
        amount           NUMERIC,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeTask()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS task (
        id               BLOB PRIMARY KEY,
        name             TEXT NOT NULL,
        code             TEXT,
        description      TEXT,
        note             TEXT,
        node_type        INTEGER NOT NULL,
        direction_rule   BOOLEAN NOT NULL DEFAULT FALSE,
        unit             INTEGER NOT NULL,
        issued_time      TEXT NOT NULL,
        color            TEXT,
        document         TEXT,
        is_finished      BOOLEAN NOT NULL DEFAULT FALSE,
        unit_cost        NUMERIC,
        quantity         NUMERIC,
        amount           NUMERIC,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeStakeholder()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS stakeholder (
        id                BLOB PRIMARY KEY,
        name              TEXT NOT NULL,
        code              TEXT,
        description       TEXT,
        note              TEXT,
        node_type         INTEGER NOT NULL,
        payment_term      INTEGER NOT NULL,
        unit              INTEGER NOT NULL,
        deadline          INTEGER CHECK (deadline BETWEEN 1 AND 31),
        employee          BLOB,
        tax_rate          NUMERIC,
        amount            NUMERIC,
        user_id           BLOB NOT NULL,
        created_time      TEXT NOT NULL,
        created_by        BLOB NOT NULL,
        updated_time      TEXT NOT NULL,
        updated_by        BLOB NOT NULL,
        version           INTEGER NOT NULL DEFAULT 1,
        is_valid          BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::NodeOrder(const QString& order)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1 (
        id                BLOB PRIMARY KEY,
        name              TEXT NOT NULL,
        party             BLOB NOT NULL,
        description       TEXT,
        employee          BLOB,
        node_type         INTEGER NOT NULL,
        direction_rule    BOOLEAN NOT NULL DEFAULT FALSE,
        unit              INTEGER NOT NULL,
        issued_time       TEXT NOT NULL,
        first             NUMERIC,
        second            NUMERIC,
        is_finished       BOOLEAN NOT NULL DEFAULT FALSE,
        gross_amount      NUMERIC,
        discount          NUMERIC,
        settlement        NUMERIC,
        settlement_id     BLOB NOT NULL,
        user_id           BLOB NOT NULL,
        created_time      TEXT NOT NULL,
        created_by        BLOB NOT NULL,
        updated_time      TEXT NOT NULL,
        updated_by        BLOB NOT NULL,
        version           INTEGER NOT NULL DEFAULT 1,
        is_valid          BOOLEAN NOT NULL DEFAULT TRUE
    );
    )")
        .arg(order);
}

QString Sqlite::Path(const QString& table_name)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1_path (
        ancestor      BLOB NOT NULL,
        descendant    BLOB NOT NULL,
        distance      INTEGER NOT NULL CHECK (distance >= 0),
        user_id       BLOB NOT NULL,
        version       INTEGER NOT NULL DEFAULT 1
    );
)")
        .arg(table_name);
}

QString Sqlite::TransFinance()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS finance_transaction (
        id               BLOB PRIMARY KEY,
        issued_time      TEXT NOT NULL,
        code             TEXT,
        lhs_node         BLOB NOT NULL,
        lhs_ratio        NUMERIC CHECK (lhs_ratio   > 0),
        lhs_debit        NUMERIC CHECK (lhs_debit  >= 0),
        lhs_credit       NUMERIC CHECK (lhs_credit >= 0),
        description      TEXT,
        support_id       BLOB,
        document         TEXT,
        is_checked       BOOLEAN NOT NULL DEFAULT FALSE,
        rhs_credit       NUMERIC CHECK (rhs_credit >= 0),
        rhs_debit        NUMERIC CHECK (rhs_debit  >= 0),
        rhs_ratio        NUMERIC CHECK (rhs_ratio   > 0),
        rhs_node         BLOB NOT NULL,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::TransProduct()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS product_transaction (
        id               BLOB PRIMARY KEY,
        issued_time      TEXT NOT NULL,
        code             TEXT,
        lhs_node         BLOB NOT NULL,
        unit_cost        NUMERIC,
        lhs_debit        NUMERIC CHECK (lhs_debit  >= 0),
        lhs_credit       NUMERIC CHECK (lhs_credit >= 0),
        description      TEXT,
        support_id       BLOB,
        document         TEXT,
        is_checked       BOOLEAN NOT NULL DEFAULT FALSE,
        rhs_credit       NUMERIC CHECK (rhs_credit >= 0),
        rhs_debit        NUMERIC CHECK (rhs_debit  >= 0),
        rhs_node         BLOB NOT NULL,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::TransTask()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS task_transaction (
        id               BLOB PRIMARY KEY,
        issued_time      TEXT NOT NULL,
        code             TEXT,
        lhs_node         BLOB NOT NULL,
        unit_cost        NUMERIC,
        lhs_debit        NUMERIC CHECK (lhs_debit  >= 0),
        lhs_credit       NUMERIC CHECK (lhs_credit >= 0),
        description      TEXT,
        support_id       BLOB,
        document         TEXT,
        is_checked       BOOLEAN NOT NULL DEFAULT FALSE,
        rhs_credit       NUMERIC CHECK (rhs_credit >= 0),
        rhs_debit        NUMERIC CHECK (rhs_debit  >= 0),
        rhs_node         BLOB NOT NULL,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )");
}

QString Sqlite::TransStakeholder()
{
    return QStringLiteral(R"(
    CREATE TABLE IF NOT EXISTS stakeholder_transaction (
        id                 BLOB PRIMARY KEY,
        issued_time        TEXT NOT NULL,
        code               TEXT,
        lhs_node           BLOB NOT NULL,
        unit_price         NUMERIC,
        description        TEXT,
        outside_product    BLOB,
        document           TEXT,
        is_checked         BOOLEAN NOT NULL DEFAULT FALSE,
        inside_product     BLOB NOT NULL,
        user_id            BLOB NOT NULL,
        created_time       TEXT NOT NULL,
        created_by         BLOB NOT NULL,
        updated_time       TEXT NOT NULL,
        updated_by         BLOB NOT NULL,
        version            INTEGER NOT NULL DEFAULT 1,
        UNIQUE(lhs_node, inside_product)
    );
    )");
}

QString Sqlite::TransOrder(const QString& order)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1_transaction (
        id                  BLOB PRIMARY KEY,
        code                TEXT,
        lhs_node            BLOB NOT NULL,
        unit_price          NUMERIC,
        first               NUMERIC,
        second              NUMERIC,
        description         TEXT,
        outside_product     BLOB,
        discount            NUMERIC,
        net_amount          NUMERIC,
        gross_amount        NUMERIC,
        discount_price      NUMERIC,
        inside_product      BLOB NOT NULL,
        user_id             BLOB NOT NULL,
        created_time        TEXT NOT NULL,
        created_by          BLOB NOT NULL,
        updated_time        TEXT NOT NULL,
        updated_by          BLOB NOT NULL,
        version             INTEGER NOT NULL DEFAULT 1,
        is_valid            BOOLEAN NOT NULL DEFAULT TRUE
    );
    )")
        .arg(order);
}

QString Sqlite::SettlementOrder(const QString& order)
{
    return QString(R"(
    CREATE TABLE IF NOT EXISTS %1_settlement (
        id               BLOB PRIMARY KEY,
        party            BLOB NOT NULL,
        issued_time      TEXT NOT NULL,
        description      TEXT,
        is_finished      BOOLEAN NOT NULL DEFAULT FALSE,
        gross_amount     NUMERIC,
        user_id          BLOB NOT NULL,
        created_time     TEXT NOT NULL,
        created_by       BLOB NOT NULL,
        updated_time     TEXT NOT NULL,
        updated_by       BLOB NOT NULL,
        version          INTEGER NOT NULL DEFAULT 1,
        is_valid         BOOLEAN NOT NULL DEFAULT TRUE
    );
    )")
        .arg(order);
}
#endif
