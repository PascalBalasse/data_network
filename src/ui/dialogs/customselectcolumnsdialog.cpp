#include "customselectcolumnsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QDebug>
#include <QHeaderView>

using namespace dn::dialogs;

CustomSelectColumnsDialog::CustomSelectColumnsDialog(const dn::core::DataTable* table,
                                                     QWidget *parent)
    : QDialog(parent)
    , m_table(table)
    , m_previewModel(new dn::ui::DataTableModel())
    , m_searchEdit(nullptr)
    , m_columnList(nullptr)
    , m_previewTable(nullptr)
    , m_okButton(nullptr)
    , m_selectAllButton(nullptr)
    , m_deselectAllButton(nullptr)
{
    setupUI();

    if (m_table) {
        m_allColumns = m_table->columnNames();
        m_filteredColumns = m_allColumns;

        for (const QString& col : m_allColumns) {
            m_selectedSet.insert(col);
        }
        m_selectedColumns = m_allColumns;

        updateColumnList();
        updateStats();
        updatePreview();
    }
}

void CustomSelectColumnsDialog::setupUI()
{
    setWindowTitle("Sélectionner les colonnes");
    setMinimumWidth(700);
    setMinimumHeight(600);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *instructionLabel = new QLabel(
        "<b>Sélectionnez les colonnes à conserver :</b><br>"
        "Cochez les colonnes que vous souhaitez garder dans le résultat.", this);
    instructionLabel->setWordWrap(true);
    mainLayout->addWidget(instructionLabel);

    QGroupBox *searchGroup = new QGroupBox("Rechercher", this);
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Filtrer les colonnes...");
    m_searchEdit->setClearButtonEnabled(true);
    searchLayout->addWidget(m_searchEdit);

    mainLayout->addWidget(searchGroup);

    QGroupBox *columnsGroup = new QGroupBox("Colonnes disponibles", this);
    QVBoxLayout *columnsLayout = new QVBoxLayout(columnsGroup);

    m_columnList = new QListWidget(this);
    m_columnList->setSelectionMode(QAbstractItemView::MultiSelection);
    columnsLayout->addWidget(m_columnList);

    mainLayout->addWidget(columnsGroup);

    QHBoxLayout *quickSelectLayout = new QHBoxLayout();
    m_selectAllButton = new QPushButton("Tout sélectionner", this);
    m_deselectAllButton = new QPushButton("Tout désélectionner", this);
    quickSelectLayout->addWidget(m_selectAllButton);
    quickSelectLayout->addWidget(m_deselectAllButton);
    quickSelectLayout->addStretch();
    mainLayout->addLayout(quickSelectLayout);

    QGroupBox *previewGroup = new QGroupBox("Aperçu du résultat", this);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);

    m_previewTable = new QTableView(this);
    m_previewTable->setModel(m_previewModel);
    m_previewTable->setAlternatingRowColors(true);
    m_previewTable->setMinimumHeight(150);
    previewLayout->addWidget(m_previewTable);

    mainLayout->addWidget(previewGroup);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("Appliquer", this);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    QPushButton *cancelButton = new QPushButton("Annuler", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &CustomSelectColumnsDialog::onSearchTextChanged);
    connect(m_selectAllButton, &QPushButton::clicked,
            this, &CustomSelectColumnsDialog::onSelectAll);
    connect(m_deselectAllButton, &QPushButton::clicked,
            this, &CustomSelectColumnsDialog::onDeselectAll);
    connect(m_okButton, &QPushButton::clicked,
            this, &CustomSelectColumnsDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked,
            this, &CustomSelectColumnsDialog::onReject);
    connect(m_columnList, &QListWidget::itemChanged,
            this, &CustomSelectColumnsDialog::onSelectionChanged);
}

void CustomSelectColumnsDialog::updateColumnList()
{
    if (m_columnList == nullptr) return;

    m_columnList->clear();
    m_columnList->setUpdatesEnabled(false);

    for (const QString &column : m_filteredColumns) {
        QListWidgetItem *item = new QListWidgetItem(column);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(m_selectedSet.contains(column) ? Qt::Checked : Qt::Unchecked);
        m_columnList->addItem(item);
    }

    m_columnList->setUpdatesEnabled(true);
}

void CustomSelectColumnsDialog::updateStats()
{
    updatePreview();
}

void CustomSelectColumnsDialog::updatePreview()
{
    if (m_previewTable == nullptr || m_previewModel == nullptr) return;
    if (m_table == nullptr) return;
    if (m_selectedColumns.isEmpty()) return;

    m_previewData = std::make_unique<dn::core::DataTable>();
    m_previewData->setColumnNames(m_selectedColumns);

    int maxRows = qMin(m_table->rowCount(), 10);
    for (int row = 0; row < maxRows; ++row) {
        QList<QVariant> rowData;
        for (const QString &col : m_selectedColumns) {
            int colIdx = m_table->columnIndex(col);
            if (colIdx >= 0) {
                rowData.append(m_table->value(row, colIdx));
            } else {
                rowData.append(QVariant());
            }
        }
        m_previewData->addRow(rowData);
    }

    m_previewModel->setTable(m_previewData.get());
}

void CustomSelectColumnsDialog::onSearchTextChanged(const QString& text)
{
    m_filteredColumns.clear();

    if (text.isEmpty()) {
        m_filteredColumns = m_allColumns;
    } else {
        for (const QString& column : m_allColumns) {
            if (column.contains(text, Qt::CaseInsensitive)) {
                m_filteredColumns.append(column);
            }
        }
    }

    updateColumnList();
}

void CustomSelectColumnsDialog::onSelectAll()
{
    for (int i = 0; i < m_columnList->count(); ++i) {
        QListWidgetItem *item = m_columnList->item(i);
        if (item) {
            item->setCheckState(Qt::Checked);
            m_selectedSet.insert(item->text());
        }
    }
    updateStats();
}

void CustomSelectColumnsDialog::onDeselectAll()
{
    for (int i = 0; i < m_columnList->count(); ++i) {
        QListWidgetItem *item = m_columnList->item(i);
        if (item) {
            item->setCheckState(Qt::Unchecked);
            m_selectedSet.remove(item->text());
        }
    }
    updateStats();
}

void CustomSelectColumnsDialog::onSelectionChanged()
{
    // Mettre à jour m_selectedSet en fonction de l'état des cases
    for (int i = 0; i < m_columnList->count(); ++i) {
        QListWidgetItem *item = m_columnList->item(i);
        if (item) {
            if (item->checkState() == Qt::Checked) {
                m_selectedSet.insert(item->text());
            } else {
                m_selectedSet.remove(item->text());
            }
        }
    }

    // Mettre à jour m_selectedColumns dans l'ordre original
    m_selectedColumns.clear();
    for (const QString& col : m_allColumns) {
        if (m_selectedSet.contains(col)) {
            m_selectedColumns.append(col);
        }
    }

    updateStats();
}

void CustomSelectColumnsDialog::onAccept()
{
    if (m_selectedColumns.isEmpty()) {
        QMessageBox::warning(this, "Aucune colonne sélectionnée",
                             "Veuillez sélectionner au moins une colonne à conserver.");
        return;
    }

    accept();
}

void CustomSelectColumnsDialog::onReject()
{
    reject();
}

// Méthode statique
bool CustomSelectColumnsDialog::getSelectedColumns(QWidget *parent,
                                                   const dn::core::DataTable* table,
                                                   QStringList& columns)
{
    if (!table || table->rowCount() == 0) {
        QMessageBox::warning(parent, "Erreur",
                             "Aucune donnée disponible pour sélectionner des colonnes.");
        return false;
    }

    CustomSelectColumnsDialog dialog(table, parent);

    if (dialog.exec() == QDialog::Accepted) {
        columns = dialog.getSelectedColumns();
        return true;
    }

    return false;
}

