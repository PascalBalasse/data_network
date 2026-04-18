/**
 * @file SQLConnector.cpp
 * @brief Implémentation du connecteur SQL
 *
 * Établit une connexion à une base de données via Qt SQL.
 * Supporte SQLite, MySQL, PostgreSQL, etc.
 *
 * @see SQLConnector.h
 */

#include "SQLConnector.h"
#include "../core/DataTable.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QVariant>
#include <QDebug>
#include <QSqlError>

using namespace dn::connectors;
using namespace dn::core;

SQLConnector::SQLConnector(QObject *parent)
    : DataConnector(parent)
{
}

/// Configure la connexion et la requête
void SQLConnector::configure(const QMap<QString, QVariant>& params)
{
    m_connectionName = params.value("connectionName", "").toString();
    m_databaseType = params.value("databaseType", "QSQLITE").toString();
    m_tableName = params.value("tableName", "").toString();
    m_sqlQuery = params.value("sqlQuery", "").toString();

    // Extrait les paramètres de connexion restants
    m_connectionParams.clear();
    for (auto it = params.begin(); it != params.end(); ++it) {
        if (it.key() != "connectionName" && it.key() != "databaseType" &&
            it.key() != "tableName" && it.key() != "sqlQuery") {
            m_connectionParams[it.key()] = it.value().toString();
        }
    }
}

std::unique_ptr<DataTable> SQLConnector::load()
{
    if (m_tableName.isEmpty() && m_sqlQuery.isEmpty()) {
        qWarning() << "No table name or SQL query specified";
        return nullptr;
    }

    QString connectionName = m_connectionName.isEmpty() ?
        QSqlDatabase::defaultConnection : m_connectionName;

    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(m_databaseType, connectionName);

    if (m_connectionParams.contains("hostName"))
        db.setHostName(m_connectionParams["hostName"]);
    if (m_connectionParams.contains("databaseName"))
        db.setDatabaseName(m_connectionParams["databaseName"]);
    if (m_connectionParams.contains("userName"))
        db.setUserName(m_connectionParams["userName"]);
    if (m_connectionParams.contains("password"))
        db.setPassword(m_connectionParams["password"]);
    if (m_connectionParams.contains("port"))
        db.setPort(m_connectionParams["port"].toInt());

    if (!db.open()) {
        qWarning() << "Cannot open database:" << db.lastError().text();
        QSqlDatabase::removeDatabase(connectionName);
        return nullptr;
    }

    QString queryStr;
    if (!m_sqlQuery.isEmpty()) {
        queryStr = m_sqlQuery;
    } else {
        queryStr = QString("SELECT * FROM %1").arg(m_tableName);
    }

    QSqlQuery query(db);
    if (!query.exec(queryStr)) {
        qWarning() << "SQL query failed:" << query.lastError().text();
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
        return nullptr;
    }

    auto table = std::make_unique<DataTable>();

    QSqlRecord record = query.record();
    QStringList headers;
    for (int i = 0; i < record.count(); ++i) {
        headers << record.fieldName(i);
    }
    table->setColumnNames(headers);

    while (query.next()) {
        QList<QVariant> row;
        for (int i = 0; i < record.count(); ++i) {
            QVariant value = query.value(i);
            row.append(value);
        }
        table->addRow(row);
    }

    db.close();
    QSqlDatabase::removeDatabase(connectionName);

    table->autoDetectAndConvertTypes();

    qDebug() << "SQL Loaded:" << table->rowCount() << "rows,"
             << table->columnCount() << "columns";

    return table;
}
