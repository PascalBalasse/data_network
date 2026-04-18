/**
 * @file JSONConnector.cpp
 * @brief Implémentation du connecteur JSON
 *
 * Gère la lecture et l'écriture de fichiers JSON.
 * Supporte les types: primitives, tableaux, objets, valeurs nulles.
 *
 * @see JSONConnector.h
 */

#include "JSONConnector.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

using namespace dn::core;
using namespace dn::connectors;

/// Configure le connecteur avec les paramètres
void JSONConnector::configure(const QMap<QString, QVariant>& params)
{
    if (params.contains("fileName")) {
        m_fileName = params.value("fileName").toString();
    }
    if (params.contains("prettyPrint")) {
        m_prettyPrint = params.value("prettyPrint").toBool();
    }
}

std::unique_ptr<DataTable> JSONConnector::load()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        return nullptr;
    }

    auto table = std::make_unique<DataTable>();
    *table = parseJson(doc);
    return table;
}

bool JSONConnector::write(const DataTable& data)
{
    QJsonDocument doc;
    
    if (data.rowCount() == 0) {
        doc = QJsonDocument();
    } else {
        QJsonArray rows;
        for (int row = 0; row < data.rowCount(); ++row) {
            QJsonObject rowObj;
            for (int col = 0; col < data.columnCount(); ++col) {
                QString colName = data.columnName(col);
                QVariant value = data.value(row, col);
                rowObj[colName] = jsonValueFromVariant(value);
            }
            rows.append(rowObj);
        }
        doc.setArray(rows);
    }

    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QByteArray dataBytes = m_prettyPrint 
        ? doc.toJson(QJsonDocument::Indented) 
        : doc.toJson(QJsonDocument::Compact);
    file.write(dataBytes);
    file.close();
    return true;
}

DataTable JSONConnector::parseJson(const QJsonDocument& doc)
{
    DataTable table;

    qDebug() << "JSON parsing - isArray:" << doc.isArray() << "isObject:" << doc.isObject();

    if (doc.isArray()) {
        qDebug() << "Top-level is Array";
        QJsonArray array = doc.array();
        if (array.isEmpty()) {
            return table;
        }

        table.setColumnNames({"Liste"});
        table.setColumnTypes({ColumnType::String});

        for (const auto& item : array) {
            QList<QVariant> row;
            row.append(parseJsonValue(item));
            table.addRow(std::move(row));
        }

        detectAndSetComplexColumnTypes(table);
    } else if (doc.isObject()) {
        qDebug() << "Top-level is Object (key-value pairs)";
        table.setColumnNames({"key", "value"});
        table.setColumnTypes({ColumnType::String, ColumnType::String});

        QJsonObject obj = doc.object();
        qDebug() << "JSON object keys:" << obj.keys();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QJsonValue jsonValue = it.value();
            qDebug() << "  Key:" << it.key();
            qDebug() << "    JSON type:" << static_cast<int>(jsonValue.type()) << "(Array=4, Object=5)";
            qDebug() << "    isArray:" << jsonValue.isArray();
            qDebug() << "    isObject:" << jsonValue.isObject();

            if (jsonValue.isObject()) {
                QJsonObject nested = jsonValue.toObject();
                qDebug() << "    Nested object keys:" << nested.keys();
                if (!nested.isEmpty()) {
                    QJsonObject::iterator firstIt = nested.begin();
                    qDebug() << "    First nested key:" << firstIt.key();
                    qDebug() << "    First nested value type:" << static_cast<int>(firstIt.value().type());
                }
            }
            if (jsonValue.isArray()) {
                qDebug() << "    Array size:" << jsonValue.toArray().size();
            }

            QVariant parsedValue = parseJsonValue(jsonValue);
            qDebug() << "    Parsed QVariant typeId:" << parsedValue.typeId()
                     << "(QVariantList=9, QVariantMap=8)";

            QList<QVariant> row;
            row.append(it.key());
            row.append(parsedValue);
            table.addRow(std::move(row));
        }

        detectAndSetComplexColumnTypes(table);
    } else if (doc.isNull()) {
        qDebug() << "Top-level is null";
        table.setColumnNames({"value"});
        table.setColumnTypes({ColumnType::String});
    }

    qDebug() << "Parsed table - rows:" << table.rowCount() << "cols:" << table.columnCount();
    for (int col = 0; col < table.columnCount(); ++col) {
        qDebug() << "  Column" << col << ":" << table.columnName(col) << "type:" << static_cast<int>(table.getColumnType(col));
    }

    return table;
}

QVariant JSONConnector::parseJsonValue(const QJsonValue& value)
{
    switch (value.type()) {
    case QJsonValue::Null:
        return QVariant();
    case QJsonValue::Bool:
        return value.toBool();
    case QJsonValue::Double:
        return value.toDouble();
    case QJsonValue::String:
        return value.toString();
    case QJsonValue::Array: {
        QList<QVariant> list;
        QJsonArray arr = value.toArray();
        for (const auto& item : arr) {
            list.append(parseJsonValue(item));
        }
        return list;
    }
    case QJsonValue::Object: {
        QVariantMap map;
        QJsonObject obj = value.toObject();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            map[it.key()] = parseJsonValue(it.value());
        }
        return map;
    }
    default:
        return QVariant();
    }
}

QJsonValue JSONConnector::jsonValueFromVariant(const QVariant& value)
{
    if (!value.isValid() || value.isNull()) {
        return QJsonValue::Null;
    }

    switch (value.typeId()) {
    case QMetaType::Bool:
        return value.toBool();
    case QMetaType::Int:
    case QMetaType::LongLong:
        return value.toLongLong();
    case QMetaType::Double:
    case QMetaType::Float:
        return value.toDouble();
    case QMetaType::QString:
        return value.toString();
    case QMetaType::QVariantList: {
        QJsonArray arr;
        for (const auto& item : value.toList()) {
            arr.append(jsonValueFromVariant(item));
        }
        return arr;
    }
    case QMetaType::QVariantMap: {
        QJsonObject obj;
        QVariantMap map = value.toMap();
        for (auto it = map.begin(); it != map.end(); ++it) {
            obj[it.key()] = jsonValueFromVariant(it.value());
        }
        return obj;
    }
    default:
        return value.toString();
    }
}

void JSONConnector::detectAndSetComplexColumnTypes(DataTable& table)
{
    qDebug() << "detectAndSetComplexColumnTypes - rows:" << table.rowCount() << "cols:" << table.columnCount();

    for (int col = 0; col < table.columnCount(); ++col) {
        bool hasList = false;
        bool hasPair = false;
        bool pairContainsList = false;
        bool pairContainsPrimitive = false;

        for (int row = 0; row < table.rowCount(); ++row) {
            QVariant value = table.value(row, col);

            bool isList = DataTable::isListValue(value);
            bool isPair = DataTable::isPairValue(value);

            qDebug() << "  Row" << row << "col" << col
                     << "value typeId:" << value.typeId()
                     << "isList:" << isList
                     << "isPair:" << isPair;

            if (isList) {
                hasList = true;
                qDebug() << "    -> Has List";
            }
            if (isPair) {
                hasPair = true;
                QVariantMap map = value.toMap();
                for (auto it = map.begin(); it != map.end(); ++it) {
                    if (DataTable::isListValue(it.value())) {
                        pairContainsList = true;
                        qDebug() << "    -> Pair contains List at key:" << it.key();
                    } else if (DataTable::isPairValue(it.value())) {
                        qDebug() << "    -> Pair contains Pair at key:" << it.key();
                    } else if (it.value().isValid() && !it.value().isNull()) {
                        pairContainsPrimitive = true;
                        qDebug() << "    -> Pair contains primitive at key:" << it.key() << "type:" << it.value().typeId();
                    }
                }
            }
        }

        qDebug() << "  Column" << col << "hasList:" << hasList << "hasPair:" << hasPair
                 << "pairContainsList:" << pairContainsList << "pairContainsPrimitive:" << pairContainsPrimitive;

        if (hasList && !hasPair) {
            table.setColumnType(col, ColumnType::List);
            qDebug() << "    -> Set to List";
        } else if (hasPair && !hasList) {
            if (pairContainsList) {
                table.setColumnType(col, ColumnType::Any);
                qDebug() << "    -> Set to Any (Pair contains List)";
            } else {
                table.setColumnType(col, ColumnType::Pair);
                qDebug() << "    -> Set to Pair";
            }
        } else if (hasList && hasPair) {
            table.setColumnType(col, ColumnType::Any);
            qDebug() << "    -> Set to Any (mixed List and Pair)";
        }
    }
}