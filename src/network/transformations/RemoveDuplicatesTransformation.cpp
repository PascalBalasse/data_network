/**
 * @file RemoveDuplicatesTransformation.cpp
 * @brief Implémentation de la suppression de doublons
 *
 * Namespace: dn::transformations
 */

#include "RemoveDuplicatesTransformation.h"
#include "../core/DataTable.h"
#include <QJsonArray>

using namespace dn::core;
using namespace dn::transformations;

RemoveDuplicatesTransformation::RemoveDuplicatesTransformation(const QStringList& columns, QObject* parent)
    : Transformation(parent)
    , m_columns(columns)
{
}

QString RemoveDuplicatesTransformation::description() const
{
    return QString("Supprime les doublons sur %1").arg(m_columns.join(", "));
}

DataTable RemoveDuplicatesTransformation::transform(const DataTable& input)
{
    if (input.rowCount() == 0 || m_columns.isEmpty()) {
        return input;
    }

    DataTable result;

    result.setColumnNames(input.columnNames());
    result.setColumnTypes(input.columnTypes());

    QSet<QString> seenKeys;

    for (int row = 0; row < input.rowCount(); ++row) {
        QString key;
        for (const QString& colName : m_columns) {
            int colIdx = input.columnIndex(colName);
            if (colIdx >= 0) {
                key += input.value(row, colIdx).toString() + "|";
            }
        }

        if (!seenKeys.contains(key)) {
            seenKeys.insert(key);

            QVector<QVariant> rowData;
            for (int col = 0; col < input.columnCount(); ++col) {
                rowData.append(input.value(row, col));
            }
            result.addRow(rowData);
        }
    }

    return result;
}

QJsonObject RemoveDuplicatesTransformation::toJson() const
{
    QJsonObject json;
    QJsonArray cols;
    for (const QString& col : m_columns) {
        cols.append(col);
    }
    json["columns"] = cols;
    return json;
}