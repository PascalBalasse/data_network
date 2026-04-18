/**
 * @file DataTableModel.h
 * @brief Modèle Qt pour l'affichage d'un DataTable
 *
 * Adaptateur entre DataTable et QAbstractTableModel.
 * Permet l'affichage dans un QTableView.
 *
 * Namespace: dn::ui
 */

#ifndef DATATABLEMODEL_H
#define DATATABLEMODEL_H

#include <QAbstractTableModel>
#include "../../network/core/DataTable.h"

namespace dn::ui {

    class DataTableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit DataTableModel(QObject *parent = nullptr);

        /// Définit le DataTable à afficher
        void setTable(const dn::core::DataTable* table);

        /// Retourne le nombre de lignes
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        /// Retourne le nombre de colonnes
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        /// Retourne les données pour une cellule
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        /// Retourne les en-têtes de lignes/colonnes
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;

    private:
        const dn::core::DataTable* m_table = nullptr;
    };

}

#endif // DATATABLEMODEL_H
