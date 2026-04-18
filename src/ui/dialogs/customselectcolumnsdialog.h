/**
 * @file customselectcolumnsdialog.h
 * @brief Dialogue de sélection de colonnes
 *
 * Permet de sélectionner les colonnes à garder.
 * Interface avec:
 * - Liste de cases à cocher
 * - Barre de recherche
 * - Aperçu en temps réel
 * - Boutons Tout sélectionner/Tout désélectionner
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMSELECTCOLUMNSDIALOG_H
#define CUSTOMSELECTCOLUMNSDIALOG_H

#include "../../network/core/DataTable.h"
#include "../../ui/models/DataTableModel.h"
#include <QDialog>
#include <QStringList>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTableView>


namespace dn::dialogs{

    class CustomSelectColumnsDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomSelectColumnsDialog(const dn::core::DataTable* table,
                                           QWidget *parent = nullptr);

        /// Retourne la liste des colonnes sélectionnées
        QStringList getSelectedColumns() const { return m_selectedColumns; }

        /// Retourne true si au moins une colonne est sélectionnée
        bool isValid() const { return !m_selectedColumns.isEmpty(); }

        /// Méthode statique simplifiée
        static bool getSelectedColumns(QWidget *parent,
                                       const dn::core::DataTable* table,
                                       QStringList& columns);

    private slots:
        void onAccept();
        void onReject();
        void onSelectAll();
        void onDeselectAll();
        void onSearchTextChanged(const QString& text);
        void onSelectionChanged();

    private:
        void setupUI();
        void updateColumnList();
        void updateStats();
        void updatePreview();

        //══════════════════════════════════════════════════════════════════
        // Widgets
        //══════════════════════════════════════════════════════════════════
        QLineEdit *m_searchEdit;
        QListWidget *m_columnList;
        QTableView *m_previewTable;
        dn::ui::DataTableModel *m_previewModel;
        QPushButton *m_okButton;
        QPushButton *m_selectAllButton;
        QPushButton *m_deselectAllButton;

        //══════════════════════════════════════════════════════════════════
        // Données
        //══════════════════════════════════════════════════════════════════
        const dn::core::DataTable* m_table;
        QStringList m_allColumns;
        QStringList m_filteredColumns;
        QSet<QString> m_selectedSet;
        QStringList m_selectedColumns;
        std::unique_ptr<dn::core::DataTable> m_previewData;
    };

}


#endif // CUSTOMSELECTCOLUMNSDIALOG_H
