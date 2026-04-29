#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QUuid>
#include <memory>
#include "nodes/RuntimeNode.h"
#include "nodes/SourceNode.h"
#include "nodes/TransformationNode.h"
#include "nodes/MergeNode.h"
#include "connectors/DataConnector.h"
#include "transformations/Transformation.h"
#include "nodes/TargetNode.h"

using namespace dn::nodes;

namespace dn::network {

    class Network : public QObject
    {
        Q_OBJECT

    public:
        explicit Network(QObject *parent = nullptr);
        ~Network();

        // ========== CRÉATION DE NŒUDS ==========
        SourceNode* createSourceNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                                     const QString& name = "Source");

        TransformationNode* createTransformationNode(dn::transformations::Transformation* transform,
                                                     RuntimeNode* parent,
                                                     const QString& name = "Transformation");

        MergeNode* createMergeNode(const QVector<RuntimeNode*>& parents,
                                   MergeType type = MergeType::Union,
                                   const QStringList& joinKeys = QStringList(),
                                   const QString& name = "Merge");

        TargetNode* createTargetNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                                     RuntimeNode* parent,
                                     const QString& name = "Target");

        // ========== GESTION DES NŒUDS ==========
        void removeNode(RuntimeNode* node);
        RuntimeNode* findNode(QUuid id) const;
        QVector<RuntimeNode*> getAllNodes() const { return m_nodes; }

        // ========== CONNEXIONS ==========
        // Connexion
        bool addConnection(RuntimeNode* from, RuntimeNode* to, int inputSlot = 0);
        bool disconnectNodes(RuntimeNode* target, int inputSlot = 0);

        // Obtenir les successeurs d'un nœud
        QVector<RuntimeNode*> getSuccessors(RuntimeNode* node) const;

        // ========== EXÉCUTION ==========
        // Exécute tous les nœuds dans l'ordre topologique
        void computeAll();

        // Exécute à partir d'un nœud spécifique
        void computeFrom(RuntimeNode* startNode);

        // Exécute un nœud spécifique et ses dépendances
        void computeNode(RuntimeNode* node);

        // ========== VALIDATION ==========
        bool validate() const;
        QVector<RuntimeNode*> findRootNodes() const;
        QVector<RuntimeNode*> findLeafNodes() const;

        // ========== SÉRIALISATION ==========
        QJsonObject toJson() const;
        bool fromJson(const QJsonObject& json);

        // ========== DEBUG ==========
        void debugPrint() const;

    signals:
        void nodeAdded(dn::nodes::RuntimeNode* node);
        void nodeRemoved(dn::nodes::RuntimeNode* node);

        void networkChanged();

    private:
        // Calcul de l'ordre topologique
        QVector<RuntimeNode*> computeTopologicalOrder() const;
        void collectDependencies(RuntimeNode* node, QSet<RuntimeNode*>& visited,
                                 QVector<RuntimeNode*>& order) const;

        // Vérification des cycles
        bool hasCycle() const;
        bool hasCycleUtil(RuntimeNode* node, QSet<RuntimeNode*>& visited,
                          QSet<RuntimeNode*>& recursionStack) const;

        RuntimeNode* createNodeFromJson(NodeType type,
                           const QString& name,
                           const QJsonObject& params,
                                        const QPointF& position);

        QVector<RuntimeNode*> m_nodes;
        QHash<QUuid, RuntimeNode*> m_nodeMap;
    };

}

#endif // NETWORK_H
