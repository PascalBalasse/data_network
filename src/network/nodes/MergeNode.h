/**
 * @file MergeNode.h
 * @brief Nœud de fusion de plusieurs sources
 *
 * Représente un nœud qui combine les données de plusieurs nœuds amont.
 * Types de fusion supportés:
 * - Union: toutes les lignes combinées
 * - Join: jointure sur clés commune
 * - Intersection: lignes communes
 * - Concatenate: concaténation horizontale
 *
 * Parent: RuntimeNode
 * Entrée: plusieurs (reçoit les données de plusieurs nœuds)
 *
 * Namespace: dn::nodes
 */

#ifndef MERGENODE_H
#define MERGENODE_H

#include "RuntimeNode.h"
#include "../core/DataTable.h"

namespace dn::nodes {

    class MergeNode : public RuntimeNode
    {
        Q_OBJECT

    public:
        explicit MergeNode(const QVector<RuntimeNode*>& parents,
                           MergeType type = MergeType::Union,
                           const QStringList& joinKeys = QStringList(),
                           const QString& name = "Merge",
                           QObject *qtParent = nullptr);

        /// Nombre d'entrées (dynamique)
        int inputCount() const override { return m_inputs.size(); }

        /// Combine les données des entrées
        void compute() override;

        /// Reçoit les résultats des nœuds amont
        void setCachedResult(const QVector<const dn::core::DataTable*>& inputs) override;

        /// Ajoute une entrée
        void addInput(RuntimeNode* node);

        /// Retire une entrée
        void removeInput(int index);

        /// Définit le type de fusion
        void setMergeType(MergeType type);

        /// Retourne le type de fusion
        MergeType getMergeType() const { return m_mergeType; }

        /// Définit les clés de jointure
        void setJoinKeys(const QStringList& keys);

        /// Retourne les clés de jointure
        QStringList getJoinKeys() const { return m_joinKeys; }

    private:
        void updateParameters();
        dn::core::DataTable unionMerge(const QVector<const dn::core::DataTable*>& inputs) const;
        dn::core::DataTable joinMerge(const QVector<const dn::core::DataTable*>& inputs) const;
        dn::core::DataTable intersectionMerge(const QVector<const dn::core::DataTable*>& inputs) const;
        dn::core::DataTable concatenateMerge(const QVector<const dn::core::DataTable*>& inputs) const;

        MergeType m_mergeType;
        QStringList m_joinKeys;
    };

}


#endif // MERGENODE_H
