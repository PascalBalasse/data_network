/**
 * @file PDFConnector.h
 * @brief Connecteur pour les fichiers PDF
 *
 * Lit les données depuis des fichiers PDF contenant des tableaux.
 * Utilise la bibliothèque Poppler pour le parsing.
 *
 * Paramètres:
 * - "fileName": Chemin du fichier PDF
 * - "startPage": Page de début (défaut: 1)
 * - "endPage": Page de fin (-1 = toutes)
 * - "hasHeader": Première ligne = en-têtes
 *
 * Note: Lecture seule (écriture non supportée).
 * Dépendance: libpoppler-qt5
 *
 * Namespace: dn::connectors
 */

#ifndef PDFCONNECTOR_H
#define PDFCONNECTOR_H

#include "DataConnector.h"
#include <QString>
#include <QMap>
#include <QVariant>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

namespace dn::connectors {

    class PDFConnector : public DataConnector
    {
        Q_OBJECT

    public:
        explicit PDFConnector(QObject *parent = nullptr);
        ~PDFConnector() override;

        /// @brief Lit les données du PDF
        std::unique_ptr<dn::core::DataTable> load() override;

        /// @brief Non supporté (lecture seule)
        bool write(const dn::core::DataTable& data) override;

        /// @brief Configure le connecteur
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return false; }
        bool supportsReading() const override { return true; }

        QString getFileName() const { return m_fileName; }
        int getPageCount() const { return m_pageCount; }
        QStringList getAvailablePages() const;

    private:
        bool parsePDF();
        bool extractPageData(int pageNum);
        QStringList parseLinesToColumns(const QStringList& lines);

        QString m_fileName;
        int m_startPage = 1;
        int m_endPage = -1;
        bool m_hasHeader = true;
        int m_pageCount = 0;

        QStringList m_extractedLines;
    };

}

#endif // PDFCONNECTOR_H