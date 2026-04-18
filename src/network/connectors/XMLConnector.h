/**
 * @file XMLConnector.h
 * @brief Connecteur pour les fichiers XML
 *
 * Permet la lecture et écriture de données au format XML.
 * Supporte les formats:
 * - Simple: lignes comme éléments, colonnes comme sous-éléments
 * - Attributs: données dans les attributs des éléments
 *
 * Paramètres:
 * - "fileName": Chemin du fichier XML
 * - "rootElement": Nom de l'élément racine
 * - "rowElement": Nom de l'élément représentant une ligne
 *
 * Namespace: dn::connectors
 */

#ifndef XMLCONNECTOR_H
#define XMLCONNECTOR_H

#include "DataConnector.h"
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace dn::connectors {

    class XMLConnector : public DataConnector {
    public:
        explicit XMLConnector(QObject *parent = nullptr) : DataConnector(parent) {}

        std::unique_ptr<dn::core::DataTable> load() override;
        bool write(const dn::core::DataTable& data) override;
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return true; }
        bool supportsReading() const override { return true; }

    private:
        QString m_fileName;
        QString m_rootElement = "data";
        QString m_rowElement = "row";

        dn::core::DataTable parseXML();
        QStringList parseElements(QXmlStreamReader& reader, const QString& rowElement);
    };

}

#endif // XMLCONNECTOR_H