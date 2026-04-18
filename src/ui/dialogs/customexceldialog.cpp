#include "customexceldialog.h"
#include "../../network/connectors/ExcelConnector.h"
#include "../../network/core/DataTable.h"
#include "../../ui/models/DataTableModel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QComboBox>
#include <QHeaderView>
#include <QTimer>

using namespace dn::dialogs;

CustomExcelDialog::CustomExcelDialog(ConnectorMode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_accepted(false)
    , m_previewTable(nullptr)
    , m_previewModel(nullptr)
{
    m_previewModel = new dn::ui::DataTableModel(this);
    setupUI();
    updateUIForMode();
}

void CustomExcelDialog::setupUI()
{
    if (m_mode == ConnectorMode::Read) {
        setWindowTitle("Chargement d'un fichier Excel");
    } else {
        setWindowTitle("Export vers un fichier Excel");
    }
    setMinimumWidth(500);
    setMinimumHeight(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === File selection ===
    QLabel *fileLabel = new QLabel(this);
    mainLayout->addWidget(fileLabel);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText(m_mode == ConnectorMode::Read ?
                                        "Sélectionnez un fichier Excel à charger..." :
                                        "Sélectionnez l'emplacement de sauvegarde...");
    fileLayout->addWidget(m_fileEdit);

    m_browseButton = new QPushButton("Parcourir...", this);
    fileLayout->addWidget(m_browseButton);
    mainLayout->addLayout(fileLayout);

    // === Sheet selection (Read mode only) ===
    QGroupBox *sheetGroup = new QGroupBox("Feuille de calcul", this);
    QVBoxLayout *sheetLayout = new QVBoxLayout(sheetGroup);

    QHBoxLayout *sheetSelectLayout = new QHBoxLayout();
    QLabel *sheetLabel = new QLabel("Feuille:", this);
    sheetSelectLayout->addWidget(sheetLabel);

    m_sheetCombo = new QComboBox(this);
    m_sheetCombo->addItem("(Sélectionnez d'abord un fichier)");
    m_sheetCombo->setEnabled(false);
    sheetSelectLayout->addWidget(m_sheetCombo);
    sheetSelectLayout->addStretch();

    sheetLayout->addLayout(sheetSelectLayout);
    mainLayout->addWidget(sheetGroup);

    // === Options ===
    QGroupBox *optionsGroup = new QGroupBox("Options", this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);

    if (m_mode == ConnectorMode::Read) {
        m_headerCheck = new QCheckBox("La première ligne contient les en-têtes", this);
        m_headerCheck->setChecked(true);
        optionsLayout->addWidget(m_headerCheck);
    } else {
        m_headerCheck = new QCheckBox("Inclure les en-têtes dans le fichier", this);
        m_headerCheck->setChecked(true);
        optionsLayout->addWidget(m_headerCheck);
    }

    mainLayout->addWidget(optionsGroup);

    // === Preview (Read mode only) ===
    if (m_mode == ConnectorMode::Read) {
        QGroupBox *previewGroup = new QGroupBox("Aperçu", this);
        QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);

        m_previewTable = new QTableView(this);
        m_previewTable->setModel(m_previewModel);
        m_previewTable->setMinimumHeight(150);
        m_previewTable->setAlternatingRowColors(true);
        previewLayout->addWidget(m_previewTable);

        mainLayout->addWidget(previewGroup);
    } else {
        m_previewTable = nullptr;
    }

    // === Buttons ===
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

    // Connections
    connect(m_browseButton, &QPushButton::clicked, this, &CustomExcelDialog::onBrowseClicked);
    connect(m_okButton, &QPushButton::clicked, this, &CustomExcelDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &CustomExcelDialog::onReject);
    connect(m_fileEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_okButton->setEnabled(!text.trimmed().isEmpty());
    });
    connect(m_sheetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CustomExcelDialog::onSheetSelected);
}

void CustomExcelDialog::updateUIForMode()
{
    QLabel *fileLabel = findChild<QLabel*>();
    if (fileLabel) {
        if (m_mode == ConnectorMode::Read) {
            fileLabel->setText("<b>Fichier Excel à charger :</b>");
        } else {
            fileLabel->setText("<b>Fichier Excel de destination :</b>");
        }
    }
}

void CustomExcelDialog::onBrowseClicked()
{
    QString fileName;

    if (m_mode == ConnectorMode::Read) {
        fileName = QFileDialog::getOpenFileName(
            this,
            "Ouvrir un fichier Excel",
            QString(),
            "Excel Files (*.xlsx *.xls);;All Files (*)"
            );
    } else {
        fileName = QFileDialog::getSaveFileName(
            this,
            "Sauvegarder le fichier Excel",
            QString(),
            "Excel Files (*.xlsx);;All Files (*)",
            nullptr,
            QFileDialog::Option::DontConfirmOverwrite
            );

        if (fileName.isEmpty())
            return;

        if (!fileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
            fileName += ".xlsx";
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
        onFileSelected(fileName);
    }
}

void CustomExcelDialog::onFileSelected(const QString& fileName)
{
    if (m_mode == ConnectorMode::Read) {
        m_sheetCombo->clear();
        m_sheetCombo->addItem("(Chargement...)");
        m_sheetCombo->setEnabled(false);

        m_sheetCombo->clear();
        m_sheetCombo->addItem("Sheet1");
        m_sheetCombo->setEnabled(true);

        QTimer::singleShot(100, this, &CustomExcelDialog::loadPreview);
    }
}

void CustomExcelDialog::onSheetSelected(int index)
{
    Q_UNUSED(index);
    if (!m_previewLoaded) {
        loadPreview();
        m_previewLoaded = true;
    }
}

void CustomExcelDialog::loadPreview()
{
    QString fileName = m_fileEdit->text();
    if (fileName.isEmpty() || m_mode != ConnectorMode::Read) {
        m_previewTableData.reset();
        m_previewModel->setTable(nullptr);
        m_okButton->setEnabled(false);
        return;
    }

    QFile file(fileName);
    if (!file.exists()) {
        m_previewTableData.reset();
        m_previewModel->setTable(nullptr);
        m_okButton->setEnabled(false);
        return;
    }

    auto connector = std::make_unique<dn::connectors::ExcelConnector>();
    QMap<QString, QVariant> params;
    params["fileName"] = fileName;
    params["hasHeader"] = m_headerCheck->isChecked();
    params["sheetName"] = m_sheetCombo->currentText();
    connector->configure(params);

    m_previewTableData = connector->load();
    if (m_previewTableData && m_previewTableData->rowCount() > 0) {
        m_previewModel->setTable(m_previewTableData.get());
        m_okButton->setEnabled(true);
    } else {
        m_previewTableData.reset();
        m_previewModel->setTable(nullptr);
        m_okButton->setEnabled(false);
    }
}

void CustomExcelDialog::onAccept()
{
    validateAndAccept();
}

void CustomExcelDialog::onReject()
{
    m_accepted = false;
    reject();
}

void CustomExcelDialog::validateAndAccept()
{
    if (m_fileEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             m_mode == ConnectorMode::Read ?
                                 "Veuillez sélectionner un fichier Excel à charger." :
                                 "Veuillez sélectionner un emplacement pour sauvegarder le fichier.");
        return;
    }

    if (m_mode == ConnectorMode::Read) {
        QFile file(m_fileEdit->text());
        if (!file.exists()) {
            QMessageBox::warning(this, "Erreur", "Le fichier sélectionné n'existe pas.");
            return;
        }
    }

    // Store parameters
    m_params.clear();
    m_params["fileName"] = m_fileEdit->text();
    m_params["hasHeader"] = m_headerCheck->isChecked();

    if (m_sheetCombo->isEnabled() && m_sheetCombo->currentIndex() >= 0) {
        m_params["sheetName"] = m_sheetCombo->currentText();
    } else {
        m_params["sheetName"] = "Sheet1";
    }

    m_params["mode"] = (m_mode == ConnectorMode::Read) ? "read" : "write";

    m_accepted = true;
    accept();
}

QMap<QString, QVariant> CustomExcelDialog::getParameters() const
{
    return m_params;
}

QMap<QString, QVariant> CustomExcelDialog::getExcelReadParameters(QWidget *parent)
{
    CustomExcelDialog dialog(ConnectorMode::Read, parent);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }

    return QMap<QString, QVariant>();
}

QMap<QString, QVariant> CustomExcelDialog::getExcelWriteParameters(QWidget *parent)
{
    CustomExcelDialog dialog(ConnectorMode::Write, parent);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getParameters();
    }

    return QMap<QString, QVariant>();
}
