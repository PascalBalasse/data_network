/**
 * @file TransformationNode.cpp
 * @brief Implémentation du nœud de transformation
 *
 * Applique une transformation aux données entrantes.
 *
 * @see TransformationNode.h
 */

#include "TransformationNode.h"
#include "../transformations/FilterTransformation.h"
#include "../transformations/SelectColumnsTransformation.h"
#include "../transformations/RenameColumnsTransformation.h"
#include "../transformations/CalculatedColumnTransformation.h"

using namespace dn::transformations;
using namespace dn::nodes;

TransformationNode::TransformationNode(dn::transformations::Transformation* transform,
                                       RuntimeNode* parent,
                                       const QString& name,
                                       QObject *qtParent)
    : RuntimeNode(parent, qtParent)
    , m_transformation(transform)
{
    setNodeType(dn::core::NodeType::Transform);
    setName(name);
    syncParametersFromTransformation();
}

/// Calcule et met à jour le niveau
void TransformationNode::compute(){
    QVector<const dn::core::DataTable*> inputTables=getInputTables();
    setCachedResult(inputTables);

    RuntimeNode* inputNode=m_inputs[0];
    int level=inputNode->getNodeLevel();
    setNodeLevel(level+1);

    emit tableChanged(m_cachedResult);
}

/// Applique la transformation
void TransformationNode::setCachedResult(const QVector<const dn::core::DataTable*>& inputs)
{
    if (!m_transformation || inputs.size() < 1 || !inputs[0]) {
        m_cachedResult = dn::core::DataTable();
    }

    m_cachedResult = m_transformation->transform(*inputs[0]);
}

/// Change la transformation et recalcule
void TransformationNode::setTransformation(dn::transformations::Transformation* transform)
{
    m_transformation = transform;
    syncParametersFromTransformation();
    compute();
}

/// Synchronise les paramètres depuis la transformation
void TransformationNode::syncParametersFromTransformation()
{
    if (!m_transformation)
        return;

    // Stocke la description comme paramètre
    setParameter("description", m_transformation->description());

    // Extrait les paramètres spécifiques par type
    if (auto* filter = dynamic_cast<dn::transformations::FilterTransformation*>(m_transformation)) {
    }
    else if (auto* select = dynamic_cast<dn::transformations::SelectColumnsTransformation*>(m_transformation)) {
    }
    else if (auto* rename = dynamic_cast<dn::transformations::RenameColumnsTransformation*>(m_transformation)) {
    }
    else if (auto* calc = dynamic_cast<dn::transformations::CalculatedColumnTransformation*>(m_transformation)) {
        // Pour les colonnes calculées, on stocke l'expression et le nom de colonne
        setParameter("columnName", calc->getColumnName());
        setParameter("expression", calc->getExpression());
        setParameter("columnType", QString::number(static_cast<int>(calc->getColumnType())));
    }
}
