/**
 * @file customwebdialog.h
 * @brief Dialogue pour charger des données depuis une page web
 *
 * Paramètres:
 * - url: URL de la page web
 * - tableIndex: Index du tableau à extraire
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMWEBDIALOG_H
#define CUSTOMWEBDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

using namespace dn::core;

namespace dn::dialogs {

    class CustomWebDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomWebDialog(const QString& defaultNodeName = QString(),
                                 QWidget *parent = nullptr);

        QMap<QString, QVariant> getParameters() const;

        static QMap<QString, QVariant> getWebReadParameters(QWidget *parent = nullptr,
                                                            const QString& defaultNodeName = "Web Source");

    private slots:
        void onAccept();
        void onReject();

    private:
        void setupUI();

        QLineEdit *m_nameEdit;
        QLineEdit *m_urlEdit;
        QSpinBox *m_tableIndexSpin;
        QPushButton *m_okButton;

        QMap<QString, QVariant> m_params;
        QString m_defaultNodeName;
        bool m_accepted;
    };

}

#endif // CUSTOMWEBDIALOG_H