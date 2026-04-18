/**
 * @file customjsondialog.cpp
 * @brief Implémentation du dialogue JSON
 *
 * @see customjsondialog.h
 */

#include "customjsondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

using namespace dn::dialogs;

/// Constructeur
CustomJSONDialog::CustomJSONDialog(ConnectorMode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_accepted(false)
{
    setupUI();
    updateUIForMode();
}

/// Configure l'interface
void CustomJSONDialog::setupUI()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier JSON");
    } else {
        setWindowTitle("Export vers un fichier JSON");
    }
    setMinimumWidth(450);
    setMinimumHeight(250);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Sélection du fichier ===
    QLabel *fileLabel = new QLabel("Fichier:", this);
    mainLayout->addWidget(fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText(m_mode == ConnectorMode::Read ?
                                       "Sélectionnez un fichier JSON à charger..." :
                                       "Sélectionnez l'emplacement de sauvegarde...");
    fileLayout->addWidget(m_fileEdit);

    m_browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(m_browseButton);
    mainLayout->addLayout(fileLayout);

    // === Options JSON ===
    if (m_mode == ConnectorMode::Write) {
        m_prettyPrintCheck = new QCheckBox("Formatage lisible (pretty print)", this);
        m_prettyPrintCheck->setChecked(true);
        mainLayout->addWidget(m_prettyPrintCheck);
    }

    // === Boutons ===
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton(m_mode == ConnectorMode::Read ? "Charger" : "Exporter", this);
    QPushButton *cancelButton = new QPushButton("Annuler", this);

    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // Connexions
    connect(m_browseButton, &QPushButton::clicked, this, &CustomJSONDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CustomJSONDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomJSONDialog::onReject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void CustomJSONDialog::updateUIForMode()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier JSON");
    } else {
        setWindowTitle("Export vers un fichier JSON");
    }
}

void CustomJSONDialog::onBrowseClicked()
{
    QString title = m_mode == ConnectorMode::Read ? "Ouvrir un fichier JSON" : "Enregistrer sous";
    QString filter = "Fichiers JSON (*.json);;Tous les fichiers (*)";

    QString fileName;
    if (m_mode == ConnectorMode::Read) {
        fileName = QFileDialog::getOpenFileName(this, title, QString(), filter);
    } else {
        fileName = QFileDialog::getSaveFileName(this, title, QString(), filter);
    }

    if (!fileName.isEmpty()) {
        m_fileEdit->setText(fileName);
    }
}

void CustomJSONDialog::onAccept()
{
    validateAndAccept();
    if (m_accepted) {
        accept();
    }
}

void CustomJSONDialog::onReject()
{
    reject();
}

void CustomJSONDialog::validateAndAccept()
{
    QString fileName = m_fileEdit->text().trimmed();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un fichier");
        return;
    }

    m_params["fileName"] = fileName;
    if (m_mode == ConnectorMode::Write) {
        m_params["prettyPrint"] = m_prettyPrintCheck->isChecked();
    }
    m_accepted = true;
}

QMap<QString, QVariant> CustomJSONDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomJSONDialog::getJSONReadParameters(QWidget *parent)
{
    CustomJSONDialog dialog(ConnectorMode::Read, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> CustomJSONDialog::getJSONWriteParameters(QWidget *parent)
{
    CustomJSONDialog dialog(ConnectorMode::Write, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}