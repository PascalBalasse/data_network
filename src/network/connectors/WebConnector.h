/**
 * @file WebConnector.h
 * @brief Connecteur pour les données depuis une page web
 *
 * Permet de récupérer les données depuis une URL web.
 * Extrait les tableaux HTML de la page.
 *
 * Paramètres:
 * - "url": URL de la page web
 * - "tableIndex": Index du tableau à extraire (0 = premier)
 *
 * Namespace: dn::connectors
 */

#ifndef WEBCONNECTOR_H
#define WEBCONNECTOR_H

#include "DataConnector.h"
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

namespace dn::connectors {

    class WebConnector : public DataConnector {
    Q_OBJECT

    public:
        explicit WebConnector(QObject *parent = nullptr);
        ~WebConnector();

        std::unique_ptr<dn::core::DataTable> load() override;
        bool write(const dn::core::DataTable& data) override;
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return false; }
        bool supportsReading() const override { return true; }

    private:
        QString m_url;
        int m_tableIndex = 0;

        QNetworkAccessManager* m_networkManager = nullptr;

        dn::core::DataTable parseHTMLFromUrl();
        dn::core::DataTable parseHTML(const QString& html);
        QStringList extractTables(const QString& html);
        QString extractTableAtIndex(const QString& html, int index);
        QStringList parseTableRows(const QString& tableHtml);
        QStringList parseTableHeaders(const QString& tableHtml);
    };

}

#endif // WEBCONNECTOR_H