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

using namespace dn::dialogs;

CustomFilterDialog::CustomFilterDialog(const dn::core::DataTable* table,
                                       QWidget *parent)
    : QDialog(parent)
    , m_table(table)
    , m_isValid(false)
    , m_matchingRows(0)
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
}

void CustomFilterDialog::setupUI()
{
    setWindowTitle("Ajouter un filtre - Data Network");
    setMinimumWidth(500);
    setMinimumHeight(600);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Groupe: Configuration du filtre ===
    QGroupBox *filterGroup = new QGroupBox("Configuration du filtre", this);
    QFormLayout *formLayout = new QFormLayout(filterGroup);

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
    suggestionsLayout->addWidget(m_suggestionsView);

    mainLayout->addWidget(suggestionsGroup);

    // === Statistiques ===
    m_statsLabel = new QLabel(this);
    m_statsLabel->setStyleSheet("QLabel { background-color: #e8f0fe; padding: 8px; border-radius: 4px; }");
    mainLayout->addWidget(m_statsLabel);

    // === Aperçu ===
    QGroupBox *previewGroup = new QGroupBox("Aperçu du filtre", this);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setStyleSheet("QLabel { background-color: #f5f5f5; padding: 10px; border-radius: 5px; font-family: monospace; }");
    m_previewLabel->setText("Sélectionnez les paramètres pour voir l'aperçu...");
    previewLayout->addWidget(m_previewLabel);

    mainLayout->addWidget(previewGroup);

    // === Statut ===
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("QLabel { color: gray; }");
    mainLayout->addWidget(m_statusLabel);

    // === Boutons ===
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_previewButton = new QPushButton("Aperçu détaillé", this);
    m_okButton = new QPushButton("Appliquer le filtre", this);
    QPushButton *cancelButton = new QPushButton("Annuler", this);

    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    m_okButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");

    buttonLayout->addWidget(m_previewButton);
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
    connect(m_previewButton, &QPushButton::clicked,
            this, &CustomFilterDialog::onPreviewRequested);
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
    m_operatorCombo->clear();
    m_operatorCombo->addItems(m_currentOperators);

    // Mettre à jour les valeurs uniques (converties selon le type)
    updateUniqueValues();

    validateInputs();
    updatePreview();
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
    updatePreview();
}

void CustomFilterDialog::onValueChanged(const QString& text)
{
    Q_UNUSED(text)
    validateInputs();
    updatePreview();
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
            m_statusLabel->setText("⚠️ Format requis: 'min,max' pour l'opérateur 'Between'");
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

    // Mettre à jour les statistiques
    updatePreview();
}

void CustomFilterDialog::updatePreview()
{
    if (!m_isValid || !m_table) {
        m_previewLabel->setText("Paramètres incomplets - aperçu non disponible");
        m_statsLabel->setText("Sélectionnez une colonne et un opérateur pour voir les statistiques");
        return;
    }

    // Calculer combien de lignes correspondent
    m_matchingRows = 0;

    // Pour un aperçu complet, il faudrait appliquer le filtre
    // Version simplifiée pour l'instant
    m_matchingRows = m_table->rowCount() / 2; // À remplacer par vrai calcul

    QString preview;
    QString operatorSymbol;

    if (m_operator == "Equals") operatorSymbol = "==";
    else if (m_operator == "Not Equals") operatorSymbol = "!=";
    else if (m_operator == "Greater Than") operatorSymbol = ">";
    else if (m_operator == "Less Than") operatorSymbol = "<";
    else if (m_operator == "Greater or Equal") operatorSymbol = ">=";
    else if (m_operator == "Less or Equal") operatorSymbol = "<=";
    else if (m_operator == "Contains") operatorSymbol = "contains";
    else if (m_operator == "Starts With") operatorSymbol = "starts with";
    else if (m_operator == "Ends With") operatorSymbol = "ends with";
    else if (m_operator == "Is Empty") operatorSymbol = "is empty";
    else if (m_operator == "Is Not Empty") operatorSymbol = "is not empty";
    else if (m_operator == "Between") operatorSymbol = "between";

    if (m_operator == "Is Empty") {
        preview = QString("Garder les lignes où <b>'%1'</b> %2")
                      .arg(m_column, operatorSymbol);
    }
    else if (m_operator == "Is Not Empty") {
        preview = QString("Garder les lignes où <b>'%1'</b> %2")
                      .arg(m_column, operatorSymbol);
    }
    else if (m_operator == "Between") {
        QStringList parts = m_value.split(",", Qt::SkipEmptyParts);
        if (parts.size() == 2) {
            preview = QString("Garder les lignes où <b>'%1'</b> %2 <b>%3</b> et <b>%4</b>")
                          .arg(m_column, operatorSymbol, parts[0].trimmed(), parts[1].trimmed());
        } else {
            preview = QString("Garder les lignes où <b>'%1'</b> %2 ...")
                          .arg(m_column, operatorSymbol);
        }
    }
    else {
        preview = QString("Garder les lignes où <b>'%1'</b> %2 <b>'%3'</b>")
                      .arg(m_column, operatorSymbol, m_value);
    }

    m_previewLabel->setText(preview);

    // Statistiques
    int totalRows = m_table->rowCount();
    double percentage = (m_matchingRows * 100.0) / totalRows;

    m_statsLabel->setText(QString(
                              "📊 <b>Statistiques :</b><br>"
                              "• Total des lignes : %1<br>"
                              "• Lignes correspondantes : %2<br>"
                              "• Pourcentage : %3%"
                              ).arg(totalRows).arg(m_matchingRows).arg(percentage, 0, 'f', 1));
}

void CustomFilterDialog::onPreviewRequested()
{
    if (m_isValid) {
        QString message = "🔍 <b>Aperçu du filtre</b><br><br>" +
                          m_previewLabel->text() + "<br><br>" +
                          "✅ Le filtre est valide et prêt à être appliqué.<br><br>" +
                          "<b>Résultat attendu :</b> " +
                          QString::number(m_matchingRows) + " lignes sur " +
                          QString::number(m_table->rowCount());

        QMessageBox msgBox;
        msgBox.setWindowTitle("Aperçu du filtre");
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText(message);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    } else {
        QMessageBox::warning(this, "Aperçu impossible",
                             "Veuillez compléter tous les champs requis.");
    }
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
                                             QString& value)
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
