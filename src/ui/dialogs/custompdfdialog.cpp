#include "custompdfdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>

using namespace dn::dialogs;

CustomPDFDialog::CustomPDFDialog(ConnectorMode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_accepted(false)
{
    setupUI();
    updateUIForMode();
}

void CustomPDFDialog::setupUI()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier PDF");
    } else {
        setWindowTitle("Export vers un fichier PDF");
    }
    setMinimumWidth(450);
    setMinimumHeight(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *fileLabel = new QLabel(this);
    mainLayout->addWidget(fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText("Sélectionnez un fichier PDF à charger...");
    fileLayout->addWidget(m_fileEdit);

    m_browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(m_browseButton);
    mainLayout->addLayout(fileLayout);

    QGroupBox *optionsGroup = new QGroupBox("Options", this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);

    m_headerCheck = new QCheckBox("La première ligne contient les en-têtes", this);
    m_headerCheck->setChecked(true);
    optionsLayout->addWidget(m_headerCheck);

    QHBoxLayout *pageLayout = new QHBoxLayout();
    QLabel *startLabel = new QLabel("Page de début:", this);
    pageLayout->addWidget(startLabel);
    m_startPageCombo = new QComboBox(this);
    m_startPageCombo->addItem("1", 1);
    pageLayout->addWidget(m_startPageCombo);
    pageLayout->addStretch();

    QLabel *endLabel = new QLabel("Page de fin:", this);
    pageLayout->addWidget(endLabel);
    m_endPageCombo = new QComboBox(this);
    m_endPageCombo->addItem("Toutes", -1);
    pageLayout->addWidget(m_endPageCombo);

    optionsLayout->addLayout(pageLayout);
    mainLayout->addWidget(optionsGroup);

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

    connect(m_browseButton, &QPushButton::clicked, this, &CustomPDFDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CustomPDFDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomPDFDialog::onReject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void CustomPDFDialog::updateUIForMode()
{
    QLabel *fileLabel = findChild<QLabel*>();
    if (fileLabel) {
        fileLabel->setText("<b>Fichier PDF à charger :</b>");
    }
}

void CustomPDFDialog::onBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Ouvrir un fichier PDF",
        QString(),
        "PDF Files (*.pdf);;All Files (*)"
        );

    if (!fileName.isEmpty()) {
        m_fileEdit->setText(fileName);
    }
}

void CustomPDFDialog::onAccept()
{
    validateAndAccept();
}

void CustomPDFDialog::onReject()
{
    m_accepted = false;
    reject();
}

void CustomPDFDialog::validateAndAccept()
{
    if (m_fileEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un fichier PDF à charger.");
        return;
    }

    QFile file(m_fileEdit->text());
    if (!file.exists()) {
        QMessageBox::warning(this, "Erreur", "Le fichier sélectionné n'existe pas.");
        return;
    }

    m_params.clear();
    m_params["fileName"] = m_fileEdit->text();
    m_params["hasHeader"] = m_headerCheck->isChecked();
    m_params["startPage"] = m_startPageCombo->currentData().toInt();
    m_params["endPage"] = m_endPageCombo->currentData().toInt();
    m_params["mode"] = "read";

    m_accepted = true;
    accept();
}

QMap<QString, QVariant> CustomPDFDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomPDFDialog::getPDFReadParameters(QWidget *parent)
{
    CustomPDFDialog dialog(ConnectorMode::Read, parent);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }

    return QMap<QString, QVariant>();
}