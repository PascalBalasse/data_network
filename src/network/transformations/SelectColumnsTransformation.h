/**
 * @file SelectColumnsTransformation.h
 * @brief Transformation de sélection de colonnes
 *
 * Sélectionne un sous-ensemble de colonnes d'un DataTable.
 *
 * Usage:
 * @code
 * auto select = new SelectColumnsTransformation({"Nom", "Prénom", "Age"});
 * DataTable result = select->transform(input);
 * @endcode
 *
 * Namespace: dn::transformations
 */

#ifndef SELECTCOLUMNSTRANSFORMATION_H
#define SELECTCOLUMNSTRANSFORMATION_H

#include "Transformation.h"
#include <QStringList>

namespace dn::transformations {

class SelectColumnsTransformation : public Transformation
    {
        Q_OBJECT

    public:
        explicit SelectColumnsTransformation(const QStringList& columns,
                                           QObject *parent = nullptr);

        /// Sélectionne les Colonnes spécifiées
        DataTable transform(const dn::core::DataTable& input) override;

        /// Retourne la description
        QString description() const override;

    private:
        QStringList m_columns;
    };

}

#endif // SELECTCOLUMNSTRANSFORMATION_H
