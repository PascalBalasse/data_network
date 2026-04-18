/**
 * @file ExcelConnector.h
 * @brief Connecteur pour les fichiers Excel (.xlsx)
 *
 * Lit et écrit des fichiers Excel au format Office Open XML.
 * Utilise ZipReader/ZipWriter pour manipuler le contenu ZIP interne.
 * Supporte plusieurs feuilles de calcul.
 *
 * Paramètres:
 * - "fileName": Chemin du fichier .xlsx
 * - "sheetName": Nom de la feuille à lire/écrire
 * - "hasHeader": Première ligne = en-têtes (défaut: true)
 *
 * Note: Supporte lecture et écriture.
 *
 * Namespace: dn::connectors
 */

#ifndef EXCELCONNECTOR_H
#define EXCELCONNECTOR_H

#include "DataConnector.h"
#include "ZipReader.h"
#include "ZipWriter.h"
#include <QString>
#include <QMap>
#include <QVariant>
#include <QXmlStreamReader>

using namespace dn::core;

namespace dn::connectors {

    class ExcelConnector : public DataConnector
    {
        Q_OBJECT

    public:
        explicit ExcelConnector(QObject *parent = nullptr);
        ~ExcelConnector() override;

        /// @brief Lit une feuille Excel et retourne un DataTable
        std::unique_ptr<DataTable> load() override;

        /// @brief Écrit un DataTable dans une feuille Excel
        bool write(const DataTable& data) override;

        /// @brief Configure le connecteur
        void configure(const QMap<QString, QVariant>& params) override;

        bool supportsWriting() const override { return true; }
        bool supportsReading() const override { return true; }

        QString getFileName() const { return m_fileName; }
        QString getSheetName() const { return m_sheetName; }
        bool hasHeader() const { return m_hasHeader; }

        QStringList getAvailableSheets() const { return m_availableSheets; }

    private:
        bool parseXlsx();
        bool parseSheet(const QString& sheetPath);
        void parseSharedStrings();
        QString getCellValue(const QXmlStreamAttributes& attrs, const QString& cellType);

        QString m_fileName;
        QString m_sheetName;
        bool m_hasHeader = true;
        QStringList m_availableSheets;

        QMap<QString, QStringList> m_sharedStrings;
        QVector<QVector<QVariant>> m_cells;
        int m_maxColumn = 0;
    };

}

#endif
