#ifndef CUSTOMFECDIALOG_H
#define CUSTOMFECDIALOG_H

#include "../../network/core/enums.h"

#include <QDialog>
#include <QMap>
#include <QVariant>
#include <QLineEdit>

using namespace dn::core;

namespace dn::dialogs{

    class CustomFECDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomFECDialog(const QString& defaultNodeName = QString(),
                                 QWidget *parent = nullptr);

        QMap<QString, QVariant> getParameters() const;

        static QMap<QString, QVariant> getFECReadParameters(QWidget *parent = nullptr,
                                                            const QString& defaultNodeName = "FEC Source");

    private slots:
        void onBrowseClicked();

    private:
        QLineEdit *m_nameEdit;
        QLineEdit *m_fileEdit;
        QPushButton *m_okButton;
        QMap<QString, QVariant> m_params;
        QString m_defaultNodeName;
    };

}

#endif // CUSTOMFECDIALOG_H
