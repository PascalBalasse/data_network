/**
 * @file CalculatedColumnTransformation.h
 * @brief Transformation de colonne calculée
 *
 * Crée une nouvelle colonne basée sur une expression utilisant les colonnes existantes.
 *
 * Usage:
 * @code
 * auto calc = new CalculatedColumnTransformation("Total", "[Prix] * [Quantité]", ColumnType::Double);
 * DataTable result = calc->transform(input);
 * @endcode
 *
 * Namespace: dn::transformations
 */

#ifndef CALCULATEDCOLUMnTRANSFORMATION_H
#define CALCULATEDCOLUMnTRANSFORMATION_H

#include "Transformation.h"
#include <QString>
#include "../../network/core/DataTable.h"

namespace dn::transformations {

class CalculatedColumnTransformation : public Transformation
{
    Q_OBJECT

public:
    explicit CalculatedColumnTransformation(const QString& columnName,
                                            const QString& expression,
                                            dn::core::ColumnType columnType,
                                            QObject *parent = nullptr);

    /// Crée une colonne calculée basée sur l'expression
    DataTable transform(const dn::core::DataTable& input) override;

    /// Retourne la description
    QString description() const override;

    /// Getters pour les propriétés de la transformation
    QString getColumnName() const { return m_columnName; }
    QString getExpression() const { return m_expression; }
    dn::core::ColumnType getColumnType() const { return m_columnType; }

private:
    QString m_columnName;
    QString m_expression;
    dn::core::ColumnType m_columnType;
};

}

#endif // CALCULATEDCOLUMnTRANSFORMATION_H