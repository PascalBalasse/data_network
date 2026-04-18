/**
 * @file JSONConnector.h
 * @brief Connecteur pour les fichiers JSON
 *
 * Permet la lecture et écriture de données au format JSON.
 * Supporte les types complexes: tableaux, objets, tables imbriquées.
 *
 * Paramètres:
 * - "fileName": Chemin du fichier JSON
 * - "prettyPrint": Formatage lisible (par défaut: true)
 *
 * Namespace: dn::connectors
 */

#ifndef JSONCONNECTOR_H
#define JSONCONNECTOR_H

#include "DataConnector.h"
#include <QString>

namespace dn::connectors {

    class JSONConnector : public DataConnector {
    public:
        explicit JSONConnector(QObject *parent = nullptr) : DataConnector(parent) {}

        /// @brief Lit un fichier JSON et retourne un DataTable
        std::unique_ptr<dn::core::DataTable> load() override;

        /// @brief Écrit un DataTable en JSON
        bool write(const dn::core::DataTable& data) override;

        /// @brief Configure le connecteur
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return true; }
        bool supportsReading() const override { return true; }

    private:
        QString m_fileName;
        bool m_prettyPrint = true;

        dn::core::DataTable parseJson(const QJsonDocument& doc);
        QList<QVariant> parseJsonArray(const QJsonArray& array);
        QVariant parseJsonValue(const QJsonValue& value);
        QJsonValue jsonValueFromVariant(const QVariant& value);
        void detectAndSetComplexColumnTypes(dn::core::DataTable& table);
    };

}

#endif // JSONCONNECTOR_H