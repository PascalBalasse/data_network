/**
 * @file customsqldialog.h
 * @brief Dialogue de configuration SQL
 *
 * Permet de configurer la connexion à une base de données.
 * Paramètres:
 * - databaseType: Type de BDD (SQLite, MySQL, PostgreSQL)
 * - hostName, port, databaseName, userName, password
 * - tableName ou sqlQuery
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMSQLDIALOG_H
#define CUSTOMSQLDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

using namespace dn::core;

namespace dn::dialogs{

    class CustomSQLDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomSQLDialog(const QString& defaultNodeName = QString(),
                                 QWidget *parent = nullptr);

        /// Retourne les paramètres
        QMap<QString, QVariant> getParameters() const;

        /// Affiche le dialogue
        static QMap<QString, QVariant> getSQLReadParameters(QWidget *parent = nullptr,
                                                            const QString& defaultNodeName = "SQL Source");

    private slots:
        void onDatabaseTypeChanged(int index);
        void onTestConnection();

    private:
        //══════════════════════════════════════════════════════════════════
        // Widgets
        //══════════════════════════════════════════════════════════════════
        QLineEdit *m_nameEdit;
        QComboBox *m_dbTypeCombo;
        QLineEdit *m_hostEdit;
        QLineEdit *m_portEdit;
        QLineEdit *m_databaseEdit;
        QLineEdit *m_usernameEdit;
        QLineEdit *m_passwordEdit;
        QLineEdit *m_tableNameEdit;
        QLineEdit *m_sqlQueryEdit;
        QPushButton *m_okButton;
        QPushButton *m_testButton;
        QString m_defaultNodeName;
    };

}

#endif // CUSTOMSQLDIALOG_H
