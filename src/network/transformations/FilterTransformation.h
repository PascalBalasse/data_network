/**
 * @file FilterTransformation.h
 * @brief Transformation de filtrage des lignes
 *
 * Filtre les lignes d'un DataTable selon une condition.
 * Opérateurs supportés:
 * - Equals: égal à la valeur
 * - NotEquals: différent de la valeur
 * - GreaterThan: supérieur à la valeur
 * - LessThan: inférieur à la valeur
 * - Contains: contient la chaîne
 *
 * Usage:
 * @code
 * auto filter = new FilterTransformation("Age", FilterTransformation::GreaterThan, "18");
 * DataTable result = filter->transform(input);
 * @endcode
 *
 * Namespace: dn::transformations
 */

#ifndef FILTERTRANSFORMATION_H
#define FILTERTRANSFORMATION_H

#include "Transformation.h"
#include <QString>

using namespace dn::core;

namespace dn::transformations {

    class FilterTransformation : public Transformation
    {
        Q_OBJECT

    public:
        /// Opérateurs de comparaison
        enum Operator {
            Equals,        ///< Égal à
            NotEquals,     ///< Différent de
            GreaterThan,   ///< Supérieur à
            LessThan,       ///< Inférieur à
            Contains       ///< Contient la chaîne
        };

        FilterTransformation(const QString &columnName, Operator op, const QString &value, QObject *parent = nullptr);

        /// Filtre les lignes selon la condition
        DataTable transform(const dn::core::DataTable& input) override;

        /// Retourne la description
        QString description() const override;

    private:
        /// Évalue une cellule
        bool evaluate(const QVariant &cellValue) const;

        QString m_columnName;
        Operator m_operator;
        QString m_value;
    };

}

#endif // FILTERTRANSFORMATION_H
