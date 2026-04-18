/**
 * @file RuntimeNode.h
 * @brief Classe de base pour tous les nœuds du réseau
 *
 * Classe abstraite représentant un nœud dans le graphe de données.
 * Gère les connexions entre nœuds, l'exécution et le cache des résultats.
 *
 * Héritée par:
 * - SourceNode: point d'entrée des données
 * - TransformationNode: transformation des données
 * - MergeNode: fusion de plusieurs sources
 * - TargetNode: point de sortie des données
 *
 * Namespace: dn::nodes
 */

#ifndef RUNTIMENODE_H
#define RUNTIMENODE_H

#include "../core/DataTable.h"
#include "../network/core/enums.h"
#include <QObject>
#include <QVector>
#include <QUuid>
#include <QMap>

using namespace dn::core;


namespace dn::nodes {

    class RuntimeNode : public QObject
    {
        Q_OBJECT

    public:
        /// Constructeur de base (pour SourceNode sans parent)
        explicit RuntimeNode(QObject *parent = nullptr);

        /// Constructeur pour nœud avec UN parent (TransformationNode, TargetNode)
        explicit RuntimeNode(RuntimeNode* parent, QObject *qtParent = nullptr);

        /// Constructeur pour nœud avec PLUSIEURS parents (MergeNode)
        explicit RuntimeNode(const QVector<RuntimeNode*>& parents, QObject *qtParent = nullptr);
        virtual ~RuntimeNode() = default;

        //══════════════════════════════════════════════════════════════════
        // Métadonnées
        //══════════════════════════════════════════════════════════════════

        /// Retourne l'identifiant unique du nœud
        QUuid getId() const { return m_id; }

        /// Définit le type de nœud (Source, Transform, Merge, Target)
        void setNodeType(NodeType type) { m_nodeType = type; }

        /// Retourne le type de nœud
        NodeType getNodeType() const { return m_nodeType; }

        /// Définit le nom affiché du nœud
        void setName(const QString& name) { m_name = name; }

        /// Retourne le nom du nœud
        QString getName() const { return m_name; }

        /// Définit un paramètre
        void setParameter(const QString& key, const QString& value);

        /// Retourne un paramètre
        QString getParameter(const QString& key) const;

        /// Retourne tous les paramètres
        const QMap<QString, QString>& getAllParameters() const { return m_params; }

        //══════════════════════════════════════════════════════════════════
        // Interface d'entrées
        //══════════════════════════════════════════════════════════════════

        /// Retourne le nombre d'entrées du nœud
        virtual int inputCount() const = 0;

        /// Définit l'entrée (pour nœuds avec 1 entrée)
        virtual void setInput(RuntimeNode* node);

        /// Retourne l'entrée (pour nœuds avec 1 entrée)
        virtual RuntimeNode* getInput() const;

        /// Définit une entrée par index (pour MergeNode)
        virtual void setInput(int index, RuntimeNode* node);

        /// Retourne une entrée par index
        virtual RuntimeNode* getInput(int index) const;

        /// Retourne le nombre d'entrées connectées
        virtual int getInputsCount() const { return m_inputs.size(); }

        /// Retourne le niveau du nœud dans le graphe
        int getNodeLevel() {return m_nodeLevel;};

        //══════════════════════════════════════════════════════════════════
        // Exécution et résultats
        //══════════════════════════════════════════════════════════════════

        /// Retourne le résultat mis en cache
        const DataTable& getCachedResult() const { return m_cachedResult; }

        /// Exécute le calcul du nœud
        virtual void compute()=0;

    signals:
        /// Émis quand les données changent
        void tableChanged(const dn::core::DataTable& newTable);

        /// Émis quand une entrée est connectée
        void inputConnected(int index, dn::nodes::RuntimeNode* source);

        /// Émis quand une entrée est déconnectée
        void inputDisconnected(int index);

        /// Émis quand un paramètre change
        void parameterChanged(const QString& key, const QString& value);

    protected:
        /// Retourne les tables des entrées connectées
        QVector<const DataTable*> getInputTables();

        /// Met en cache les résultats des entrées
        virtual void setCachedResult(const QVector<const dn::core::DataTable*>& inputs) = 0;

        /// Définit le niveau du nœud dans le graphe
        int m_nodeLevel=0;
        void setNodeLevel(int level);

        /// Configure la connexion avec un parent
        void setupParentConnection(RuntimeNode* parent);

        /// Configure les connexions avec plusieurs parents
        void setupParentsConnections(const QVector<RuntimeNode*>& parents);

        /// Entrées connectées
        QVector<RuntimeNode*> m_inputs;

        /// Résultat mis en cache
        DataTable m_cachedResult;

    private:
        /// Identifiant unique
        QUuid m_id;

        /// Type de nœud
        NodeType m_nodeType = NodeType::Transform;

        /// Nom du nœud
        QString m_name;

        /// Paramètres de configuration
        QMap<QString, QString> m_params;

    };

}

#endif // RUNTIMENODE_H
