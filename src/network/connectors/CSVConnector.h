/**
 * @file CSVConnector.h
 * @brief Connecteur pour les fichiers CSV (Comma-Separated Values)
 *
 * Ce connecteur permet de lire et écrire des fichiers au format CSV.
 * Il supporte:
 * - Personnalisation du séparateur (virgule, point-virgule, tabulation, etc.)
 * - Détection automatique des en-têtes de colonnes
 * - Échappement des caractères spéciaux (guillemets, séparateurs)
 * - Détection automatique des types de données (entiers, dates, etc.)
 *
 * Paramètres de configuration:
 * - "fileName": Chemin du fichier CSV (obligatoire)
 * - "separator": Caractère de séparation (par défaut: ",")
 * - "hasHeader":true si la première ligne contient les noms de colonnes
 *
 * Point tricky: L'échappement des guillemets utilise la règle CSV standard:
 * les guillemets dans les valeurs sont doublés (" -> "").
 *
 * Namespace: dn::connectors
 */

#ifndef CSVCONNECTOR_H
#define CSVCONNECTOR_H

#include "DataConnector.h"
#include <QChar>
#include <QString>

namespace dn::connectors {

    /**
     * @class CSVConnector
     * @brief Connecteur pour fichiers CSV/TXT délimités
     *
     * Implémente les méthodes de lecture et écriture pour les fichiers.
     * Utilise QTextStream pour le parsing ligne par ligne.
     * Peut gérer n'importe quel séparateurvia le paramètre "separator".
     *
     * Exemple d'utilisation:
     * @code
     * auto connector = std::make_unique<CSVConnector>();
     * connector->configure({{"fileName", "data.csv"}, {"separator", ";"}, {"hasHeader", true}});
     * auto table = connector->load();
     * connector->write(*table);
     * @endcode
     *
     * Pour TXT (tabulation):
     * @code
     * connector->configure({{"fileName", "data.txt"}, {"separator", "\t"}, {"hasHeader", true}});
     * @endcode
     */
    class CSVConnector : public DataConnector
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructeur
         * @param parent Parent Qt pour la gestion de mémoire
         */
        explicit CSVConnector(QObject *parent = nullptr);

        /**
         * @brief Lit un fichier CSV et retourne un DataTable
         * @return Pointeur unique vers le DataTable chargé
         *
         * Étapes:
         * 1. Ouverture du fichier en lecture
         * 2. Lecture de la première ligne (en-têtes ou données)
         * 3. Lecture des lignes suivantes
         * 4. Détection automatique des types de colonnes
         */
        std::unique_ptr<dn::core::DataTable> load() override;

        /**
         * @brief Écrit un DataTable dans un fichier CSV
         * @param data Le DataTable à écrire
         * @return true si succès, false sinon
         *
         * Écrit d'abord les noms de colonnes (si hasHeader=true),
         * puis chaque ligne de données.
         * Point tricky: Échappe correctement les valeurs contenant
         * le séparateur ou des guillemets.
         */
        bool write(const dn::core::DataTable& data) override;

        /**
         * @brief Configure le connecteur CSV
         * @param params Paramètres de configuration
         *
         * Params acceptés:
         * - "fileName": QString - Chemin du fichier
         * - "separator": QString - Séparateur (1 caractère)
         * - "hasHeader": bool - Première ligne = en-têtes
         */
        void configure(const QMap<QString, QVariant>& params) override;

        /**
         * @brief Indique que ce connecteur supporte l'écriture
         */
        bool supportsWriting() const override { return true; }

        /**
         * @brief Indique que ce connecteur supporte la lecture
         */
        bool supportsReading() const override { return true; }

        // ========== Accesseurs pour les tests ==========

        /// @brief Retourne le nom du fichier configuré
        QString getFileName() const { return m_fileName; }

        /// @brief Retourne le caractère séparateur
        QChar getSeparator() const { return m_separator; }

        /// @brief Retourne true si les en-têtes sont attendus
        bool hasHeader() const { return m_hasHeader; }

    private:
        QString m_fileName;          ///< Chemin du fichier CSV
        QChar m_separator = ',';     ///< Séparateur de champs (par défaut: virgule)
        bool m_hasHeader = true;     ///< Indique si la première ligne contient les en-têtes
    };

}
#endif // CSVCONNECTOR_H
