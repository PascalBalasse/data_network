/**
 * @file customtxtdialog.cpp
 * @brief Implémentation du dialogue TXT
 *
 * @see customtxtdialog.h
 */

#include "customtxtdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

using namespace dn::dialogs;

/// Constructeur
CustomTXTDialog::CustomTXTDialog(ConnectorMode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_accepted(false)
{
    setupUI();
    updateUIForMode();
}

/// Configure l'interface
void CustomTXTDialog::setupUI()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier TXT");
    } else {
        setWindowTitle("Export vers un fichier TXT");
    }
    setMinimumWidth(450);
    setMinimumHeight(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Sélection du fichier ===
    QLabel *fileLabel = new QLabel(this);
    mainLayout->addWidget(fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText(m_mode == ConnectorMode::Read ?
                                       "Sélectionnez un fichier TXT à charger..." :
                                       "Sélectionnez l'emplacement de sauvegarde...");
    fileLayout->addWidget(m_fileEdit);

    m_browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(m_browseButton);
    mainLayout->addLayout(fileLayout);

    // === Options TXT ===
    QGroupBox *optionsGroup = new QGroupBox("Options TXT", this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);

    if (m_mode == ConnectorMode::Read) {
        m_headerCheck = new QCheckBox("La première ligne contient les en-têtes", this);
        m_headerCheck->setChecked(true);
    } else {
        m_headerCheck = new QCheckBox("Inclure les en-têtes dans le fichier", this);
        m_headerCheck->setChecked(true);
    }
    optionsLayout->addWidget(m_headerCheck);

    // Séparateur (par défaut: tabulation)
    QHBoxLayout *sepLayout = new QHBoxLayout();
    QLabel *sepLabel = new QLabel("Séparateur:", this);
    sepLayout->addWidget(sepLabel);

    m_sepCombo = new QComboBox(this);
    m_sepCombo->addItem("Tabulation", "\t");
    m_sepCombo->addItem("Point-virgule (;)", ";");
    m_sepCombo->addItem("Virgule (,)", ",");
    m_sepCombo->addItem("Espace", " ");
    m_sepCombo->addItem("Pipe (|)", "|");
    m_sepCombo->setCurrentIndex(0);
    sepLayout->addWidget(m_sepCombo);
    optionsLayout->addLayout(sepLayout);

    mainLayout->addWidget(optionsGroup);

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
    connect(m_browseButton, &QPushButton::clicked, this, &CustomTXTDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CustomTXTDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomTXTDialog::onReject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void CustomTXTDialog::updateUIForMode()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier TXT");
    } else {
        setWindowTitle("Export vers un fichier TXT");
    }
}

void CustomTXTDialog::onBrowseClicked()
{
    QString title = m_mode == ConnectorMode::Read ? "Ouvrir un fichier TXT" : "Enregistrer sous";
    QString filter = "Fichiers TXT (*.txt);;Tous les fichiers (*)";

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

void CustomTXTDialog::onAccept()
{
    validateAndAccept();
    if (m_accepted) {
        accept();
    }
}

void CustomTXTDialog::onReject()
{
    reject();
}

void CustomTXTDialog::validateAndAccept()
{
    QString fileName = m_fileEdit->text().trimmed();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un fichier");
        return;
    }

    m_params["fileName"] = fileName;
    m_params["separator"] = m_sepCombo->currentData();
    m_params["hasHeader"] = m_headerCheck->isChecked();
    m_accepted = true;
}

QMap<QString, QVariant> CustomTXTDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomTXTDialog::getTXTReadParameters(QWidget *parent)
{
    CustomTXTDialog dialog(ConnectorMode::Read, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> CustomTXTDialog::getTXTWriteParameters(QWidget *parent)
{
    CustomTXTDialog dialog(ConnectorMode::Write, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}