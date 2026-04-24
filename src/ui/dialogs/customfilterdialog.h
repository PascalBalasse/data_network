/**
 * @file customfilterdialog.h
 * @brief Dialogue de configuration du filtre
 *
 * Permet de configurer un filtre sur les données.
 * Interface avec:
 * - Sélection de colonne
 * - Sélection d'opérateur
 * - Saisie de valeur
 * - Aperçu du nombre de lignes correspondantes
 * - Autocomplétion avec les valeurs uniques
 *
 * Namespace: dn::dialogs
 */

#ifndef CUSTOMFILTERDIALOG_H
#define CUSTOMFILTERDIALOG_H

#include <QDialog>
#include <QListView>
#include <QLabel>
#include <QComboBox>
#include <QMap>
#include <QVariant>
#include <QStringList>
#include "../../network/core/DataTable.h"
#include "../../network/transformations/FilterTransformation.h"

using namespace dn::transformations;

namespace dn::dialogs{
    class CustomFilterDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CustomFilterDialog(const dn::core::DataTable* table,
                                    QWidget *parent = nullptr);

        /// Retourne la colonne sélectionnée
        QString getColumn() const { return m_column; }

        /// Retourne l'opérateur
        QString getOperator() const { return m_operator; }

        /// Retourne la valeur de filtre
        QString getValue() const { return m_value; }

        /// Retourne true si les paramètres sont valides
        bool isValid() const { return m_isValid; }

        /// Convertit une chaîne en opérateur
        static FilterTransformation::Operator convertOperatorString(const QString& opStr);

        /// Méthode statique simplifiée
        static bool getFilterParameters(QWidget *parent,
                                        const dn::core::DataTable* table,
                                        QString& column,
                                        QString& op,
                                        QString& value,
                                        QString* nodeName = nullptr);

    private slots:
        void onColumnChanged(int index);
        void onOperatorChanged(int index);
        void onValueChanged(const QString& text);
        void onValueSelected(const QModelIndex& index);
        void onPreviewRequested();
        void onAccept();
        void onReject();

    private:
        void setupUI();
        void updatePreview();
        void validateInputs();
        void updateUniqueValues();
        void setupAutoComplete();

        //══════════════════════════════════════════════════════════════════
        // Widgets
        //══════════════════════════════════════════════════════════════════
        QComboBox *m_columnCombo;
        QComboBox *m_operatorCombo;
        QLineEdit *m_nameEdit;
        QLineEdit *m_valueEdit;
        QListView *m_suggestionsView;
        QLabel *m_previewLabel;
        QLabel *m_statsLabel;
        QLabel *m_statusLabel;
        QPushButton *m_okButton;
        QPushButton *m_previewButton;

        //══════════════════════════════════════════════════════════════════
        // Données
        //══════════════════════════════════════════════════════════════════
        const dn::core::DataTable* m_table;
        QString m_column;
        QString m_operator;
        QString m_value;
        QString m_nodeName;
        bool m_isValid;
        int m_matchingRows;

        //══════════════════════════════════════════════════════════════════
        // Opérateurs par type
        //══════════════════════════════════════════════════════════════════
        QStringList m_stringOperators;
        QStringList m_numericOperators;
        QStringList m_currentOperators;

        //══════════════════════════════════════════════════════════════════
        // Autocomplétion
        //══════════════════════════════════════════════════════════════════
        QStringList m_uniqueValues;
    };
}


#endif // CUSTOMFILTERDIALOG_H
