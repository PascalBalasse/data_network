/**
 * @file RemoveDuplicatesTransformation.h
 * @brief Transformation pour supprimer les doublons
 *
 * Namespace: dn::transformations
 */

#ifndef REMOVEDUPLICATESTRANSFORMATION_H
#define REMOVEDUPLICATESTRANSFORMATION_H

#include "Transformation.h"
#include <QStringList>

namespace dn::transformations {

    class RemoveDuplicatesTransformation : public Transformation
    {
        Q_OBJECT

    public:
        explicit RemoveDuplicatesTransformation(const QStringList& columns, QObject* parent = nullptr);

        DataTable transform(const DataTable& input) override;
        QString description() const override;

        static QString type() { return "RemoveDuplicates"; }
        QString getType() const override { return type(); }
        QJsonObject toJson() const override;

        QStringList getColumns() const { return m_columns; }

    private:
        QStringList m_columns;
    };

}

#endif