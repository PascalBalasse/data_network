/**
 * @file customxmldialog.h
 * @brief Dialogue de configuration XML
 *
 * Permet de configurer la lecture/écriture de fichiers XML.
 *
 * Paramètres:
 * - fileName: Chemin du fichier
 * - rootElement: Nom de l'élément racine
 * - rowElement: Nom de l'élément ligne
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMXMLDIALOG_H
#define CUSTOMXMLDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QLabel>

using namespace dn::core;

namespace dn::dialogs {

    class CustomXMLDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomXMLDialog(ConnectorMode mode = ConnectorMode::Read, QWidget *parent = nullptr);

        QMap<QString, QVariant> getParameters() const;

        static QMap<QString, QVariant> getXMLReadParameters(QWidget *parent = nullptr);
        static QMap<QString, QVariant> getXMLWriteParameters(QWidget *parent = nullptr);

        ConnectorMode getMode() const { return m_mode; }

    private slots:
        void onBrowseClicked();
        void onAccept();
        void onReject();

    private:
        void setupUI();
        void validateAndAccept();
        void updateUIForMode();

        QLineEdit *m_fileEdit;
        QLineEdit *m_rootEdit;
        QLineEdit *m_rowEdit;
        QPushButton *m_okButton;
        QPushButton *m_browseButton;

        QMap<QString, QVariant> m_params;
        ConnectorMode m_mode;
        bool m_accepted;
    };

}

#endif // CUSTOMXMLDIALOG_H