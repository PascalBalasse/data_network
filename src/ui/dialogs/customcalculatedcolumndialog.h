/**
 * @file customcalculatedcolumndialog.h
 * @brief Dialogue pour créer une colonne calculée
 *
 * Permet de créer une nouvelle colonne basée sur une expression
 * utilisant les colonnes existantes.
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMCALCULATEDCOLUMNDIALOG_H
#define CUSTOMCALCULATEDCOLUMNDIALOG_H

#include "../../network/core/DataTable.h"
#include "../../ui/models/DataTableModel.h"
#include <QDialog>
#include <QStringList>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

namespace dn::dialogs{

    class CustomCalculatedColumnDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomCalculatedColumnDialog(const dn::core::DataTable* table,
                                              QWidget *parent = nullptr);

        /// Retourne les détails de la colonne calculée à créer
        QString getColumnName() const { return m_columnNameEdit->text(); }
        QString getExpression() const { return m_expressionEdit->toPlainText(); }
        dn::core::ColumnType getColumnType() const { return dn::core::ColumnType::Any; }

        /// Méthode statique simplifiée
        static bool getCalculatedColumnDetails(QWidget *parent,
                                               const dn::core::DataTable* table,
                                               QString& columnName,
                                               QString& expression,
                                               dn::core::ColumnType& columnType);

    private slots:
        void onAccept();
        void onReject();
        void onColumnDoubleClicked(const QListWidgetItem* item);
        void onPreviewExpression();

    private:
        void setupUI();
        void updateColumnList();
        void insertColumnAtCursor(const QString& columnName);
        QString buildPreviewExpression() const;

        //═══════════════════════════════════════════════════════════════════
        // Widgets
        //══════════════════════════════════════════════════════════════════
        QLineEdit *m_columnNameEdit;
        QTextEdit *m_expressionEdit;
        QListWidget *m_columnList;
        QLabel *m_previewLabel;
        QPushButton *m_okButton;
        QPushButton *m_previewButton;

        //═══════════════════════════════════════════════════════════════════
        // Données
        //══════════════════════════════════════════════════════════════════
        const dn::core::DataTable* m_table;
        QStringList m_columnNames;
    };

}

#endif // CUSTOMCALCULATEDCOLUMNDIALOG_H