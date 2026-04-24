/**
 * @file customcsvdialog.h
 * @brief Dialogue de configuration CSV
 *
 * Permet de configurer la lecture/écriture de fichiers CSV.
 *
 * Paramètres:
 * - fileName: Chemin du fichier
 * - separator: Séparateur (, ou ; ou \t)
 * - hasHeader: Première ligne = en-têtes
 *
 * Namespace:dn::dialogs
 */

#ifndef CUSTOMCSVDIALOG_H
#define CUSTOMCSVDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

using namespace dn::core;

namespace dn::dialogs{

    class CustomCSVDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomCSVDialog(ConnectorMode mode = ConnectorMode::Read,
                                 const QString& defaultNodeName = QString(),
                                 QWidget *parent = nullptr);

        /// Retourne les paramètres configurés
        QMap<QString, QVariant> getParameters() const;

        /// Affiche le dialogue en mode lecture
        static QMap<QString, QVariant> getCSVReadParameters(QWidget *parent = nullptr,
                                                            const QString& defaultNodeName = "CSV Source");

        /// Affiche le dialogue en mode écriture
        static QMap<QString, QVariant> getCSVWriteParameters(QWidget *parent = nullptr,
                                                             const QString& defaultNodeName = "CSV Target");

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
        QLineEdit *m_fileEdit;
        QLineEdit *m_nameEdit;
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


#endif // CUSTOMCSVDIALOG_H
