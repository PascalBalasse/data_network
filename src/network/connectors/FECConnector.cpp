/**
 * @file FECConnector.cpp
 * @brief Implémentation du connecteur FEC
 *
 * Lit les fichiers FEC (Fichier des Écritures Comptables).
 * Détecte automatiquement le séparateur (tabulation ou pipe).
 *
 * @see FECConnector.h
 */

#include "FECConnector.h"
#include "../core/DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDate>

using namespace dn::connectors;
using namespace dn::core;

/// Constructeur
FECConnector::FECConnector(QObject *parent)
    : DataConnector(parent)
{
}

/// Configure le connecteur
void FECConnector::configure(const QMap<QString, QVariant>& params)
{
    if (params.contains("fileName")) {
        m_fileName = params["fileName"].toString();
    }
    if (params.contains("separator")) {
        QString sep = params["separator"].toString();
        if (!sep.isEmpty()) {
            m_separator = sep[0];
        }
    }
}

/// Lit un fichier FEC
std::unique_ptr<DataTable> FECConnector::load()
{
    if (m_fileName.isEmpty())
        return nullptr;

    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << m_fileName;
        return nullptr;
    }

    QTextStream stream(&file);
    auto table = std::make_unique<DataTable>();

    if (stream.atEnd()) {
        file.close();
        return table;
    }

    // Première ligne = en-têtes
    QString firstLine = stream.readLine();

    // Détecte automatiquement le séparateur
    int tabCount = firstLine.count('\t');
    int pipeCount = firstLine.count('|');
    if (tabCount >= pipeCount && tabCount > 0) {
        m_separator = '\t';
    } else if (pipeCount > 0) {
        m_separator = '|';
    }

    QStringList headers = firstLine.split(m_separator);

    // Nettoie les en-têtes
    for (int i = 0; i < headers.size(); ++i) {
        headers[i] = headers[i].trimmed().remove('"');
    }
    table->setColumnNames(headers);

    // Lit les lignes de données
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList fields = line.split(m_separator);

        if (fields.size() < headers.size())
            continue;

        QList<QVariant> row;
        for (const QString &field : fields) {
            QString trimmed = field.trimmed().remove('"');
            if (trimmed.isEmpty()) {
                row.append(QVariant());
            } else {
                row.append(trimmed);
            }
        }
        table->addRow(row);
    }

    file.close();

    // Détection automatique des types
    table->autoDetectAndConvertTypes();

    qDebug() << "FEC Loaded:" << table->rowCount() << "rows,"
             << table->columnCount() << "columns";

    return table;
}
