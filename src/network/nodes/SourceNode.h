/**
 * @file SourceNode.h
 * @brief Nœud source - point d'entrée des données
 *
 * Représente un nœud source qui lit les données depuis un connecteur
 * (fichier CSV, Excel, JSON, base de données, etc.).
 * C'est le point d'entrée du réseau de données.
 *
 * Parent: RuntimeNode
 *
 * Namespace: dn::nodes
 */

#ifndef SOURCENODE_H
#define SOURCENODE_H

#include "RuntimeNode.h"
#include "../connectors/DataConnector.h"

namespace dn::nodes {

    class SourceNode : public RuntimeNode
    {
        Q_OBJECT

    public:
        explicit SourceNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                            const QString& name = "Source",
                            QObject *parent = nullptr);

        /// Pas d'entrées (c'est un nœud source)
        int inputCount() const override { return 0; }

        /// Charge les données depuis le connecteur
        void compute() override;

        /// Reçoit les résultats mis en cache (non utilisé pour les sources)
        void setCachedResult(const QVector<const dn::core::DataTable*>& inputs) override;

        /// @brief Retourne le connecteur configuré
        dn::connectors::DataConnector* getConnector() const { return m_connector.get(); }

    private:
        std::unique_ptr<dn::connectors::DataConnector> m_connector;

        dn::core::DataTable loadData();
    };

}

#endif // SOURCENODE_H
