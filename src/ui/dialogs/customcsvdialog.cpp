/**
 * @file customcsvdialog.cpp
 * @brief Implémentation du dialogue CSV
 *
 * @see customcsvdialog.h
 */

#include "customcsvdialog.h"
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
CustomCSVDialog::CustomCSVDialog(ConnectorMode mode, const QString& defaultNodeName, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_defaultNodeName(defaultNodeName)
    , m_accepted(false)
{
    setupUI();
    updateUIForMode();
}

/// Configure l'interface
void CustomCSVDialog::setupUI()
{
    // Titre selon le mode
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier CSV");
    } else {
        setWindowTitle("Export vers un fichier CSV");
    }
    setMinimumWidth(450);
    setMinimumHeight(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Nom du noeud ===
    m_nameLabel = new QLabel("<b>Nom du nœud :</b>", this);
    mainLayout->addWidget(m_nameLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(20);
    m_nameEdit->setPlaceholderText("Nom du nœud CSV");
    m_nameEdit->setText(m_defaultNodeName.trimmed().left(20));
    mainLayout->addWidget(m_nameEdit);

    // === Groupe d'information selon le mode ===
  //  QGroupBox *infoGroup = new QGroupBox(this);
  //  QVBoxLayout *infoLayout = new QVBoxLayout(infoGroup);

 //   mainLayout->addWidget(infoGroup);

    // === Sélection du fichier ===
    m_fileLabel = new QLabel(this);
    mainLayout->addWidget(m_fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText(m_mode == ConnectorMode::Read ?
                                       "Sélectionnez un fichier CSV à charger..." :
                                       "Sélectionnez l'emplacement de sauvegarde...");
    fileLayout->addWidget(m_fileEdit);

    m_browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(m_browseButton);
    mainLayout->addLayout(fileLayout);

    // === Options CSV (communes aux deux modes) ===
    QGroupBox *optionsGroup = new QGroupBox("Options CSV", this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);

    // En-têtes (mode lecture) ou inclure les en-têtes (mode écriture)
    if (m_mode == ConnectorMode::Read) {
        m_headerCheck = new QCheckBox("La première ligne contient les en-têtes", this);
        m_headerCheck->setChecked(true);
    } else {
        m_headerCheck = new QCheckBox("Inclure les en-têtes dans le fichier", this);
        m_headerCheck->setChecked(true);
    }
    optionsLayout->addWidget(m_headerCheck);

    // Séparateur
    QHBoxLayout *sepLayout = new QHBoxLayout();
    QLabel *sepLabel = new QLabel("Séparateur:", this);
    sepLayout->addWidget(sepLabel);

    m_sepCombo = new QComboBox(this);
    m_sepCombo->addItem("Virgule (,)", ",");
    m_sepCombo->addItem("Point-virgule (;)", ";");
    m_sepCombo->addItem("Tabulation", "\t");
    m_sepCombo->addItem("Espace", " ");
    m_sepCombo->addItem("Pipe (|)", "|");
    sepLayout->addWidget(m_sepCombo);
    optionsLayout->addLayout(sepLayout);

    // Options spécifiques au mode écriture
    if (m_mode == ConnectorMode::Write) {
        QHBoxLayout *encodingLayout = new QHBoxLayout();
        QLabel *encodingLabel = new QLabel("Encodage:", this);
        encodingLayout->addWidget(encodingLabel);

        QComboBox *encodingCombo = new QComboBox(this);
        encodingCombo->addItem("UTF-8", "UTF-8");
        encodingCombo->addItem("UTF-8 with BOM", "UTF-8-BOM");
        encodingCombo->addItem("ISO-8859-1 (Latin-1)", "ISO-8859-1");
        encodingLayout->addWidget(encodingCombo);
        optionsLayout->addLayout(encodingLayout);

        // Stocker le pointeur pour récupérer la valeur plus tard
        encodingCombo->setObjectName("encodingCombo");
    }

    mainLayout->addWidget(optionsGroup);

    // === Aperçu (mode lecture seulement) ===
    if (m_mode == ConnectorMode::Read) {
        QGroupBox *previewGroup = new QGroupBox("Aperçu", this);
        QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);

        QLabel *previewLabel = new QLabel("Sélectionnez un fichier pour voir un aperçu", this);
        previewLabel->setWordWrap(true);
        previewLabel->setStyleSheet("QLabel { background-color: #f5f5f5; padding: 10px; font-family: monospace; }");
        previewLabel->setMinimumHeight(100);
        previewLabel->setObjectName("previewLabel");
        previewLayout->addWidget(previewLabel);

        mainLayout->addWidget(previewGroup);
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
    connect(m_browseButton, &QPushButton::clicked, this, &CustomCSVDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CustomCSVDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomCSVDialog::onReject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void CustomCSVDialog::updateUIForMode()
{
    if (m_fileLabel) {
        if (m_mode == ConnectorMode::Read) {
            m_fileLabel->setText("<b>Fichier CSV à charger :</b>");
        } else {
            m_fileLabel->setText("<b>Fichier CSV de destination :</b>");
        }
    }
}

void CustomCSVDialog::onBrowseClicked()
{
    QString fileName;

    if (m_mode == ConnectorMode::Read) {
        fileName = QFileDialog::getOpenFileName(
            this,
            "Ouvrir un fichier CSV",
            QString(),
            "CSV Files (*.csv);;All Files (*)"
            );

        // Afficher un aperçu si possible
        if (!fileName.isEmpty() && m_mode == ConnectorMode::Read) {
            QLabel *previewLabel = findChild<QLabel*>("previewLabel");
            if (previewLabel) {
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream stream(&file);
                    QStringList previewLines;
                    for (int i = 0; i < 5 && !stream.atEnd(); ++i) {
                        QString line = stream.readLine();
                        if (line.length() > 100) {
                            line = line.left(100) + "...";
                        }
                        previewLines << line;
                    }
                    file.close();

                    if (!previewLines.isEmpty()) {
                        previewLabel->setText("<b>Aperçu des 5 premières lignes :</b><br>" +
                                              previewLines.join("<br>"));
                    } else {
                        previewLabel->setText("Fichier vide");
                    }
                } else {
                    previewLabel->setText("Impossible d'ouvrir le fichier pour l'aperçu");
                }
            }
        }
    } else {
        fileName = QFileDialog::getSaveFileName(
            this,
            "Sauvegarder le fichier CSV",
            QString(),
            "CSV Files (*.csv);;All Files (*)",
            nullptr,
            QFileDialog::Option::DontConfirmOverwrite
            );

        if (fileName.isEmpty())
            return;

        if (!fileName.endsWith(".csv", Qt::CaseInsensitive)) {
            fileName += ".csv";
        }

        QFile file(fileName);
        if (file.exists()) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Fichier existant");
            msgBox.setText("Le fichier existe déjà. Voulez-vous l'écraser ?");
            QPushButton *yesButton = msgBox.addButton("Oui", QMessageBox::YesRole);
            QPushButton *noButton = msgBox.addButton("Non", QMessageBox::NoRole);
            msgBox.setDefaultButton(noButton);
            msgBox.exec();

            if (msgBox.clickedButton() != yesButton) {
                m_fileEdit->clear();
                return;
            }
        }
    }

    if (!fileName.isEmpty()) {
        m_fileEdit->setText(fileName);
    }
}

void CustomCSVDialog::onAccept()
{
    validateAndAccept();
}

void CustomCSVDialog::onReject()
{
    m_accepted = false;
    reject();
}

void CustomCSVDialog::validateAndAccept()
{
    // Validation
    if (m_fileEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             m_mode == ConnectorMode::Read ?
                                 "Veuillez sélectionner un fichier CSV à charger." :
                                 "Veuillez sélectionner un emplacement pour sauvegarder le fichier.");
        return;
    }

    // Vérifier que le fichier existe en mode lecture
    if (m_mode == ConnectorMode::Read) {
        QFile file(m_fileEdit->text());
        if (!file.exists()) {
            QMessageBox::warning(this, "Erreur", "Le fichier sélectionné n'existe pas.");
            return;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier en lecture.");
            return;
        }
        file.close();
    }

    // Stocker les paramètres
    m_params.clear();
    m_params["nodeName"] = m_nameEdit->text().trimmed();
    m_params["fileName"] = m_fileEdit->text();
    m_params["hasHeader"] = m_headerCheck->isChecked();
    m_params["separator"] = m_sepCombo->currentData().toString();

    // Ajouter le mode comme paramètre
    m_params["mode"] = (m_mode == ConnectorMode::Read) ? "read" : "write";

    // Récupérer l'encodage en mode écriture
    if (m_mode == ConnectorMode::Write) {
        QComboBox *encodingCombo = findChild<QComboBox*>("encodingCombo");
        if (encodingCombo) {
            m_params["encoding"] = encodingCombo->currentData().toString();
        } else {
            m_params["encoding"] = "UTF-8";
        }
    }

    m_accepted = true;
    accept();
}

QMap<QString, QVariant> CustomCSVDialog::getParameters() const
{
    return m_params;
}

// Méthodes statiques
QMap<QString, QVariant> CustomCSVDialog::getCSVReadParameters(QWidget *parent, const QString& defaultNodeName)
{
    CustomCSVDialog dialog(ConnectorMode::Read, defaultNodeName, parent);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }

    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> CustomCSVDialog::getCSVWriteParameters(QWidget *parent, const QString& defaultNodeName)
{
    CustomCSVDialog dialog(ConnectorMode::Write, defaultNodeName, parent);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }

    return QMap<QString, QVariant>();
}