#include "customfilterdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QListView>
#include <QStringListModel>
#include <QSet>
#include <QDebug>
#include <QTimer>

using namespace dn::dialogs;

CustomFilterDialog::CustomFilterDialog(const dn::core::DataTable* table,
                                       QWidget *parent)
    : QDialog(parent)
    , m_table(table)
    , m_isValid(false)
{
    // Initialiser les opérateurs par type
    m_stringOperators = QStringList() << "Equals" << "Not Equals"
                                      << "Contains" << "Starts With"
                                      << "Ends With" << "Is Empty"
                                      << "Is Not Empty";

    m_numericOperators = QStringList() << "Equals" << "Not Equals"
                                       << "Greater Than" << "Less Than"
                                       << "Greater or Equal" << "Less or Equal"
                                       << "Between";

    setupUI();

    if (m_table && m_table->rowCount() > 0) {
        updateUniqueValues();
    }

    validateInputs();

    // Déclencher manuellement onColumnChanged si une colonne est sélectionnée
    if (m_columnCombo->currentIndex() >= 0) {
        // Forcer le signal car addItems() ne le déclenche pas
        QTimer::singleShot(0, this, [this]() {
            onColumnChanged(m_columnCombo->currentIndex());
        });
    }
}


void CustomFilterDialog::setupUI()
{
    setWindowTitle("Ajouter un filtre - Data Network");
    setMinimumWidth(500);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Groupe: Configuration du filtre ===
    QGroupBox *filterGroup = new QGroupBox("Configuration du filtre", this);
    QFormLayout *formLayout = new QFormLayout(filterGroup);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(20);
    m_nameEdit->setText("Filter");
    formLayout->addRow("Nom du nœud:", m_nameEdit);

    // Sélection de la colonne
    m_columnCombo = new QComboBox(this);
    if (m_table) {
        m_columnCombo->addItems(m_table->columnNames());
    }
    if (m_columnCombo->count() == 0) {
        m_columnCombo->addItem("Aucune colonne disponible");
        m_columnCombo->setEnabled(false);
    }
    formLayout->addRow("Colonne:", m_columnCombo);

    // Sélection de l'opérateur
    m_operatorCombo = new QComboBox(this);
    formLayout->addRow("Opérateur:", m_operatorCombo);

    // Valeur à filtrer
    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setPlaceholderText("Entrez la valeur à rechercher...");
    formLayout->addRow("Valeur:", m_valueEdit);

    mainLayout->addWidget(filterGroup);

    // === Suggestions de valeurs ===
    QGroupBox *suggestionsGroup = new QGroupBox("Valeurs existantes (suggestions)", this);
    QVBoxLayout *suggestionsLayout = new QVBoxLayout(suggestionsGroup);

    m_suggestionsView = new QListView(this);
    m_suggestionsView->setMaximumHeight(120);
    m_suggestionsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    suggestionsLayout->addWidget(m_suggestionsView);

    suggestionsGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    mainLayout->addWidget(suggestionsGroup);

    // === Statut ===
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("QLabel { color: gray; }");
    mainLayout->addWidget(m_statusLabel);

    // === Boutons ===
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_okButton = new QPushButton("Appliquer le filtre", this);
    QPushButton *cancelButton = new QPushButton("Annuler", this);

    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // === Connexions ===
    connect(m_columnCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CustomFilterDialog::onColumnChanged);
    connect(m_operatorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CustomFilterDialog::onOperatorChanged);
    connect(m_valueEdit, &QLineEdit::textChanged,
            this, &CustomFilterDialog::onValueChanged);
    connect(m_suggestionsView, &QListView::clicked,
            this, &CustomFilterDialog::onValueSelected);
    connect(m_okButton, &QPushButton::clicked,
            this, &CustomFilterDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked,
            this, &CustomFilterDialog::onReject);
}

void CustomFilterDialog::onColumnChanged(int index)
{
    Q_UNUSED(index)

    if (!m_table || m_columnCombo->currentIndex() < 0) {
        return;
    }

    m_column = m_columnCombo->currentText();
    int colIndex = m_table->columnIndex(m_column);

    // Utiliser le type DÉTECTÉ de la colonne
    ColumnType type = ColumnType::String;
    if (colIndex >= 0 && colIndex < m_table->columnTypes().size()) {
        type = m_table->columnTypes()[colIndex];
    }

    // Choisir les opérateurs selon le type RÉEL
    if (type == ColumnType::Integer || type == ColumnType::Double) {
        m_currentOperators = m_numericOperators;
        m_valueEdit->setPlaceholderText("Entrez un nombre...");
    } else {
        m_currentOperators = m_stringOperators;
        m_valueEdit->setPlaceholderText("Entrez le texte à rechercher...");
    }

    // Mettre à jour le combo des opérateurs
    QString previousOp = m_operatorCombo->currentText();
    m_operatorCombo->clear();
    m_operatorCombo->addItems(m_currentOperators);

    // Restaurer l'opérateur précédent si toujours valide
    int newIndex = m_operatorCombo->findText(previousOp);
    if (newIndex >= 0) {
        m_operatorCombo->setCurrentIndex(newIndex);
    } else {
        m_operatorCombo->setCurrentIndex(0);
    }

    // Déclencher manuellement onOperatorChanged après initialisation
    onOperatorChanged(m_operatorCombo->currentIndex());

    // Mettre à jour les valeurs uniques (converties selon le type)
    updateUniqueValues();

    validateInputs();
}

void CustomFilterDialog::onOperatorChanged(int index)
{
    Q_UNUSED(index)

    // Activer/désactiver le champ valeur selon l'opérateur
    QString currentOp = m_operatorCombo->currentText();
    bool needsValue = !(currentOp == "Is Empty" || currentOp == "Is Not Empty");

    m_valueEdit->setEnabled(needsValue);
    m_suggestionsView->setEnabled(needsValue);

    if (!needsValue) {
        m_valueEdit->clear();
    }

    validateInputs();
}

void CustomFilterDialog::onValueChanged(const QString& text)
{
    Q_UNUSED(text)
    validateInputs();
}

void CustomFilterDialog::updateUniqueValues()
{
    if (!m_table || m_columnCombo->currentIndex() < 0) {
        return;
    }

    m_column = m_columnCombo->currentText();
    int colIndex = m_table->columnIndex(m_column);

    if (colIndex < 0) {
        return;
    }

    ColumnType type = m_table->getColumnType(colIndex);
    QSet<QString> uniqueSet;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QVariant value = m_table->value(row, colIndex);

        if (!value.isNull()) {
            QString displayValue;

            // Formater selon le type
            if (type == ColumnType::Integer) {
                displayValue = QString::number(value.toInt());
            } else if (type == ColumnType::Double) {
                displayValue = QString::number(value.toDouble(), 'f', 2);
            } else {
                displayValue = value.toString();
            }

            if (!displayValue.isEmpty()) {
                uniqueSet.insert(displayValue);
            }
        }
    }

    m_uniqueValues = uniqueSet.values();

    // Trier numériquement si nécessaire
    if (type == ColumnType::Integer || type == ColumnType::Double) {
        std::sort(m_uniqueValues.begin(), m_uniqueValues.end(),
                  [](const QString& a, const QString& b) {
                      return a.toDouble() < b.toDouble();
                  });
    } else {
        m_uniqueValues.sort();
    }

    // Limiter à 50 valeurs
    if (m_uniqueValues.size() > 50) {
        m_uniqueValues = m_uniqueValues.mid(0, 50);
        m_uniqueValues.append("...");
    }

    // Afficher dans la liste
    QStringListModel *model = new QStringListModel(m_uniqueValues, this);
    m_suggestionsView->setModel(model);
}

void CustomFilterDialog::onValueSelected(const QModelIndex& index)
{
    if (index.isValid()) {
        QString value = m_uniqueValues[index.row()];
        if (value != "...") {
            m_valueEdit->setText(value);
        }
    }
}

void CustomFilterDialog::validateInputs()
{
    if (!m_table || m_columnCombo->currentIndex() < 0) {
        m_statusLabel->setText("❌ Aucune colonne disponible");
        m_okButton->setEnabled(false);
        m_isValid = false;
        return;
    }

    m_column = m_columnCombo->currentText();
    m_operator = m_operatorCombo->currentText();
    m_value = m_valueEdit->text();

    // Validation selon l'opérateur
    bool hasValue = true;
    QString operatorLower = m_operator.toLower();

    if (operatorLower == "is empty" || operatorLower == "is not empty") {
        hasValue = true;
        m_statusLabel->setText("✓ Filtre valide - vérifie les champs vides/non vides");
    }
    else if (operatorLower == "contains" ||
             operatorLower == "starts with" ||
             operatorLower == "ends with") {
        hasValue = !m_value.trimmed().isEmpty();
        if (!hasValue) {
            m_statusLabel->setText("⚠️ Veuillez entrer une valeur pour l'opérateur '" + m_operator + "'");
        } else {
            m_statusLabel->setText("✓ Filtre valide");
        }
    }
    else if (operatorLower == "between") {
        // Format spécial pour "between": valeur1,valeur2
        QStringList parts = m_value.split(",", Qt::SkipEmptyParts);
        hasValue = (parts.size() == 2);
        if (!hasValue) {
            m_statusLabel->setText("⚠️ Format requis : 'min,max' pour l'opérateur 'Between'");
        } else {
            m_statusLabel->setText("✓ Filtre valide");
        }
    }
    else {
        hasValue = !m_value.trimmed().isEmpty();
        if (!hasValue) {
            m_statusLabel->setText("⚠️ Veuillez entrer une valeur de comparaison");
        } else {
            m_statusLabel->setText("✓ Filtre valide");
        }
    }

    m_isValid = hasValue && m_columnCombo->currentIndex() >= 0;
    m_okButton->setEnabled(m_isValid);
}

void CustomFilterDialog::onAccept()
{
    if (m_isValid) {
        accept();
    } else {
        QMessageBox::warning(this, "Validation",
                               "Veuillez remplir correctement tous les champs.");
    }
}

void CustomFilterDialog::onReject()
{
    reject();
}

// Méthode statique simplifiée
bool CustomFilterDialog::getFilterParameters(QWidget *parent,
                                             const dn::core::DataTable* table,
                                             QString& column,
                                             QString& op,
                                             QString& value,
                                             QString* nodeName)
{
    if (!table || table->rowCount() == 0) {
        QMessageBox::warning(parent, "Erreur",
                             "Aucune donnée disponible pour créer un filtre.");
        return false;
    }

    CustomFilterDialog dialog(table, parent);

    if (dialog.exec() == QDialog::Accepted) {
        column = dialog.getColumn();
        op = dialog.getOperator();
        value = dialog.getValue();
        if (nodeName) {
            *nodeName = dialog.m_nameEdit->text().trimmed();
        }
        return true;
    }

    return false;
}

FilterTransformation::Operator CustomFilterDialog::convertOperatorString(const QString& opStr)
{
    if (opStr == "Equals") return FilterTransformation::Equals;
    if (opStr == "Not Equals") return FilterTransformation::NotEquals;
    if (opStr == "Greater Than") return FilterTransformation::GreaterThan;
    if (opStr == "Less Than") return FilterTransformation::LessThan;
    if (opStr == "Greater or Equal") return FilterTransformation::GreaterThan; // À ajouter
    if (opStr == "Less or Equal") return FilterTransformation::LessThan;      // À ajouter
    if (opStr == "Contains") return FilterTransformation::Contains;
    if (opStr == "Starts With") return FilterTransformation::Contains;
    if (opStr == "Ends With") return FilterTransformation::Contains;
    if (opStr == "Is Empty") return FilterTransformation::Equals;
    if (opStr == "Is Not Empty") return FilterTransformation::NotEquals;
    if (opStr == "Between") return FilterTransformation::GreaterThan; // Temporaire

    return FilterTransformation::Equals;
}
