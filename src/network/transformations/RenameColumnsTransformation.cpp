/**
 * @file RenameColumnsTransformation.cpp
 * @brief Implémentation du renommage de colonnes
 *
 * @see RenameColumnsTransformation.h
 */

#include "RenameColumnsTransformation.h"

using namespace dn::transformations;
using namespace dn::core;

/// Constructeur
RenameColumnsTransformation::RenameColumnsTransformation(
    const QHash<QString, QString>& renames,
    QObject *parent)
    : Transformation(parent),
    m_renames(renames)
{
}

/// Renomme les colonnes
DataTable RenameColumnsTransformation::transform(const DataTable& input)
{
    DataTable result;

    // Copie les noms et types
    QStringList newNames = input.columnNames();
    QList<ColumnType> newTypes = input.columnTypes();

    for (int i = 0; i < newNames.size(); ++i) {
        const QString& oldName = newNames[i];
        if (m_renames.contains(oldName)) {
            newNames[i] = m_renames.value(oldName);
        }
    }

    result.setColumnNames(newNames);
    result.setColumnTypes(newTypes);

    // Copie les lignes
    for (const auto& row : input.rows()) {
        result.addRow(row);
    }

    return result;
}

/// Retourne la description
QString RenameColumnsTransformation::description() const
{
    QStringList parts;

    for (auto it = m_renames.begin(); it != m_renames.end(); ++it) {
        parts << QString("%1 → %2").arg(it.key(), it.value());
    }

    return QString("Rename columns: %1").arg(parts.join(", "));
}