/**
 * @file Transformation.h
 * @brief Interface de base pour les transformations
 *
 * Classe abstraite que toutes les transformations doivent implémenter.
 * Chaque transformation prend un DataTable en entrée et retourne
 * un DataTable transformé.
 *
 * Usage:
 * @code
 * class MyTransform : public Transformation {
 *     DataTable transform(const DataTable& input) override { ... }
 *     QString description() const override { return "Ma transformation"; }
 * };
 * @endcode
 *
 * Namespace: dn::transformations
 */

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "../core/DataTable.h"
#include <QObject>
#include <QString>

using namespace dn::core;

namespace dn::transformations {

    class Transformation : public QObject
    {
        Q_OBJECT

    public:
        explicit Transformation(QObject *parent = nullptr) : QObject(parent) {}
        virtual ~Transformation() {}

        /// Applique la transformation aux données d'entrée
        virtual DataTable transform(const DataTable& input) = 0;

        /// Retourne une description de la transformation
        virtual QString description() const = 0;
    };

}

#endif // TRANSFORMATION_H
