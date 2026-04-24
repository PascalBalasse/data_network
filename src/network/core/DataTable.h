/**
 * @file DataTable.h
 * @brief Représentation tabulaire des données dans le réseau
 *
 * Cette classe encapsulate un tableau de données avec :
 * - Des lignes (enregistrements)
 * - Des colonnes (champs) avec noms et types
 * - Détection automatique des types de données
 * - Conversion de types (String -> Integer, Date, etc.)
 *
 * Fait le lien avec Qt via QAbstractTableModel pour l'affichage dans les vues.
 *
 * Namespace: dn::core
 */

#ifndef DATATABLE_H
#define DATATABLE_H

#include "enums.h"
#include <QAbstractTableModel>
#include <QList>
#include <QVariant>
#include <QHash>

namespace dn::core {

    /**
     * @brief Classe principale représentant un tableau de données
     *
     * DataTable stocke les données sous forme de liste de lignes.
     * Chaque ligne est une liste de QVariant (permettant tout type).
     * Les colonnes sont caractérisées par un nom et un type détecté automatiquement.
     *
     * Usage typique :
     * @code
     * DataTable table;
     * table.setColumnNames({"Nom", "Age", "Date"});
     * table.addRow({"Dupont", 30, "1993-05-15"});
     * table.autoDetectAndConvertTypes();
     * @endcode
     */
    class DataTable
    {
    public:
        DataTable() = default;
        DataTable& operator=(const DataTable&) = default;

        //══════════════════════════════════════════════════════════════════════
        // Structure du tableau
        //═════════════════════════════════════════════════════════════════════

        /**
         * @brief Retourne le nombre de lignes (enregistrements) dans le tableau
         * @return Nombre de lignes
         */
        int rowCount() const;

        /**
         * @brief Retourne le nombre de colonnes dans le tableau
         * @return Nombre de colonnes
         */
        int columnCount() const;

        //═════════════════════════════════════════════════════════════════════
        // Accès aux données
        //════════════════════════════════════════════════════════════════════

        /**
         * @brief Récupère la valeur à une position row, col
         * @param row Index de ligne (0-based)
         * @param col Index de colonne (0-based)
         * @return Valeur QVariant (peut être vide si invalide)
         */
        QVariant value(int row, int col) const;

        /**
         * @brief Retourne le nom d'une colonne par son index
         * @param col Index de colonne
         * @return Nom de la colonne ou chaîne vide si invalide
         */
        QString columnName(int col) const;

        /**
         * @brief Retourne l'index d'une colonne par son nom
         * @param name Nom de la colonne
         * @return Index ou -1 si non trouvé
         */
        int columnIndex(const QString& name) const;

        /**
         * @brief Retourne la référence constante aux données brutes (liste de lignes)
         * @return Liste de listes de QVariant
         */
        const QList<QList<QVariant>>& rows() const { return m_data; }

        /**
         * @brief Retourne la référence constante aux noms de colonnes
         * @return Liste de chaînes
         */
        const QStringList& columnNames() const { return m_columnNames; }

        /**
         * @brief Retourne la référence constante aux types de colonnes
         * @return Liste de ColumnType
         */
        const QList<ColumnType>& columnTypes() const { return m_columnTypes; }

        //═════════════════════════════════════════════════════════════════════
        // Gestion des types
        //═══════════════════════════════════════════════════════════════════

        /**
         * @brief Retourne le type détecté d'une colonne
         * @param colIndex Index de colonne
         * @return ColumnType (String par défaut si invalide)
         */
        ColumnType getColumnType(int colIndex) const;

        /**
         * @brief Détecte automatiquement les types de toutes les colonnes
         *
         * Parcourt toutes les valeurs et détermine le type le plus approprié
         * pour chaque colonne (Integer, Double, Boolean, Date, DateTime, String).
         * Convertit ensuite les valeurs vers le type détecté.
         */
        void autoDetectAndConvertTypes();

        /**
         * @brief Définit le type d'une colonne sans conversion des valeurs
         * @param colIndex Index de colonne
         * @param type Nouveau type à utiliser
         */
        void setColumnType(int colIndex, ColumnType type);

        /**
         * @brief Force un type de colonne avec option de conversion
         * @param colIndex Index de colonne
         * @param type Type cible
         * @param convertValues true (par défaut) pour convertir les valeurs existantes
         */
        void forceColumnType(int colIndex, ColumnType type, bool convertValues = true);

        //════════════════════════════════════════════════════════════════════
        // Types complexes (List, Pair, Table)
        //══════════════════════════════════════════════════════════════════

        /**
         * @brief Vérifie si un type est complexe (List, Pair ou Table)
         * @param type Type à tester
         * @return true si complexe
         */
        bool isComplexType(ColumnType type) const;

        /**
         * @brief Teste si une valeur est une liste (QVariantList)
         * @param value Valeur à tester
         * @return true si liste
         */
        static bool isListValue(const QVariant& value);

        /**
         * @brief Teste si une valeur est une paire clé-valeur (QVariantMap)
         * @param value Valeur à tester
         * @return true si map/paire
         */
        static bool isPairValue(const QVariant& value);

        /**
         * @brief Teste si une valeur est une table imbriquée
         * @param value Valeur à tester
         * @return true si DataTable imbriquée
         */
        static bool isTableValue(const QVariant& value);

        //═══════════════════════════════════════════════════════════════════
        // Colonnes calculées
        //═════════════════════════════════════════════════════════════════

        /**
         * @brief Ajoute une colonne calculée basée sur une expression
         * @param columnName Nom de la nouvelle colonne
         * @param expression Expression utilisant les références de colonnes comme [NomColonne]
         * @param type Type de données de la colonne résultante
         * @return true si succès, false si erreur dans l'expression
         */
        bool addCalculatedColumn(const QString& columnName,
                                 const QString& expression,
                                 ColumnType type);

        /**
         * @brief Évalue une expression simple (pour les colonnes calculées)
         * @param expression Expression à évaluer
         * @return Résultat de l'évaluation
         */
        QVariant evaluateExpression(const QString& expression) const;

        //═══════════════════════════════════════════════════════════════════
        // Modification du contenu
        //═════════════════════════════════════════════════════════════════

        /**
         * @brief Définit les noms de colonnes (et initialise les types à String)
         * @param names Liste de noms
         */
        void setColumnNames(const QStringList &names);

        /**
         * @brief Définit les types de colonnes manuellement
         * @param types Liste de types
         */
        void setColumnTypes(const QList<ColumnType> &types);

        /**
         * @brief Ajoute une ligne au tableau (copie)
         * @param row Ligne à ajouter (liste de valeurs)
         */
        void addRow(const QList<QVariant> &row);

        /**
         * @brief Ajoute une ligne au tableau (déplacement)
         * @param row Ligne à ajouter (move semantics)
         */
        void addRow(QList<QVariant>&& row);

        /**
         * @brief Efface toutes les données (lignes, colonnes, types)
         */
        void clear();

    private:
        /// Données : liste de lignes, chaque ligne étant une liste de valeurs
        QList<QList<QVariant>> m_data;

        /// Noms des colonnes (dans l'ordre)
        QStringList m_columnNames;

        /// Types de chaque colonne (correspondance index-to-type)
        QList<ColumnType> m_columnTypes;

        //═══════════════════════════════════════════════════════════════════
        // Détails d'implémentation privés
        //══════════════════════════════════════════════════════════════════

        /**
         * @brief Détecte le type d'une colonne en analysant ses valeurs
         * @param colIndex Index de colonne
         * @return Type détecté (String par défaut si indétectable)
         */
        ColumnType detectColumnType(int colIndex) const;

        /**
         * @brief Convertit une valeur vers un type cible
         * @param value Valeur source
         * @param targetType Type cible
         * @return Valeur convertie ou QVariant() si échec
         */
        QVariant convertToType(const QVariant& value, ColumnType targetType) const;

        /**
         * @brief Compte les valeurs vides dans une colonne
         * @param colIndex Index de colonne
         * @return Nombre de cellules vides ou trimées vides
         */
        int countEmptyRows(int colIndex) const;

        /**
         * @brief Convertit toutes les valeurs d'une colonne vers un type
         * @param colIndex Index de colonne
         * @param newType Type cible pour la conversion
         */
        void convertColumnValues(int colIndex, ColumnType newType);
    };

}

#endif // DATATABLE_H