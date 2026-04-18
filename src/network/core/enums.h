/**
 * @file enums.h
 * @brief Définitions des énumérations utilisées dans le réseau de données
 *
 * Ce fichier contient tous les types énumérés (enum class) utilisés pour :
 * - Typologie des nœuds dans le graphe
 * - Types de connecteurs (formats de données)
 * - Types de transformations
 * - Types de jointures/fusions
 * - Types de colonnes (simples et complexes)
 *
 * Namespace: dn::core
 */

#ifndef ENUMS_H
#define ENUMS_H

namespace dn::core{

    /**
     * @enum NodeType
     * @brief Type de nœud dans le graphe de données
     *
     * Chaque nœud du réseau a un rôle spécifique :
     * - Source: Point d'entrée des données (lecture depuis un fichier/BDD)
     * - Transform: Transformation des données (filtre, sélection, etc.)
     * - Merge: Fusion de plusieurs sources de données
     * - Target: Point de sortie des données (écriture vers un fichier/BDD)
     */
    enum class NodeType {
        Source,    ///< Nœud source - importe les données depuis un connecteur
        Transform, ///< Nœud de transformation - applique une transformation
        Merge,     ///< Nœud de fusion - combine plusieurs entrées
        Target     ///< Nœud cible - exporte les données via un connecteur
    };


    /**
     * @enum ConnectorType
     * @brief Type de connecteur pour l'import/export de données
     *
     * Définit les formats supportés pour la lecture et l'écriture des données.
     * Chaque connecteur implémente l'interface DataConnector pour un format spécifique.
     *
     * Point tricky: Certains formats (XML, FEC, SQL) nécessitent des bibliothèques
     * externes ou des configurations spécifiques.
     */
enum class ConnectorType{
        CSV,       ///< Format CSV (valeurs séparées par des virgules)
        TXT,       ///< Format texte délimité (personnalisable)
        EXCEL,     ///< Format Excel (.xlsx) - nécessite lecture XLSX
        PDF,       ///< Format PDF (lecture de tableaux)
        JSON,      ///< Format JSON (supporte types complexes)
        XML,       ///< Format XML
        WEB,       ///< Page web (extraction de tableaux HTML)
        FOLDER,    ///< Dossier contenant plusieurs fichiers
        DATABASE,  ///< Base de données générique
        FEC,       ///< Fichier FEC (Fichier des Écritures Comptables)
        SQL        ///< Requête SQL vers une base de données
    };


    /**
     * @enum ConnectorMode
     * @brief Mode d'utilisation d'un connecteur
     *
     * Utilisé pour configurer si le connecteur doit lire ou écrire les données.
     * Note: Certains connecteurs peuvent supporter les deux modes.
     */
    enum class ConnectorMode {
        Read,   ///< Mode lecture - import des données depuis une source
        Write   ///< Mode écriture - export des données vers une cible
    };

    /**
     * @enum TransformationType
     * @brief Type de transformation applicable aux données
     *
     * Définit les transformations disponibles qui peuvent être appliquées
     * aux données dans un nœud de transformation.
     */
    enum class TransformationType{
        Filter,          ///< Filtrage des lignes selon une condition
        SelectColumns   ///< Sélection d'un sous-ensemble de colonnes
    };

    /**
     * @enum MergeType
     * @brief Type de fusion de plusieurs tables de données
     *
     * Définit les différents modes de combinaison de plusieurs sources de données.
     * Point tricky: Les comportement varient significativement selon le type.
     *
     * - Union: Combine toutes les lignes (comme SQL UNION)
     * - Join: Jointure sur des clés (comme SQL JOIN)
     * - Intersection: Garder uniquement les lignes communes
     * - Concatenate: Concaténation horizontale (ajout de colonnes)
     */
    enum class MergeType {
        Union,         ///< Union: toutes les lignes des tables combinées
        Join,          ///< Jointure: fusion sur clés communes (INNER JOIN)
        Intersection,  ///< Intersection: lignes présentes dans TOUTES les tables
        Concatenate    ///< Concatenation: ajout de colonnes côte à côte
    };

    /**
     * @enum ColumnType
     * @brief Type de données d'une colonne
     *
     * Types simples: String, Integer, Double, Boolean, Date, DateTime
     * Types complexes: List, Pair, Table (pour données hiérarchiques JSON/XML)
     *
     * Point tricky: La détection automatique des types est importantes pour
     * le traitement correct des données. Les types complexes nécessitent une
     * gestion spéciale dans les connecteurs et transformations.
     */
    enum class ColumnType {
        String,     ///< Texte (QString)
        Integer,    ///< Nombre entier (int, long)
        Double,     ///< Nombre décimal (double)
        Boolean,    ///< Booléen (true/false)
        Date,       ///< Date seule (QDate)
        DateTime,   ///< Date et heure (QDateTime)
        List,       ///< Liste ordonnée (tableau JSON, éléments XML)
        Pair,       ///< Paire clé-valeur (objet JSON, élément XML avec attributs)
        Table,      ///< Table imbriquée (tableau d'objets)
        Any         ///< Type mixte (plusieurs types dans la colonne)
    };
}

#endif // ENUMS_H
