/**
 * @file SelectColumnsTransformation.cpp
 * @brief Implémentation de la sélection de colonnes
 *
 * @see SelectColumnsTransformation.h
 */

#include "SelectColumnsTransformation.h"
#include "../core/DataTable.h"

using namespace dn::transformations;
using namespace dn::core;

SelectColumnsTransformation::SelectColumnsTransformation(const QStringList& columns,
                                                         QObject *parent)
    : Transformation(parent),
    m_columns(columns)
{
}

/// Sélectionne les colonnes demandées
DataTable SelectColumnsTransformation::transform(const DataTable& input)
{
    DataTable result;

    // Étape 1: trouve les index des colonnes
    QList<int> indexes;
    for (const QString& colName : m_columns) {
        int idx = input.columnIndex(colName);
        if (idx >= 0)
            indexes.append(idx);
    }

    if (indexes.isEmpty())
        return result;

    // Étape 2: définit les noms et types
    QStringList newNames;
    QList<ColumnType> newTypes;
    for (int idx : indexes) {
        newNames << input.columnName(idx);
        newTypes << input.getColumnType(idx);
    }

    result.setColumnNames(newNames);
    result.setColumnTypes(newTypes);

    // Étape 3: copie les lignes avec les colonnes sélectionnées
    for (const auto& row : input.rows()) {
        QList<QVariant> newRow;
        for (int idx : indexes) {
            if (idx < row.size())
                newRow.append(row.at(idx));
        }
        result.addRow(newRow);
    }

    return result;
}

/// Retourne la description
QString SelectColumnsTransformation::description() const
{
    return QString("Select columns: %1").arg(m_columns.join(", "));
}