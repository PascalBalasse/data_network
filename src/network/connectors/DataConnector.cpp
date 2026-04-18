/**
 * @file DataConnector.cpp
 * @brief Implémentation de la classe DataConnector
 *
 * Ce fichier est vide car DataConnector est une classe abstraite.
 * Toutes les méthodes sont soit:
 * - virtuelles pures (load(), configure())
 * - implémentées par défaut (write(), supportsWriting(), supportsReading())
 *
 * L'implémentation concrète se trouve dans les sous-classes:
 * - CSVConnector, ExcelConnector, JSONConnector, etc.
 *
 * Si des fonctionnalités communes doivent être ajoutées à tous les connecteurs,
 * c'est ici qu'elles seraient implémentées (ex: gestion d'erreur, logging).
 */