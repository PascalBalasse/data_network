/**
 * @file customcalculatedcolumndialog.cpp
 * @brief Implementation du dialogue pour créer une colonne calculée
 */

#include "customcalculatedcolumndialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

namespace dn::dialogs{

    CustomCalculatedColumnDialog::CustomCalculatedColumnDialog(const dn::core::DataTable* table,
                                                               QWidget *parent)
        : QDialog(parent), m_table(table)
    {
        setupUI();
        updateColumnList();
        
        setWindowTitle("Créer une colonne calculée");
        resize(500, 400);
    }

    void CustomCalculatedColumnDialog::setupUI()
    {
        // Layout principal
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Nom de la colonne
        QFormLayout* nameLayout = new QFormLayout();
        m_columnNameEdit = new QLineEdit();
        m_columnNameEdit->setPlaceholderText("Nom de la nouvelle colonne");
        nameLayout->addRow("Nom de la colonne :", m_columnNameEdit);
        mainLayout->addLayout(nameLayout);
        
        // Expression
        mainLayout->addWidget(new QLabel("Expression :"));
        m_expressionEdit = new QTextEdit();
        m_expressionEdit->setPlaceholderText("Exemple: [Colonne1] + [Colonne2] * 2");
        mainLayout->addWidget(m_expressionEdit);
        
        // Boutons pour l'expression
        QHBoxLayout* exprButtonsLayout = new QHBoxLayout();
        m_previewButton = new QPushButton("Aperçu");
        connect(m_previewButton, &QPushButton::clicked, this, &CustomCalculatedColumnDialog::onPreviewExpression);
        exprButtonsLayout->addWidget(m_previewButton);
        exprButtonsLayout->addStretch();
        mainLayout->addLayout(exprButtonsLayout);
        
        // Aperçu
        mainLayout->addWidget(new QLabel("Aperçu de l'expression :"));
        m_previewLabel = new QLabel("");
        m_previewLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; }");
        m_previewLabel->setWordWrap(true);
        mainLayout->addWidget(m_previewLabel);
        
        // Liste des colonnes disponibles
        mainLayout->addWidget(new QLabel("Colonnes disponibles (double-cliquez pour insérer) :"));
        m_columnList = new QListWidget();
        m_columnList->setMaximumHeight(100);
        connect(m_columnList, &QListWidget::itemDoubleClicked, 
                this, &CustomCalculatedColumnDialog::onColumnDoubleClicked);
        mainLayout->addWidget(m_columnList);
        
        // Boutons OK/Annuler
        QHBoxLayout* buttonsLayout = new QHBoxLayout();
        buttonsLayout->addStretch();
        m_okButton = new QPushButton("OK");
        QPushButton* cancelButton = new QPushButton("Annuler");
        buttonsLayout->addWidget(m_okButton);
        buttonsLayout->addWidget(cancelButton);
        mainLayout->addLayout(buttonsLayout);
        
        // Connexions
        connect(m_okButton, &QPushButton::clicked, this, &CustomCalculatedColumnDialog::onAccept);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        connect(m_expressionEdit, &QTextEdit::textChanged, this, &CustomCalculatedColumnDialog::onPreviewExpression);
    }

    void CustomCalculatedColumnDialog::updateColumnList()
    {
        if (!m_table) return;
        
        m_columnNames = m_table->columnNames();
        m_columnList->clear();
        m_columnList->addItems(m_columnNames);
    }

    void CustomCalculatedColumnDialog::onColumnDoubleClicked(const QListWidgetItem* item)
    {
        if (item) {
            QString columnName = item->text();
            insertColumnAtCursor(columnName);
        }
    }

    void CustomCalculatedColumnDialog::insertColumnAtCursor(const QString& columnName)
    {
        QString formattedName = QString("[%1]").arg(columnName);
        QTextCursor cursor = m_expressionEdit->textCursor();
        cursor.insertText(formattedName);
        m_expressionEdit->setFocus();
    }

    void CustomCalculatedColumnDialog::onPreviewExpression()
    {
        QString expression = m_expressionEdit->toPlainText();
        if (expression.isEmpty()) {
            m_previewLabel->setText("");
            return;
        }
        
        // Remplacer les références de colonnes par des exemples pour l'aperçu
        QString previewExpr = expression;
        for (const QString& colName : m_columnNames) {
            previewExpr.replace(QString("[%1]").arg(colName), 
                               QString("[exemple_%1]").arg(colName));
        }
        
        m_previewLabel->setText(previewExpr);
    }

    void CustomCalculatedColumnDialog::onAccept()
    {
        if (m_columnNameEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir un nom pour la colonne.");
            return;
        }
        
        if (m_expressionEdit->toPlainText().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir une expression.");
            return;
        }
        
        accept();
    }

    void CustomCalculatedColumnDialog::onReject()
    {
        reject();
    }

    bool CustomCalculatedColumnDialog::getCalculatedColumnDetails(QWidget *parent,
                                                                   const dn::core::DataTable* table,
                                                                   QString& columnName,
                                                                   QString& expression,
                                                                   dn::core::ColumnType& columnType)
    {
        CustomCalculatedColumnDialog dialog(table, parent);
        if (dialog.exec() == QDialog::Accepted) {
            columnName = dialog.getColumnName();
            expression = dialog.getExpression();
            columnType = dialog.getColumnType();
            return true;
        }
        return false;
    }

} // namespace dn::dialogs