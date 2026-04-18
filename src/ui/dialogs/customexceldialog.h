/**
 * @file customexceldialog.h
 * @brief Dialogue de configuration Excel
 *
 * Permet de configurer la lecture/écriture de fichiers Excel.
 * Paramètres:
 * - fileName: Chemin du fichier
 * - sheetName: Nom de la feuille
 * - hasHeader: Première ligne = en-têtes
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMEXCELDIALOG_H
#define CUSTOMEXCELDIALOG_H

#include "../../network/core/enums.h"
#include "../../ui/models/DataTableModel.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QTableView>

using namespace dn::core;

namespace dn::dialogs {

    class CustomExcelDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomExcelDialog(ConnectorMode mode = ConnectorMode::Read, QWidget *parent = nullptr);

        /// Retourne les paramètres
        QMap<QString, QVariant> getParameters() const;

        /// Affiche le dialogue en mode lecture
        static QMap<QString, QVariant> getExcelReadParameters(QWidget *parent = nullptr);

        /// Affiche le dialogue en mode écriture
        static QMap<QString, QVariant> getExcelWriteParameters(QWidget *parent = nullptr);

        /// Retourne le mode
        ConnectorMode getMode() const { return m_mode; }

    private slots:
        void onBrowseClicked();
        void onAccept();
        void onReject();
        void onFileSelected(const QString& fileName);
        void onSheetSelected(int index);

    private:
        void setupUI();
        void validateAndAccept();
        void updateUIForMode();
        void loadPreview();

        //══════════════════════════════════════════════════════════════════
        // Widgets
        //══════════════════════════════════════════════════════════════════
        QLineEdit *m_fileEdit;
        QComboBox *m_sheetCombo;
        QCheckBox *m_headerCheck;
        QPushButton *m_okButton;
        QPushButton *m_browseButton;
        QTableView *m_previewTable;
        dn::ui::DataTableModel *m_previewModel;
        std::unique_ptr<dn::core::DataTable> m_previewTableData;

        //══════════════════════════════════════════════════════════════════
        // Données
        //══════════════════════════════════════════════════════════════════
        QMap<QString, QVariant> m_params;
        ConnectorMode m_mode;
        bool m_accepted;
        bool m_previewLoaded = false;
    };

}

#endif
