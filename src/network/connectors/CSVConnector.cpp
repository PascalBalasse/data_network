/**
 * @file CSVConnector.cpp
 * @brief Implémentation du connecteur CSV
 *
 * Implémente les méthodes de lecture et écriture pour les fichiers CSV.
 * Utilise QTextStream pour lire/écrire ligne par ligne.
 * Gère l'échappement des guillemets selon la norme CSV standard.
 *
 * @see CSVConnector.h pour la documentation de l'API
 */

#include "CSVConnector.h"
#include "../core/DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

using namespace dn::connectors;
using namespace dn::core;

CSVConnector::CSVConnector(QObject *parent)
    : DataConnector(parent)
{
}

void CSVConnector::configure(const QMap<QString, QVariant>& params)
{
    // Extrait les paramètres de configuration
    if (params.contains("fileName")) {
        m_fileName = params["fileName"].toString();
    }
    if (params.contains("separator")) {
        QString sep = params["separator"].toString();
        if (!sep.isEmpty()) {
            m_separator = sep[0];
        }
    }
    if (params.contains("hasHeader")) {
        m_hasHeader = params["hasHeader"].toBool();
    }
}


std::unique_ptr<DataTable> CSVConnector::load()
{
    // Validation du nom de fichier
    if (m_fileName.isEmpty())
        return nullptr;

    // Ouverture du fichier en lecture seule
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << m_fileName;
        return nullptr;
    }

    QTextStream stream(&file);
    auto table = std::make_unique<dn::core::DataTable>();

    // Lecture de la première ligne (en-têtes ou données)
    if (stream.atEnd())
        return table;

    QString firstLine = stream.readLine();
    QStringList firstRow = firstLine.split(m_separator);

    if (m_hasHeader) {
        // Première ligne = noms de colonnes
        table->setColumnNames(firstRow);
    } else {
        // Génère des noms par défaut: Column1, Column2, ...
        QStringList defaultHeaders;
        for (int i = 0; i < firstRow.size(); ++i)
            defaultHeaders << QString("Column%1").arg(i+1);
        table->setColumnNames(defaultHeaders);
        // Ajoute la première ligne comme données
        QList<QVariant> row;
        for (const QString &val : firstRow)
            row.append(val);
        table->addRow(row);
    }

    // Lecture des lignes restantes
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList fields = line.split(m_separator);
        // Ignore les lignes mal formées (nombre de champs différent)
        if (fields.size() != table->columnCount())
            continue;
        QList<QVariant> row;
        for (const QString &field : fields)
            row.append(field);
        table->addRow(row);
    }

    file.close();

    // Debug: affiche l'état avant détection de types
    qDebug() << "Before type detection - column types count:" << table->columnTypes().size();
    qDebug() << "Column names count:" << table->columnNames().size();

    // Détection automatique des types de colonnes
    table->autoDetectAndConvertTypes();

    // Debug: affiche les types détectés
    qDebug() << "CSV Loaded:" << table->rowCount() << "rows,"
             << table->columnCount() << "columns";
    for (int i = 0; i < table->columnCount(); ++i) {
        qDebug() << "  -" << table->columnName(i)
        << ":" << static_cast<int>(table->getColumnType(i));
    }

    return table;
}

bool CSVConnector::write(const DataTable& data)
{
    // Validation du nom de fichier
    if (m_fileName.isEmpty()) {
        qWarning() << "No file name specified for saving";
        return false;
    }

    // Ouverture du fichier en écriture
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << m_fileName;
        return false;
    }

    QTextStream stream(&file);

    // Écrit les noms de colonnes (si hasHeader true)
    if (m_hasHeader) {
        for (int col = 0; col < data.columnCount(); ++col) {
            if (col > 0) stream << m_separator;
            QString colName = data.columnName(col);
            // Échappe les guillemets et séparateurs
            if (colName.contains(m_separator) || colName.contains('"')) {
                colName = "\"" + colName.replace("\"", "\"\"") + "\"";
            }
            stream << colName;
        }
        stream << "\n";
    }

    // Écrit chaque ligne de données
    for (int row = 0; row < data.rowCount(); ++row) {
        for (int col = 0; col < data.columnCount(); ++col) {
            if (col > 0) stream << m_separator;
            QString value = data.value(row, col).toString();
            // Échappe les guillemets et séparateurs
            if (value.contains(m_separator) || value.contains('"')) {
                value = "\"" + value.replace("\"", "\"\"") + "\"";
            }
            stream << value;
        }
        stream << "\n";
    }

    file.close();
    qDebug() << "CSV saved:" << data.rowCount() << "rows to" << m_fileName;
    return true;
}
