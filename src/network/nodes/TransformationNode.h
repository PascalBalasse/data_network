/**
 * @file TransformationNode.h
 * @brief Nœud de transformation des données
 *
 * Représente un nœud qui applique une transformation aux données.
 * Les transformations incluent: filtrage, sélection de colonnes, renommage, etc.
 *
 * Parent: RuntimeNode
 * Entrée: 1 (reçoit les données d'un autre nœud)
 * Sortie: 1 (renvoie les données transformées)
 *
 * Namespace: dn::nodes
 */

#ifndef TRANSFORMATIONNODE_H
#define TRANSFORMATIONNODE_H

#include "RuntimeNode.h"
#include "../transformations/Transformation.h"

namespace dn::nodes {

    class TransformationNode : public RuntimeNode
    {
        Q_OBJECT

    public:
        explicit TransformationNode(dn::transformations::Transformation* transform,
                                    RuntimeNode* parent,
                                    const QString& name = "Transformation",
                                    QObject *qtParent = nullptr);

        /// Une entrée (reçoit les données d'un nœud amont)
        int inputCount() const override { return 1; }

        /// Applique la transformation
        void compute() override;

        /// Reçoit les résultat du nœud amont
        void setCachedResult(const QVector<const dn::core::DataTable*>& inputs) override;

        /// Définit une nouvelle transformation
        void setTransformation(dn::transformations::Transformation* transform);

        /// Retourne la transformation configurée
        dn::transformations::Transformation* getTransformation() const { return m_transformation; }

        /// Synchronise les paramètres depuis l'objet Transformation
        void syncParametersFromTransformation();

    private:
        dn::transformations::Transformation* m_transformation;
    };

}

#endif // TRANSFORMATIONNODE_H
