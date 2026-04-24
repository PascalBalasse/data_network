/**
 * @file customxmldialog.cpp
 * @brief Implémentation du dialogue XML
 *
 * @see customxmldialog.h
 */

#include "customxmldialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

using namespace dn::dialogs;

CustomXMLDialog::CustomXMLDialog(ConnectorMode mode, const QString& defaultNodeName, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_defaultNodeName(defaultNodeName)
    , m_accepted(false)
{
    setupUI();
    updateUIForMode();
}

void CustomXMLDialog::setupUI()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier XML");
    } else {
        setWindowTitle("Export vers un fichier XML");
    }
    setMinimumWidth(450);
    setMinimumHeight(280);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_nameLabel = new QLabel("<b>Nom du nœud :</b>", this);
    mainLayout->addWidget(m_nameLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(20);
    m_nameEdit->setText(m_defaultNodeName.trimmed().left(20));
    mainLayout->addWidget(m_nameEdit);

    m_fileLabel = new QLabel("Fichier:", this);
    mainLayout->addWidget(m_fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText(m_mode == ConnectorMode::Read ?
                                       "Sélectionnez un fichier XML à charger..." :
                                       "Sélectionnez l'emplacement de sauvegarde...");
    fileLayout->addWidget(m_fileEdit);

    m_browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(m_browseButton);
    mainLayout->addLayout(fileLayout);

    if (m_mode == ConnectorMode::Write) {
        QLabel *rootLabel = new QLabel("Élément racine:", this);
        mainLayout->addWidget(rootLabel);
        m_rootEdit = new QLineEdit("data", this);
        mainLayout->addWidget(m_rootEdit);

        QLabel *rowLabel = new QLabel("Élément ligne:", this);
        mainLayout->addWidget(rowLabel);
        m_rowEdit = new QLineEdit("row", this);
        mainLayout->addWidget(m_rowEdit);
    }

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

    connect(m_browseButton, &QPushButton::clicked, this, &CustomXMLDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CustomXMLDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomXMLDialog::onReject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void CustomXMLDialog::updateUIForMode()
{
    if (m_fileLabel) {
        if (m_mode == ConnectorMode::Read) {
            m_fileLabel->setText("<b>Fichier XML à charger :</b>");
        } else {
            m_fileLabel->setText("<b>Fichier XML de destination :</b>");
        }
    }

    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier XML");
    } else {
        setWindowTitle("Export vers un fichier XML");
    }
}

void CustomXMLDialog::onBrowseClicked()
{
    QString title = m_mode == ConnectorMode::Read ? "Ouvrir un fichier XML" : "Enregistrer sous";
    QString filter = "Fichiers XML (*.xml);;Tous les fichiers (*)";

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

void CustomXMLDialog::onAccept()
{
    validateAndAccept();
    if (m_accepted) {
        accept();
    }
}

void CustomXMLDialog::onReject()
{
    reject();
}

void CustomXMLDialog::validateAndAccept()
{
    QString fileName = m_fileEdit->text().trimmed();
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un fichier");
        return;
    }

    m_params["fileName"] = fileName;
    m_params["nodeName"] = m_nameEdit->text().trimmed();
    if (m_mode == ConnectorMode::Write) {
        m_params["rootElement"] = m_rootEdit->text().trimmed();
        m_params["rowElement"] = m_rowEdit->text().trimmed();
    }
    m_accepted = true;
}

QMap<QString, QVariant> CustomXMLDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomXMLDialog::getXMLReadParameters(QWidget *parent, const QString& defaultNodeName)
{
    CustomXMLDialog dialog(ConnectorMode::Read, defaultNodeName, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> CustomXMLDialog::getXMLWriteParameters(QWidget *parent, const QString& defaultNodeName)
{
    CustomXMLDialog dialog(ConnectorMode::Write, defaultNodeName, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }
    return QMap<QString, QVariant>();
}