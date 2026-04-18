/**
 * @file FilterTransformation.cpp
 * @brief Implémentation du filtrage de lignes
 *
 * @see FilterTransformation.h
 */

#include "FilterTransformation.h"
#include "../core/DataTable.h"

using namespace dn::transformations;
using namespace dn::core;

FilterTransformation::FilterTransformation(const QString &columnName,
                                           Operator op,
                                           const QString &value,
                                           QObject *parent)
    : Transformation(parent),
    m_columnName(columnName),
    m_operator(op),
    m_value(value)
{
}

/// Filtre les lignes selon la condition
DataTable FilterTransformation::transform(const DataTable& input)
{
    DataTable result;

    // Copie la structure (noms et types)
    result.setColumnNames(input.columnNames());
    result.setColumnTypes(input.columnTypes());

    int colIndex = input.columnIndex(m_columnName);
    if (colIndex < 0)
        return result;

    // Parcourt les lignes et garde celles qui vérifient la condition
    for (const auto& row : input.rows()) {
        if (colIndex >= row.size())
            continue;

        QVariant cell = row.at(colIndex);
        if (evaluate(cell)) {
            result.addRow(row);
        }
    }

    return result;
}

/// Évalue une cellule selon l'opérateur
bool FilterTransformation::evaluate(const QVariant &cellValue) const
{
    QString cellStr = cellValue.toString();

    switch (m_operator) {
    case Equals:
        return cellStr == m_value;
    case NotEquals:
        return cellStr != m_value;
    case GreaterThan:
        return cellStr > m_value;
    case LessThan:
        return cellStr < m_value;
    case Contains:
        return cellStr.contains(m_value);
    default:
        return false;
    }
}

/// Retourne la description
QString FilterTransformation::description() const
{
    QString opStr;
    switch (m_operator) {
    case Equals: opStr = "=="; break;
    case NotEquals: opStr = "!="; break;
    case GreaterThan: opStr = ">"; break;
    case LessThan: opStr = "<"; break;
    case Contains: opStr = "contains"; break;
    }

    return QString("Filter: %1 %2 %3").arg(m_columnName, opStr, m_value);
}