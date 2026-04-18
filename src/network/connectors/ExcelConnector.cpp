/**
 * @file ExcelConnector.cpp
 * @brief Implémentation du connecteur Excel (.xlsx)
 *
 * Lit et écrit des fichiers Excel au format Office Open XML.
 * Utilise ZipReader/ZipWriter pour le contenu ZIP interne.
 * Parse les fichiers XML internes (sheet, workbook, sharedStrings).
 *
 * @see ExcelConnector.h
 */

#include "ExcelConnector.h"
#include "../core/DataTable.h"
#include "ZipWriter.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

using namespace dn::connectors;
using namespace dn::core;

ExcelConnector::ExcelConnector(QObject *parent)
    : DataConnector(parent)
{
}

ExcelConnector::~ExcelConnector()
{
}

/// Configure le connecteur
void ExcelConnector::configure(const QMap<QString, QVariant>& params)
{
    if (params.contains("fileName")) {
        m_fileName = params["fileName"].toString();
    }
    if (params.contains("sheetName")) {
        m_sheetName = params["sheetName"].toString();
    }
    if (params.contains("hasHeader")) {
        m_hasHeader = params["hasHeader"].toBool();
    }
}

std::unique_ptr<DataTable> ExcelConnector::load()
{
    if (m_fileName.isEmpty()) {
        qWarning() << "No file name specified";
        return nullptr;
    }

    QFile file(m_fileName);
    if (!file.exists()) {
        qWarning() << "File does not exist:" << m_fileName;
        return nullptr;
    }

    if (!parseXlsx()) {
        qWarning() << "Failed to parse xlsx file";
        return nullptr;
    }

    auto table = std::make_unique<DataTable>();

    int startRow = m_hasHeader ? 1 : 0;
    int rowCount = m_cells.size();

    if (rowCount == 0) {
        return table;
    }

    if (m_hasHeader && !m_cells.isEmpty()) {
        const QVector<QVariant>& headerRow = m_cells[0];
        QStringList headers;
        for (int i = 0; i < headerRow.size(); ++i) {
            headers.append(headerRow[i].toString());
        }
        table->setColumnNames(headers);
    } else {
        QStringList headers;
        for (int i = 0; i < m_maxColumn; ++i) {
            headers.append(QString("Column%1").arg(i + 1));
        }
        table->setColumnNames(headers);
    }

    for (int r = startRow; r < rowCount; ++r) {
        const QVector<QVariant>& row = m_cells[r];
        QList<QVariant> tableRow;
        for (int c = 0; c < table->columnCount(); ++c) {
            if (c < row.size()) {
                tableRow.append(row[c]);
            } else {
                tableRow.append(QVariant());
            }
        }
        table->addRow(tableRow);
    }

    table->autoDetectAndConvertTypes();

    qDebug() << "Excel Loaded:" << table->rowCount() << "rows,"
             << table->columnCount() << "columns";

    return table;
}

bool ExcelConnector::write(const DataTable& data)
{
    if (m_fileName.isEmpty()) {
        qWarning() << "No file name specified for writing";
        return false;
    }

    ZipWriter zip(m_fileName);
    if (!zip.isValid()) {
        qWarning() << "Failed to create zip writer:" << zip.errorString();
        return false;
    }

    QStringList headers = data.columnNames();
    const QList<QList<QVariant>>& rows = data.rows();

    QStringList sharedStrings;
    QMap<QString, int> sharedStringIndex;
    int nextStringIndex = 0;

    auto getStringIndex = [&](const QString& str) -> int {
        if (sharedStringIndex.contains(str)) {
            return sharedStringIndex[str];
        }
        int idx = nextStringIndex++;
        sharedStrings.append(str);
        sharedStringIndex[str] = idx;
        return idx;
    };

    QByteArray sheetXml;
    QXmlStreamWriter writer(&sheetXml);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("worksheet");
    writer.writeAttribute("xmlns", "http://schemas.openxmlformats.org/spreadsheetml/2006/main");
    writer.writeAttribute("xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships");

    writer.writeStartElement("sheetData");

    int rowNum = 1;
    if (m_hasHeader && !headers.isEmpty()) {
        writer.writeStartElement("row");
        writer.writeAttribute("r", QString::number(rowNum));

        for (int col = 0; col < headers.size(); ++col) {
            writer.writeStartElement("c");
            writer.writeAttribute("r", QString("%1%2").arg(QChar('A' + col)).arg(rowNum));
            writer.writeAttribute("t", "s");
            writer.writeTextElement("v", QString::number(getStringIndex(headers[col])));
            writer.writeEndElement();
        }
        writer.writeEndElement();
        rowNum++;
    }

    for (const QList<QVariant>& dataRow : rows) {
        writer.writeStartElement("row");
        writer.writeAttribute("r", QString::number(rowNum));

        for (int col = 0; col < dataRow.size(); ++col) {
            const QVariant& val = dataRow[col];
            QString cellRef = QString("%1%2").arg(QChar('A' + col)).arg(rowNum);

            writer.writeStartElement("c");
            writer.writeAttribute("r", cellRef);

            if (val.isNull() || !val.isValid()) {
                writer.writeEndElement();
                continue;
            }

            switch (val.typeId()) {
            case QMetaType::Int:
            case QMetaType::LongLong:
            case QMetaType::UInt:
            case QMetaType::ULongLong:
                writer.writeTextElement("v", val.toString());
                break;
            case QMetaType::Double:
            case QMetaType::Float:
                writer.writeTextElement("v", QString::number(val.toDouble(), 'g', 15));
                break;
            case QMetaType::Bool:
                writer.writeTextElement("v", val.toBool() ? "1" : "0");
                break;
            default:
                writer.writeAttribute("t", "s");
                writer.writeTextElement("v", QString::number(getStringIndex(val.toString())));
                break;
            }
            writer.writeEndElement();
        }

        writer.writeEndElement();
        rowNum++;
    }

    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeEndDocument();

    QByteArray sharedStringsXml;
    if (!sharedStrings.isEmpty()) {
        QXmlStreamWriter ssWriter(&sharedStringsXml);
        ssWriter.setAutoFormatting(true);
        ssWriter.writeStartDocument();
        ssWriter.writeStartElement("sst");
        ssWriter.writeAttribute("xmlns", "http://schemas.openxmlformats.org/spreadsheetml/2006/main");
        ssWriter.writeAttribute("count", QString::number(sharedStrings.size()));
        ssWriter.writeAttribute("uniqueCount", QString::number(sharedStrings.size()));

        for (const QString& str : sharedStrings) {
            ssWriter.writeStartElement("si");
            ssWriter.writeTextElement("t", str);
            ssWriter.writeEndElement();
        }

        ssWriter.writeEndElement();
        ssWriter.writeEndDocument();
    }

    QByteArray contentTypesXml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
    <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
    <Default Extension="xml" ContentType="application/xml"/>
    <Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
    <Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
    <Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>
</Types>)";

    QByteArray relsXml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>
</Relationships>)";

    QByteArray workbookXml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
    <sheets>
        <sheet name="Sheet1" sheetId="1" r:id="rId1"/>
    </sheets>
</workbook>)";

    QByteArray workbookRelsXml = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
    <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>
    <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>
</Relationships>)";

    zip.addEntry("[Content_Types].xml", contentTypesXml);
    zip.addEntry("_rels/.rels", relsXml);
    zip.addEntry("xl/workbook.xml", workbookXml);
    zip.addEntry("xl/_rels/workbook.xml.rels", workbookRelsXml);
    zip.addEntry("xl/worksheets/sheet1.xml", sheetXml);
    if (!sharedStrings.isEmpty()) {
        zip.addEntry("xl/sharedStrings.xml", sharedStringsXml);
    } else {
        zip.addEntry("xl/sharedStrings.xml", QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<sst xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"0\" uniqueCount=\"0\"></sst>"));
    }

    qDebug() << "Excel write: fileName =" << m_fileName;
    qDebug() << "Excel write: sharedStrings count =" << sharedStrings.size();

    if (!zip.write()) {
        qWarning() << "Failed to write xlsx file:" << zip.errorString();
        return false;
    }

    qDebug() << "Excel file written successfully:" << m_fileName;
    qDebug() << "Rows written:" << data.rowCount() << "Columns:" << data.columnCount();
    return true;
}

bool ExcelConnector::parseXlsx()
{
    ZipReader zip(m_fileName);
    if (!zip.isValid()) {
        qWarning() << "Failed to open xlsx as zip:" << zip.errorString();
        return false;
    }

    // Parse workbook to get sheet names
    QByteArray workbookData = zip.entryData("xl/workbook.xml");
    if (workbookData.isEmpty()) {
        qWarning() << "workbook.xml not found in xlsx";
        return false;
    }

    m_availableSheets.clear();
    QXmlStreamReader reader(workbookData);
    while (!reader.atEnd()) {
        if (reader.isStartElement() && reader.name() == u"sheet") {
            QString name = reader.attributes().value("name").toString();
            m_availableSheets.append(name);
        }
        reader.readNext();
    }

    // If no sheet specified, use first sheet
    if (m_sheetName.isEmpty() && !m_availableSheets.isEmpty()) {
        m_sheetName = m_availableSheets[0];
    }

    // Parse shared strings
    parseSharedStrings();

    // Find the sheet file
    QString sheetPath;
    QByteArray workbookRels = zip.entryData("xl/_rels/workbook.xml.rels");
    if (!workbookRels.isEmpty()) {
        QXmlStreamReader relReader(workbookRels);
        while (!relReader.atEnd()) {
            if (relReader.isStartElement() && relReader.name() == u"Relationship") {
                QString id = relReader.attributes().value("Id").toString();
                QString target = relReader.attributes().value("Target").toString();

                // Find sheet with this id in workbook
                QXmlStreamReader wbReader(workbookData);
                while (!wbReader.atEnd()) {
                    if (wbReader.isStartElement() && wbReader.name() == u"sheet") {
                        QString sheetId = wbReader.attributes().value("r:id").toString();
                        QString sheetName = wbReader.attributes().value("name").toString();
                        if (sheetId == id && sheetName == m_sheetName) {
                            sheetPath = "xl/" + target;
                            break;
                        }
                    }
                    wbReader.readNext();
                }
                if (!sheetPath.isEmpty()) break;
            }
            relReader.readNext();
        }
    }

    // Fallback: try common sheet paths
    if (sheetPath.isEmpty()) {
        QStringList candidates = {
            QString("xl/worksheets/sheet%1.xml").arg(1),
            "xl/worksheets/sheet1.xml"
        };
        for (const QString& candidate : candidates) {
            if (zip.contains(candidate)) {
                sheetPath = candidate;
                break;
            }
        }
    }

    if (sheetPath.isEmpty()) {
        qWarning() << "Could not find sheet file for:" << m_sheetName;
        return false;
    }

    return parseSheet(sheetPath);
}

void ExcelConnector::parseSharedStrings()
{
    ZipReader zip(m_fileName);
    QByteArray sharedStringsData = zip.entryData("xl/sharedStrings.xml");

    if (sharedStringsData.isEmpty()) {
        return;
    }

    m_sharedStrings.clear();
    int currentIndex = -1;
    QStringList currentStrings;

    QXmlStreamReader reader(sharedStringsData);
    while (!reader.atEnd()) {
        if (reader.isStartElement()) {
            QString name = reader.name().toString();
            if (name == "si") {
                currentStrings.clear();
            } else if (name == "t") {
                QString text = reader.readElementText();
                currentStrings.append(text);
            }
        } else if (reader.isEndElement() && reader.name() == u"si") {
            if (!currentStrings.isEmpty()) {
                m_sharedStrings[QString::number(++currentIndex)] = currentStrings;
            }
        }
        reader.readNext();
    }
}

bool ExcelConnector::parseSheet(const QString& sheetPath)
{
    ZipReader zip(m_fileName);
    QByteArray sheetData = zip.entryData(sheetPath);

    if (sheetData.isEmpty()) {
        qWarning() << "Sheet data not found:" << sheetPath;
        qDebug() << "Available entries:" << zip.entryNames();
        return false;
    }

    qDebug() << "Sheet data size:" << sheetData.size() << "bytes";
    qDebug() << "First 500 chars:" << QString::fromUtf8(sheetData.left(500));

    m_cells.clear();
    m_maxColumn = 0;

    QXmlStreamReader reader(sheetData);
    QVector<QVariant> currentRow;
    int currentCol = 0;
    int rowCount = 0;

    while (!reader.atEnd()) {
        if (reader.isStartElement()) {
            QString name = reader.name().toString();

            if (name == "row") {
                currentRow.clear();
                currentCol = 0;
                rowCount++;
            } else if (name == "c") {
                QString cellRef = reader.attributes().value("r").toString();
                QString cellType = reader.attributes().value("t").toString();

                // Calculate column from cell reference
                int col = 0;
                for (QChar c : cellRef) {
                    if (c.isLetter()) {
                        col = col * 26 + (c.toUpper().unicode() - 'A' + 1);
                    } else {
                        break;
                    }
                }
                col--; // 0-indexed

                // Fill empty cells
                while (currentCol < col) {
                    currentRow.append(QVariant());
                    currentCol++;
                }

                // Read value
                QString value;
                while (reader.readNext(), !reader.atEnd()) {
                    if (reader.isStartElement() && reader.name() == u"v") {
                        value = reader.readElementText();
                        break;
                    }
                    if (reader.isEndElement() && reader.name() == u"c") {
                        break;
                    }
                }

                QVariant cellValue;
                if (!value.isEmpty()) {
                    if (cellType == "s") {
                        // Shared string
                        int index = value.toInt();
                        QString key = QString::number(index);
                        if (m_sharedStrings.contains(key)) {
                            cellValue = m_sharedStrings[key].join("");
                        } else {
                            qDebug() << "Shared string index" << index << "not found, keys:" << m_sharedStrings.keys();
                        }
                    } else if (cellType == "b") {
                        cellValue = value == "1";
                    } else if (cellType == "n" || cellType.isEmpty()) {
                        if (value.contains('.')) {
                            cellValue = value.toDouble();
                        } else {
                            cellValue = value.toLongLong();
                        }
                    } else {
                        cellValue = value;
                    }
                    qDebug() << "Cell" << cellRef << "type:" << cellType << "value:" << cellValue;
                }

                currentRow.append(cellValue);
                currentCol++;
                m_maxColumn = qMax(m_maxColumn, currentCol);
            }
        } else if (reader.isEndElement() && reader.name() == u"row") {
            if (!currentRow.isEmpty()) {
                m_cells.append(currentRow);
            }
        }
        reader.readNext();
    }

    qDebug() << "Parsed" << rowCount << "rows, found" << m_cells.size() << "non-empty rows, max column:" << m_maxColumn;

    return true;
}
