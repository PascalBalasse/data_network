/**
 * @file SQLConnector.h
 * @brief Connecteur pour requêtes SQL vers une base de données
 *
 * Permet d'exécuter des requêtes SQL et de récupérer les résultats.
 * Supporte différent types de bases de données (SQLite, MySQL, PostgreSQL).
 *
 * Paramètres:
 * - "databaseType": Type de BDD (SQLite, MySQL, PostgreSQL)
 * - "connectionName": Nom de connexion
 * - "tableName": Nom de la table
 * - "sqlQuery": Requête SQL à exécuter
 *
 * Note: Lecture seule (pas d'écriture SQL)
 *
 * Namespace: dn::connectors
 */

#ifndef SQLCONNECTOR_H
#define SQLCONNECTOR_H

#include "DataConnector.h"
#include <QString>
#include <QVariant>

namespace dn::connectors {

    class SQLConnector : public DataConnector
    {
        Q_OBJECT

    public:
        explicit SQLConnector(QObject *parent = nullptr);

        /// @brief Exécute la requête SQL et retourne les résultats
        std::unique_ptr<dn::core::DataTable> load() override;

        /// @brief Configure la connexion et la requête
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return false; }

        QString getConnectionName() const { return m_connectionName; }
        QString getTableName() const { return m_tableName; }
        QString getSQLQuery() const { return m_sqlQuery; }
        QString getDatabaseType() const { return m_databaseType; }

    private:
        QString m_connectionName;
        QString m_databaseType;
        QString m_tableName;
        QString m_sqlQuery;
        QMap<QString, QString> m_connectionParams;
    };

}

#endif // SQLCONNECTOR_H
