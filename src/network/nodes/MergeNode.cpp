#include "MergeNode.h"

using namespace dn::nodes;
using namespace dn::core;

MergeNode::MergeNode(const QVector<RuntimeNode*>& parents,
                     MergeType type,
                     const QStringList& joinKeys,
                     const QString& name,
                     QObject *qtParent)
    : RuntimeNode(parents, qtParent)  // ← parents injectés
    , m_mergeType(type)
    , m_joinKeys(joinKeys)
{
    setNodeType(dn::core::NodeType::Merge);
    setName(name);
    updateParameters();
}

void MergeNode::compute(){

}

void MergeNode::addInput(RuntimeNode* node)
{
    int newIndex = m_inputs.size();
    m_inputs.append(nullptr);
    setInput(newIndex, node);
    updateParameters();
}

void MergeNode::removeInput(int index)
{
    if (index < 0 || index >= m_inputs.size())
        return;

    if (m_inputs.size() <= 2)
        return;

    setInput(index, nullptr);
    m_inputs.remove(index);
    updateParameters();
}

void MergeNode::setMergeType(MergeType type)
{
    m_mergeType = type;
    updateParameters();
    compute();
}

void MergeNode::setJoinKeys(const QStringList& keys)
{
    m_joinKeys = keys;
    updateParameters();
    compute();
}

void MergeNode::updateParameters()
{
    setParameter("merge_type", QString::number(static_cast<int>(m_mergeType)));
    setParameter("input_count", QString::number(m_inputs.size()));

    if (!m_joinKeys.isEmpty()) {
        setParameter("join_keys", m_joinKeys.join(","));
    }

    QString typeStr;
    switch (m_mergeType) {
    case MergeType::Union: typeStr = "Union"; break;
    case MergeType::Join: typeStr = "Join"; break;
    case MergeType::Intersection: typeStr = "Intersection"; break;
    case MergeType::Concatenate: typeStr = "Concatenate"; break;
    }
    setParameter("merge_type_name", typeStr);
}

void MergeNode::setCachedResult(const QVector<const dn::core::DataTable*>& inputs)
{
    // Filtrer les entrées valides
    QVector<const dn::core::DataTable*> validInputs;
    for (const auto* input : inputs) {
        if (input && input->rowCount() > 0) {
            validInputs.append(input);
        }
    }

    if (validInputs.size() < 2) {
        m_cachedResult =dn::core::DataTable();
    }

    switch (m_mergeType) {
    case MergeType::Union:
        m_cachedResult = unionMerge(validInputs);
    case MergeType::Join:
        m_cachedResult = joinMerge(validInputs);
    case MergeType::Intersection:
        m_cachedResult = intersectionMerge(validInputs);
    case MergeType::Concatenate:
        m_cachedResult = concatenateMerge(validInputs);
    default:
        m_cachedResult = dn::core::DataTable();
    }
}
DataTable MergeNode::unionMerge(const QVector<const dn::core::DataTable*>& inputs) const
{
    dn::core::DataTable result;

    if (inputs.isEmpty())
        return result;

    const auto& firstTable = *inputs[0];
    result.setColumnNames(firstTable.columnNames());
    result.setColumnTypes(firstTable.columnTypes());

    for (const auto* table : inputs) {
        for (const auto& row : table->rows()) {
            result.addRow(row);
        }
    }

    return result;
}

DataTable MergeNode::joinMerge(const QVector<const dn::core::DataTable*>& inputs) const
{
    if (inputs.size() < 2 || m_joinKeys.isEmpty()) {
        return dn::core::DataTable();
    }

    dn::core::DataTable result;

    // Pour l'instant, une implémentation simple qui fait une jointure interne
    // entre les deux premières tables seulement
    const auto& leftTable = *inputs[0];
    const auto& rightTable = *inputs[1];

    // Vérifier que les clés de jointure existent dans les deux tables
    QList<int> leftKeyIndices;
    QList<int> rightKeyIndices;

    for (const QString& key : m_joinKeys) {
        int leftIdx = leftTable.columnIndex(key);
        int rightIdx = rightTable.columnIndex(key);

        if (leftIdx < 0 || rightIdx < 0) {
            qWarning() << "Join key" << key << "not found in one of the tables";
            return dn::core::DataTable();
        }

        leftKeyIndices.append(leftIdx);
        rightKeyIndices.append(rightIdx);
    }

    // Construire les noms de colonnes pour le résultat
    QStringList resultColumns;
    resultColumns.append(leftTable.columnNames());

    // Ajouter les colonnes de droite qui ne sont pas des clés de jointure
    for (int i = 0; i < rightTable.columnCount(); ++i) {
        QString colName = rightTable.columnName(i);
        if (!m_joinKeys.contains(colName)) {
            resultColumns.append(colName);
        }
    }
    result.setColumnNames(resultColumns);

    // Pour chaque ligne de gauche, trouver les lignes correspondantes à droite
    for (int leftRow = 0; leftRow < leftTable.rowCount(); ++leftRow) {
        QList<QVariant> leftRowData = leftTable.rows()[leftRow];

        for (int rightRow = 0; rightRow < rightTable.rowCount(); ++rightRow) {
            QList<QVariant> rightRowData = rightTable.rows()[rightRow];

            // Vérifier si les clés correspondent
            bool match = true;
            for (int k = 0; k < m_joinKeys.size(); ++k) {
                if (leftRowData[leftKeyIndices[k]] != rightRowData[rightKeyIndices[k]]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                // Fusionner les deux lignes
                QList<QVariant> mergedRow = leftRowData;

                // Ajouter les colonnes de droite qui ne sont pas des clés
                for (int i = 0; i < rightTable.columnCount(); ++i) {
                    if (!m_joinKeys.contains(rightTable.columnName(i))) {
                        mergedRow.append(rightRowData[i]);
                    }
                }

                result.addRow(mergedRow);
            }
        }
    }

    return result;
}

dn::core::DataTable MergeNode::intersectionMerge(const QVector<const dn::core::DataTable*>& inputs) const
{
    if (inputs.isEmpty()) {
        return dn::core::DataTable();
    }

    dn::core::DataTable result;
    const auto& firstTable = *inputs[0];

    result.setColumnNames(firstTable.columnNames());
    result.setColumnTypes(firstTable.columnTypes());

    // Pour chaque ligne de la première table
    for (int i = 0; i < firstTable.rowCount(); ++i) {
        const auto& row = firstTable.rows()[i];
        bool inAll = true;

        // Vérifier si elle existe dans TOUTES les autres tables
        for (int t = 1; t < inputs.size(); ++t) {
            if (!inputs[t]) {
                inAll = false;
                break;
            }

            bool found = false;
            for (const auto& otherRow : inputs[t]->rows()) {
                if (otherRow == row) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                inAll = false;
                break;
            }
        }

        if (inAll) {
            result.addRow(row);
        }
    }

    return result;
}

dn::core::DataTable MergeNode::concatenateMerge(const QVector<const dn::core::DataTable*>& inputs) const
{
    if (inputs.isEmpty()) {
        return dn::core::DataTable();
    }

    dn::core::DataTable result;

    // Collecter tous les noms de colonnes
    QStringList allColumns;
    for (const auto* table : inputs) {
        if (table) {
            allColumns.append(table->columnNames());
        }
    }
    result.setColumnNames(allColumns);

    // Déterminer le nombre maximum de lignes
    int maxRows = 0;
    for (const auto* table : inputs) {
        if (table && table->rowCount() > maxRows) {
            maxRows = table->rowCount();
        }
    }

    // Construire les lignes
    for (int row = 0; row < maxRows; ++row) {
        QList<QVariant> newRow;

        for (const auto* table : inputs) {
            if (!table) {
                // Table manquante, ajouter des colonnes vides
                for (int i = 0; i < (inputs[0] ? inputs[0]->columnCount() : 1); ++i) {
                    newRow.append(QVariant());
                }
            } else if (row < table->rowCount()) {
                // Ajouter toutes les colonnes de cette ligne
                newRow.append(table->rows()[row]);
            } else {
                // Ligne manquante, ajouter des valeurs vides
                for (int i = 0; i < table->columnCount(); ++i) {
                    newRow.append(QVariant());
                }
            }
        }

        result.addRow(newRow);
    }

    return result;
}