/**
 * @file SortTransformation.h
 * @brief Transformation pour trier les données
 *
 * Namespace: dn::transformations
 */

#ifndef SORTTRANSFORMATION_H
#define SORTTRANSFORMATION_H

#include "Transformation.h"
#include <QStringList>
#include <Qt>

namespace dn::transformations {

    class SortTransformation : public Transformation
    {
        Q_OBJECT

    public:
        explicit SortTransformation(const QStringList& columns, bool ascending = true, QObject* parent = nullptr);

        DataTable transform(const DataTable& input) override;
        QString description() const override;

        static QString type() { return "Sort"; }
        QString getType() const override { return type(); }
        QJsonObject toJson() const override;

        QStringList getColumns() const { return m_columns; }
        bool getAscending() const { return m_ascending; }

    private:
        QStringList m_columns;
        bool m_ascending;
    };

}

#endif