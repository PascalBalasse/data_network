/**
 * @file customjsondialog.h
 * @brief Dialogue de configuration JSON
 *
 * Permet de configurer la lecture/écriture de fichiers JSON.
 *
 * Paramètres:
 * - fileName: Chemin du fichier
 * - prettyPrint: Formatage lisible
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMJSONDIALOG_H
#define CUSTOMJSONDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>

using namespace dn::core;

namespace dn::dialogs {

    class CustomJSONDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomJSONDialog(ConnectorMode mode = ConnectorMode::Read,
                                  const QString& defaultNodeName = QString(),
                                  QWidget *parent = nullptr);

        /// Retourne les paramètres configurés
        QMap<QString, QVariant> getParameters() const;

        /// Affiche le dialogue en mode lecture
        static QMap<QString, QVariant> getJSONReadParameters(QWidget *parent = nullptr,
                                                             const QString& defaultNodeName = "JSON Source");

        /// Affiche le dialogue en mode écriture
        static QMap<QString, QVariant> getJSONWriteParameters(QWidget *parent = nullptr,
                                                              const QString& defaultNodeName = "JSON Target");

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
        QCheckBox *m_prettyPrintCheck;
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

#endif // CUSTOMJSONDIALOG_H