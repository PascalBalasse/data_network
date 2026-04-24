#include "customfecdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

using namespace dn::dialogs;

CustomFECDialog::CustomFECDialog(const QString& defaultNodeName, QWidget *parent)
    : QDialog(parent)
    , m_defaultNodeName(defaultNodeName)
{
    setWindowTitle("Chargement d'un fichier FEC");
    setMinimumWidth(500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *nameLabel = new QLabel("<b>Nom du nœud :</b>", this);
    mainLayout->addWidget(nameLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(20);
    m_nameEdit->setText(m_defaultNodeName.trimmed().left(20));
    mainLayout->addWidget(m_nameEdit);

    QLabel *infoLabel = new QLabel(
        "<b>Fichier des Écritures Comptables (FEC)</b><br>"
        "Format standard français pour la transmission des écritures comptables.", this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    QLabel *fileLabel = new QLabel("<b>Fichier FEC à charger :</b>", this);
    mainLayout->addWidget(fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText("Sélectionnez un fichier FEC...");
    fileLayout->addWidget(m_fileEdit);

    QPushButton *browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(browseButton);
    mainLayout->addLayout(fileLayout);

    QLabel *formatLabel = new QLabel(
        "Format attendu : fichier texte délimité par tabulation ou pipe (|)", this);
    formatLabel->setStyleSheet("QLabel { color: gray; font-size: 11px; }");
    mainLayout->addWidget(formatLabel);

    mainLayout->addStretch();

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

    connect(browseButton, &QPushButton::clicked, this, &CustomFECDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void CustomFECDialog::onBrowseClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Ouvrir un fichier FEC",
        QString(),
        "FEC Files (*.fec);;Text Files (*.txt);;All Files (*)"
        );

    if (!fileName.isEmpty()) {
        m_fileEdit->setText(fileName);
    }
}

QMap<QString, QVariant> CustomFECDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomFECDialog::getFECReadParameters(QWidget *parent, const QString& defaultNodeName)
{
    CustomFECDialog dialog(defaultNodeName, parent);

    if (dialog.exec() == QDialog::Accepted) {
        QMap<QString, QVariant> params;
        params["nodeName"] = dialog.m_nameEdit->text().trimmed();
        params["fileName"] = dialog.m_fileEdit->text();
        params["hasHeader"] = true;
        return params;
    }

    return QMap<QString, QVariant>();
}
