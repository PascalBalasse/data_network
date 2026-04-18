/**
 * @file TargetNode.h
 * @brief Nœud cible - point de sortie des données
 *
 * Représente un nœud cible qui écrit les données vers un connecteur
 * (fichier CSV, Excel, JSON, base de données, etc.).
 * C'est le point de sortie du réseau de données.
 *
 * Parent: RuntimeNode
 * Entrée: 1 (reçoit les données d'un autre nœud)
 *
 * Namespace: dn::nodes
 */

#ifndef TARGETNODE_H
#define TARGETNODE_H

#include "RuntimeNode.h"
#include "../connectors/DataConnector.h"

namespace dn::nodes {

    class TargetNode : public RuntimeNode
    {
        Q_OBJECT

    public:
        explicit TargetNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                            RuntimeNode* parent,
                            const QString& name = "Target",
                            QObject *qtParent = nullptr);

        /// Une entrée (reçoit les données d'un nœud amont)
        int inputCount() const override { return 1; }

        /// Écrit les données vers le connecteur
        void compute() override;

        /// Reçoit les résultat du nœud amont
        void setCachedResult(const QVector<const dn::core::DataTable*>& inputs) override;

        /// Configure le connecteur sans UI
        void configureConnector(const QMap<QString, QVariant>& params);

        /// @brief Retourne le connecteur configuré
        dn::connectors::DataConnector* getConnector() const { return m_connector.get(); }

    signals:
        /// Émis après l'exportation
        void exportCompleted(bool success, const QString& message);

    private:
        std::unique_ptr<dn::connectors::DataConnector> m_connector;
        dn::core::DataTable m_cachedInput;
        bool exportData();
    };

}

#endif // TARGETNODE_H
