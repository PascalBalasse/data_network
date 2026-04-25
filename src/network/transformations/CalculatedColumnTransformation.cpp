/**
 * @file CalculatedColumnTransformation.cpp
 * @brief Implémentation de la transformation de colonne calculée
 */

#include "CalculatedColumnTransformation.h"
#include <QRegularExpression>
#include <QDebug>

namespace dn::transformations {

    CalculatedColumnTransformation::CalculatedColumnTransformation(const QString& columnName,
                                                                   const QString& expression,
                                                                   dn::core::ColumnType columnType,
                                                                   QObject *parent)
        : Transformation(parent), m_columnName(columnName), m_expression(expression), m_columnType(columnType)
    {
    }

    DataTable CalculatedColumnTransformation::transform(const dn::core::DataTable& input)
    {
        // Créer une copie du tableau d'entrée pour y ajouter la colonne calculée
        DataTable result = input;
        
        // Ajouter la colonne calculée
        bool success = result.addCalculatedColumn(m_columnName, m_expression, m_columnType);
        
        if (!success) {
            qWarning() << "Failed to add calculated column:" << m_columnName 
                      << "with expression:" << m_expression;
            // Retourner le tableau sans modification en cas d'erreur
            return input;
        }
        
        return result;
    }

    QString CalculatedColumnTransformation::description() const
    {
        return QString("Colonne calculée: %1 = %2").arg(m_columnName, m_expression);
    }

} // namespace dn::transformations