/**
 * @file PDFConnector.cpp
 * @brief Implémentation du connecteur PDF
 *
 * Lit les données depuis des fichiers PDF contenant des tableaux.
 * Utilise la bibliothèque Poppler pour extraire le texte.
 *
 * @see PDFConnector.h
 */

#include "PDFConnector.h"
#include "../core/DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QVector>

using namespace dn::connectors;
using namespace dn::core;

PDFConnector::PDFConnector(QObject *parent)
    : DataConnector(parent)
{
}

PDFConnector::~PDFConnector()
{
}

/// Configure le connecteur
void PDFConnector::configure(const QMap<QString, QVariant>& params)
{
    if (params.contains("fileName")) {
        m_fileName = params["fileName"].toString();
    }
    if (params.contains("startPage")) {
        m_startPage = params["startPage"].toInt();
    }
    if (params.contains("endPage")) {
        m_endPage = params["endPage"].toInt();
    }
    if (params.contains("hasHeader")) {
        m_hasHeader = params["hasHeader"].toBool();
    }
}

/// Lit les données du PDF et les convertit en DataTable
std::unique_ptr<DataTable> PDFConnector::load()
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

    if (!parsePDF()) {
        qWarning() << "Failed to parse PDF file";
        return nullptr;
    }

    auto table = std::make_unique<DataTable>();

    if (m_extractedLines.isEmpty()) {
        qWarning() << "No text extracted from PDF";
        return table;
    }

    // Définit la ligne de début (skip header si demandé)
    int startRow = m_hasHeader ? 1 : 0;
    QStringList headers;

    // Extrait les en-têtes depuis la première ligne
    if (m_hasHeader && !m_extractedLines.isEmpty()) {
        headers = parseLinesToColumns(QStringList() << m_extractedLines.first());
        if (!headers.isEmpty()) {
            table->setColumnNames(headers);
        }
    }

    //Génère des noms de colonnes par défaut si nécessaire
    if (table->columnCount() == 0) {
        QStringList firstRow = parseLinesToColumns(QStringList() << m_extractedLines.first());
        for (int i = 0; i < firstRow.size(); ++i) {
            table->setColumnNames(QStringList() << QString("Column%1").arg(i + 1));
        }
    }

    // Ajoute les lignes de données
    for (int i = startRow; i < m_extractedLines.size(); ++i) {
        QStringList columns = parseLinesToColumns(QStringList() << m_extractedLines[i]);
        QList<QVariant> row;
        for (int c = 0; c < table->columnCount(); ++c) {
            if (c < columns.size()) {
                row.append(columns[c]);
            } else {
                row.append(QVariant());
            }
        }
        table->addRow(row);
    }

    // Détecte et convertit les types
    table->autoDetectAndConvertTypes();

    qDebug() << "PDF Loaded:" << table->rowCount() << "rows," << table->columnCount() << "columns";

    return table;
}

/// Non supporté (lecture seule)
bool PDFConnector::write(const DataTable& data)
{
    Q_UNUSED(data)
    qWarning() << "PDF writing not supported";
    return false;
}

/// Parse le fichier PDF et extrait le texte
bool PDFConnector::parsePDF()
{
    m_extractedLines.clear();
    m_pageCount = 0;

    QFile file(m_fileName);
    if (!file.exists()) {
        return false;
    }

    poppler::document* doc = poppler::document::load_from_file(m_fileName.toStdString());
    if (!doc) {
        qWarning() << "Failed to load PDF document";
        return false;
    }

    if (doc->is_encrypted()) {
        qWarning() << "PDF is encrypted";
        delete doc;
        return false;
    }

    m_pageCount = doc->pages();
    int start = (m_startPage > 0) ? m_startPage - 1 : 0;
    int end = (m_endPage > 0 && m_endPage <= m_pageCount) ? m_endPage : m_pageCount;

    for (int i = start; i < end; ++i) {
        extractPageData(i);
    }

    delete doc;
    return true;
}

/// Extrait le texte d'une page PDF
bool PDFConnector::extractPageData(int pageNum)
{
    poppler::document* doc = poppler::document::load_from_file(m_fileName.toStdString());
    if (!doc) {
        return false;
    }

    poppler::page* page = doc->create_page(pageNum);
    if (!page) {
        delete doc;
        return false;
    }

    std::vector<poppler::text_box> list = page->text_list();

    qDebug() << "=== PDF Page" << pageNum << "text extraction ===";
    qDebug() << "Total text boxes:" << list.size();

    for (size_t i = 0; i < list.size(); ++i) {
        poppler::text_box& box = list[i];
        poppler::ustring ustr = box.text();
        #pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        QString text = QString::fromUtf16(ustr.data());
#pragma GCC diagnostic pop
        if (text.isEmpty()) continue;

        poppler::rectf b = box.bbox();
        qDebug() << "  Word[" << i << "]:"
                 << "text='" << text << "'"
                 << "x=" << b.x() << "y=" << b.y()
                 << "w=" << b.width() << "h=" << b.height();
    }

    QString currentLine;
    QString lastText;
    qreal lastX = 0;

    // Reconstruct les lignes à partir des boxes de texte
    for (poppler::text_box& box : list) {
        poppler::ustring ustr = box.text();
        #pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        QString text = QString::fromUtf16(ustr.data());
#pragma GCC diagnostic pop
        if (text.isEmpty()) continue;

        poppler::rectf b = box.bbox();
        qreal x = b.x();

        if (!lastText.isEmpty()) {
            // Détecte les sauts de ligne basés sur la distance
            qreal distance = x - lastText.length() * 6;
            if (distance > 30) {
                if (!currentLine.trimmed().isEmpty()) {
                    m_extractedLines.append(currentLine.trimmed());
                }
                currentLine = text;
            } else {
                currentLine += " " + text;
            }
        } else {
            currentLine = text;
        }
        lastText = text;
    }

    if (!currentLine.trimmed().isEmpty()) {
        m_extractedLines.append(currentLine.trimmed());
    }

    delete page;
    delete doc;
    return true;
}

/// Convertit les lignes de texte en colonnes
QStringList PDFConnector::parseLinesToColumns(const QStringList& lines)
{
    if (lines.isEmpty()) {
        return QStringList();
    }

    QString line = lines.first();

    // Essaye d'abord les tabulations
    QVector<int> tabStops;

    for (int i = 0; i < line.length(); ++i) {
        if (line[i] == '\t') {
            tabStops.append(i);
        }
    }

    if (tabStops.size() >= 2) {
        QStringList result;
        int lastPos = 0;
        for (int pos : tabStops) {
            result.append(line.mid(lastPos, pos - lastPos).trimmed());
            lastPos = pos + 1;
        }
        result.append(line.mid(lastPos).trimmed());
        return result;
    }

    // Sinon, détecte les espaces consistants entre les lignes
    QVector<int> spaces;
    int inSpace = 0;
    for (int i = 0; i < line.length(); ++i) {
        if (line[i] == ' ') {
            inSpace++;
        } else {
            if (inSpace >= 2) {
                spaces.append(i - inSpace);
            }
            inSpace = 0;
        }
    }

    QVector<int> consistentPositions;
    for (int pos : spaces) {
        bool consistent = true;
        for (const QString& otherLine : lines) {
            if (pos < otherLine.length()) {
                if (otherLine[pos] != ' ') {
                    consistent = false;
                    break;
                }
            }
        }
        if (consistent) {
            consistentPositions.append(pos);
        }
    }

    if (consistentPositions.size() >= 2) {
        QStringList result;
        int lastPos = 0;
        for (int pos : consistentPositions) {
            result.append(line.mid(lastPos, pos - lastPos).trimmed());
            lastPos = pos;
        }
        result.append(line.mid(lastPos).trimmed());
        return result;
    }

    // Dernier recours: une seule colonne
    QStringList result;
    result.append(line);
    return result;
}

/// Retourne la liste des pages disponibles
QStringList PDFConnector::getAvailablePages() const
{
    QStringList pages;
    for (int i = 1; i <= m_pageCount; ++i) {
        pages.append(QString("Page %1").arg(i));
    }
    return pages;
}