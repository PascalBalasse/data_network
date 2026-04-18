/**
 * @file custompdfdialog.h
 * @brief Dialogue de configuration PDF
 *
 * Permet de configurer la lecture de fichiers PDF.
 * Paramètres:
 * - fileName: Chemin du fichier
 * - startPage: Page de début
 * - endPage: Page de fin (-1 = toutes)
 * - hasHeader: Première ligne = en-têtes
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMPDFDIALOG_H
#define CUSTOMPDFDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

using namespace dn::core;

namespace dn::dialogs {

    class CustomPDFDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomPDFDialog(ConnectorMode mode = ConnectorMode::Read, QWidget *parent = nullptr);

        /// Retourne les paramètres
        QMap<QString, QVariant> getParameters() const;

        /// Affiche le dialogue en mode lecture
        static QMap<QString, QVariant> getPDFReadParameters(QWidget *parent = nullptr);

        /// Retourne le mode
        ConnectorMode getMode() const { return m_mode; }

    private slots:
        void onBrowseClicked();
        void onAccept();
        void onReject();

    private:
        void setupUI();
        void validateAndAccept();
        void updateUIForMode();

        //══════════════════════════════════════════════════════════════════
        // Widgets
        //══════════════════════════════════════════════════════════════════
        QLineEdit *m_fileEdit;
        QComboBox *m_startPageCombo;
        QComboBox *m_endPageCombo;
        QCheckBox *m_headerCheck;
        QPushButton *m_okButton;
        QPushButton *m_browseButton;

        //══════════════════════════════════════════════════════════════════
        // Données
        //══════════════════════════════════════════════════════════════════
        QMap<QString, QVariant> m_params;
        ConnectorMode m_mode;
        bool m_accepted;
    };

}

#endif