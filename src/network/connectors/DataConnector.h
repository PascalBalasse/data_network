/**
 * @file DataConnector.h
 * @brief Interface abstraite pour les connecteurs de données
 *
 * Ce fichier définit la classe de base abstraite pour tous les connecteurs.
 * Un connecteur est responsable de l'import (lecture) et/ou export (écriture)
 * de données depuis/vers un format spécifique (CSV, Excel, JSON, SQL, etc.).
 *
 * Pattern: Abstract Factory / Interface
 * - load(): Lecture des données -> retourne un DataTable
 * - write(): Écriture des données <- reçoit un DataTable
 * - configure(): Configuration du connecteur via paramètres
 *
 * Point tricky: Chaque connecteur hérite de cette classe et implémente
 * les méthodes spécifiques à son format. Certains formats ne supportent
 * que la lecture (PDF) ou seulement l'écriture.
 *
 * Namespace: dn::connectors
 */

#ifndef DATACONNECTOR_H
#define DATACONNECTOR_H

#include "../core/DataTable.h"
#include <QObject>
#include <QWidget>
#include <memory>

namespace dn::connectors {

    /**
     * @class DataConnector
     * @brief Classe abstraite pour les connecteurs de données
     *
     * Cette classe définit l'interface commune à tous les connecteurs.
     * Elle utilise le pattern "Template Method" pour définir le squelette
     * des opérations load/write/configure que chaque sous-classe implémente.
     *
     * Utilisation typique:
     * @code
     * auto connector = std::make_unique<CSVConnector>();
     * connector->configure({{"fileName", "data.csv"}, {"separator", ","}});
     * auto table = connector->load();
     * connector->write(*table);
     * @endcode
     */
    class DataConnector : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructeur par défaut
         * @param parent Parent Qt pour la gestion de mémoire
         */
        explicit DataConnector(QObject *parent = nullptr) : QObject(parent) {}

        /**
         * @brief Destructeur virtuel
         * Nécessaire pour permettre la destruction polymorphique des sous-classes
         */
        virtual ~DataConnector() {}

        /**
         * @brief Charge les données depuis la source
         * @return Pointeur unique vers un DataTable contenant les données
         * @return nullptr en cas d'erreur
         *
         * Point tricky: Retourne un std::unique_ptr pour transférer la propriété
         * des données. Si la lecture échoue, retourne nullptr.
         */
        virtual std::unique_ptr<dn::core::DataTable> load() = 0;

        /**
         * @brief Écrit les données vers la cible
         * @param data Référence constante vers le DataTable à écrire
         * @return true si l'écriture a réussi, false sinon
         *
         * Implémentation par défaut retourne false (lecture seule).
         * Les sous-classes doivent surcharger cette méthode si elles
         * supportent l'écriture.
         */
        virtual bool write(const dn::core::DataTable& data) {
            Q_UNUSED(data)
            return false;
        }

        /**
         * @brief Configure le connecteur avec des paramètres
         * @param params Map de paramètres (clé-valeur)
         *
         * Paramètres communs:
         * - "fileName": Chemin du fichier
         * - "separator": Séparateur CSV
         * - "sheetName": Nom de la feuille Excel
         *
         * Point tricky: Les paramètres varient selon le type de connecteur.
         * Chaque sous-classe définit ses propres paramètres dans sa documentation.
         */
        virtual void configure(const QMap<QString, QVariant>& params) = 0;

        /**
         * @brief Indique si le connecteur supporte l'écriture
         * @return true si write() est implémenté, false sinon
         *
         * Par défaut retourne false. Les connecteurs qui supportent
         * l'écriture (CSV, Excel, JSON) surchargent cette méthode.
         */
        virtual bool supportsWriting() const { return false; }

        /**
         * @brief Indique si le connecteur supporte la lecture
         * @return true si load() est implémenté, false sinon
         *
         * Par défaut retourne true. Utile pour les connecteurs
         * qui ne font que de l'écriture (certains formats d'export).
         */
        virtual bool supportsReading() const { return true; }
    };
}
#endif // DATACONNECTOR_H
