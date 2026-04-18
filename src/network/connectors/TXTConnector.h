/**
 * @file TXTConnector.h
 * @brief Connecteur pour les fichiers TXT (texte délimité par tabulation)
 *
 * Connecteur spécialisé pour les fichiers texte avec tabulation.
 * Hérite de CSVConnector avec "\t" comme séparateur par défaut.
 *
 * Paramètres:
 * - "fileName": Chemin du fichier TXT
 * - "separator": Séparateur (par défaut: tabulation)
 * - "hasHeader": Première ligne = en-têtes
 *
 * Namespace: dn::connectors
 */

#ifndef TXTCONNECTOR_H
#define TXTCONNECTOR_H

#include "CSVConnector.h"

namespace dn::connectors {

    class TXTConnector : public CSVConnector
    {
        Q_OBJECT

    public:
        explicit TXTConnector(QObject *parent = nullptr)
            : CSVConnector(parent)
        {
        }

        void configure(const QMap<QString, QVariant>& params) override
        {
            QMap<QString, QVariant> adjustedParams = params;
            if (!params.contains("separator")) {
                adjustedParams["separator"] = "\t";
            }
            CSVConnector::configure(adjustedParams);
        }
    };

}

#endif // TXTCONNECTOR_H