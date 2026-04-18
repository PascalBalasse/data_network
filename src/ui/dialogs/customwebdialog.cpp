/**
 * @file customwebdialog.cpp
 * @brief Implémentation du dialogue web
 *
 * @see customwebdialog.h
 */

#include "customwebdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

using namespace dn::dialogs;

CustomWebDialog::CustomWebDialog(QWidget *parent)
    : QDialog(parent)
    , m_accepted(false)
{
    setupUI();
}

void CustomWebDialog::setupUI()
{
    setWindowTitle("Charger depuis une page web");
    setMinimumWidth(500);
    setMinimumHeight(200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *urlLabel = new QLabel("URL de la page web:", this);
    mainLayout->addWidget(urlLabel);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://example.com/table-page.html");
    mainLayout->addWidget(m_urlEdit);

    QLabel *tableLabel = new QLabel("Numéro du tableau (0 = premier):", this);
    mainLayout->addWidget(tableLabel);

    m_tableIndexSpin = new QSpinBox(this);
    m_tableIndexSpin->setMinimum(0);
    m_tableIndexSpin->setMaximum(99);
    m_tableIndexSpin->setValue(0);
    mainLayout->addWidget(m_tableIndexSpin);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("Charger", this);
    QPushButton *cancelButton = new QPushButton("Annuler", this);

    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_okButton, &QPushButton::clicked, this, &CustomWebDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomWebDialog::onReject);
    connect(m_urlEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty() && text.startsWith("http"));
    });
}

void CustomWebDialog::onAccept()
{
    QString url = m_urlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer une URL");
        return;
    }

    m_params["url"] = url;
    m_params["tableIndex"] = m_tableIndexSpin->value();
    m_accepted = true;
    accept();
}

void CustomWebDialog::onReject()
{
    reject();
}

QMap<QString, QVariant> CustomWebDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomWebDialog::getWebReadParameters(QWidget *parent)
{
    CustomWebDialog dialog(parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}