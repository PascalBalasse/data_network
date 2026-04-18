/**
 * @file XMLConnector.cpp
 * @brief Implémentation du connecteur XML
 *
 * @see XMLConnector.h
 */

#include "XMLConnector.h"
#include "../core/DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

using namespace dn::connectors;
using namespace dn::core;

void XMLConnector::configure(const QMap<QString, QVariant>& params)
{
    if (params.contains("fileName")) {
        m_fileName = params["fileName"].toString();
    }
    if (params.contains("rootElement")) {
        m_rootElement = params["rootElement"].toString();
    }
    if (params.contains("rowElement")) {
        m_rowElement = params["rowElement"].toString();
    }
}

std::unique_ptr<DataTable> XMLConnector::load()
{
    return std::make_unique<DataTable>(parseXML());
}

DataTable XMLConnector::parseXML()
{
    DataTable table;

    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << m_fileName;
        return table;
    }

    QXmlStreamReader reader(&file);

    if (!reader.readNextStartElement()) {
        qWarning() << "Invalid XML file";
        file.close();
        return table;
    }

    QString rootName = reader.name().toString();
    qDebug() << "Root element:" << rootName;

    QStringList columnNames;
    bool firstRow = true;

    while (reader.readNextStartElement()) {
        QString elementName = reader.name().toString();

        if (elementName != m_rowElement) {
            reader.skipCurrentElement();
            continue;
        }

        QXmlStreamAttributes attrs = reader.attributes();
        QStringList rowValues;

        if (firstRow) {
            for (const QXmlStreamAttribute& attr : attrs) {
                columnNames.append(attr.name().toString());
            }
            table.setColumnNames(columnNames);
            firstRow = false;
        }

        if (!firstRow && attrs.hasAttribute("text")) {
            QString text = attrs.value("text").toString();
            rowValues.append(text);
        }

        while (reader.readNextStartElement()) {
            QString childName = reader.name().toString();
            QString childText = reader.readElementText();

            if (firstRow) {
                columnNames.append(childName);
                table.setColumnNames(columnNames);
            }

            rowValues.append(childText);
        }

        if (!rowValues.isEmpty() || !firstRow) {
            QList<QVariant> row;
            for (const QString& val : rowValues) {
                row.append(val);
            }
            table.addRow(row);
        }

        if (firstRow) {
            firstRow = false;
        }
    }

    file.close();

    if (firstRow && table.rowCount() > 0) {
        firstRow = false;
    }

    table.autoDetectAndConvertTypes();

    qDebug() << "XML Loaded:" << table.rowCount() << "rows,"
             << table.columnCount() << "columns";

    return table;
}

bool XMLConnector::write(const DataTable& data)
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << m_fileName;
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement(m_rootElement);

    if (data.columnCount() > 0) {
        writer.writeStartElement(m_rowElement);

        for (int col = 0; col < data.columnCount(); ++col) {
            writer.writeTextElement(data.columnName(col),
                               data.value(0, col).toString());
        }

        writer.writeEndElement();

        for (int row = 1; row < data.rowCount(); ++row) {
            writer.writeStartElement(m_rowElement);

            for (int col = 0; col < data.columnCount(); ++col) {
                writer.writeTextElement(data.columnName(col),
                                     data.value(row, col).toString());
            }

            writer.writeEndElement();
        }
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    file.close();

    qDebug() << "XML saved:" << data.rowCount() << "rows to" << m_fileName;
    return true;
}