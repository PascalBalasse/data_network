/**
 * @file customtxtdialog.h
 * @brief Dialogue de configuration TXT (texte délimité)
 *
 * Permet de configurer la lecture/écriture de fichiers TXT.
 * Par défaut utilise la tabulation comme séparateur.
 *
 * Paramètres:
 * - fileName: Chemin du fichier
 * - separator: Séparateur (par défaut: tabulation)
 * - hasHeader: Première ligne = en-têtes
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMTXTDIALOG_H
#define CUSTOMTXTDIALOG_H

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

    class CustomTXTDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomTXTDialog(ConnectorMode mode = ConnectorMode::Read,
                                 const QString& defaultNodeName = QString(),
                                 QWidget *parent = nullptr);

        /// Retourne les paramètres configurés
        QMap<QString, QVariant> getParameters() const;

        /// Affiche le dialogue en mode lecture
        static QMap<QString, QVariant> getTXTReadParameters(QWidget *parent = nullptr,
                                                            const QString& defaultNodeName = "TXT Source");

        /// Affiche le dialogue en mode écriture
        static QMap<QString, QVariant> getTXTWriteParameters(QWidget *parent = nullptr,
                                                             const QString& defaultNodeName = "TXT Target");

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
        QLabel *m_nameLabel;
        QLabel *m_fileLabel;
        QLineEdit *m_nameEdit;
        QLineEdit *m_fileEdit;
        QCheckBox *m_headerCheck;
        QComboBox *m_sepCombo;
        QPushButton *m_okButton;
        QPushButton *m_browseButton;

        //══════════════════════════════════════════════════════════════════
        // Données
        //══════════════════════════════════════════════════════════════════
        QMap<QString, QVariant> m_params;
        ConnectorMode m_mode;
        QString m_defaultNodeName;
        bool m_accepted;
    };

}

#endif // CUSTOMTXTDIALOG_H