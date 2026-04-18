/**
 * @file RenameColumnsTransformation.h
 * @brief Transformation de renommage de colonnes
 *
 * Renomme les colonnes d'un DataTable selon un dictionnaire.
 *
 * Usage:
 * @code
 * QHash<QString, QString> renames = {{"nom", "Nom"}, {"prenom", "Prénom"}};
 * auto rename = new RenameColumnsTransformation(renames);
 * DataTable result = rename->transform(input);
 * @endcode
 *
 * Namespace:dn::transformations
 */

#ifndef RENAMECOLUMNSTRANSFORMATION_H
#define RENAMECOLUMNSTRANSFORMATION_H

#include "Transformation.h"

using namespace dn::core;

namespace dn::transformations {

class RenameColumnsTransformation : public Transformation
{
    Q_OBJECT

public:
    RenameColumnsTransformation(
        const QHash<QString, QString>& renames,
        QObject *parent = nullptr);

    /// Renomme les colonnes
    DataTable transform(const DataTable& input) override;

    /// Retourne la description
    QString description() const override;

private:
    QHash<QString, QString> m_renames;
};

}
#endif // RENAMECOLUMNSTRANSFORMATION_H
