/**
 * @file FECConnector.h
 * @brief Connecteur pour les fichiers FEC (Fichier des Écritures Comptables)
 *
 * Lit les fichiers FEC au format tabulé standard français.
 * Le séparateur par défaut est la tabulation.
 *
 * Paramètres:
 * - "fileName": Chemin du fichier FEC
 * - "separator": Séparateur (défaut: '\t' tabulation)
 *
 * Note: Lecture seule (format comptable standard).
 *
 * Namespace: dn::connectors
 */

#ifndef FECCONNECTOR_H
#define FECCONNECTOR_H

#include "DataConnector.h"

namespace dn::connectors {

    class FECConnector : public DataConnector
    {
        Q_OBJECT

    public:
        explicit FECConnector(QObject *parent = nullptr);

        /// @brief Lit un fichier FEC
        std::unique_ptr<dn::core::DataTable> load() override;

        /// @brief Configure le connecteur
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return false; }
        bool supportsReading() const override { return true; }

        QString getFileName() const { return m_fileName; }
        QChar getSeparator() const { return m_separator; }

    private:
        QString m_fileName;
        QChar m_separator = '\t';
    };

}

#endif // FECCONNECTOR_H
