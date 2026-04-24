/**
 * @file SortTransformation.cpp
 * @brief Implémentation du tri de données
 *
 * Namespace: dn::transformations
 */

#include "SortTransformation.h"
#include "../core/DataTable.h"
#include <algorithm>
#include <QJsonArray>

using namespace dn::core;
using namespace dn::transformations;

SortTransformation::SortTransformation(const QStringList& columns, bool ascending, QObject* parent)
    : Transformation(parent)
    , m_columns(columns)
    , m_ascending(ascending)
{
}

QString SortTransformation::description() const
{
    QString direction = m_ascending ? "↑" : "↓";
    return QString("Tri par %1 %2").arg(m_columns.join(", "), direction);
}

DataTable SortTransformation::transform(const DataTable& input)
{
    if (input.rowCount() == 0 || m_columns.isEmpty()) {
        return input;
    }

    DataTable result = input;

    QString firstCol = m_columns.first();
    int colIdx = input.columnIndex(firstCol);
    if (colIdx < 0) {
        return input;
    }

    ColumnType colType = input.getColumnType(colIdx);

    QVector<QPair<int, QVariant>> rowsWithKeys;
    for (int row = 0; row < input.rowCount(); ++row) {
        rowsWithKeys.append({row, input.value(row, colIdx)});
    }

    if (m_ascending) {
        std::stable_sort(rowsWithKeys.begin(), rowsWithKeys.end(),
            [colType](const QPair<int, QVariant>& a, const QPair<int, QVariant>& b) {
                if (colType == ColumnType::Integer || colType == ColumnType::Double) {
                    return a.second.toDouble() < b.second.toDouble();
                }
                return a.second.toString() < b.second.toString();
            });
    } else {
        std::stable_sort(rowsWithKeys.begin(), rowsWithKeys.end(),
            [colType](const QPair<int, QVariant>& a, const QPair<int, QVariant>& b) {
                if (colType == ColumnType::Integer || colType == ColumnType::Double) {
                    return a.second.toDouble() > b.second.toDouble();
                }
                return a.second.toString() > b.second.toString();
            });
    }

    DataTable sortedData;
    sortedData.setColumnNames(result.columnNames());
    sortedData.setColumnTypes(result.columnTypes());

    for (const auto& rowPair : rowsWithKeys) {
        QVector<QVariant> row;
        for (int col = 0; col < result.columnCount(); ++col) {
            row.append(result.value(rowPair.first, col));
        }
        sortedData.addRow(row);
    }

    return sortedData;
}

QJsonObject SortTransformation::toJson() const
{
    QJsonObject json;
    QJsonArray cols;
    for (const QString& col : m_columns) {
        cols.append(col);
    }
    json["columns"] = cols;
    json["ascending"] = m_ascending;
    return json;
}