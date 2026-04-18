/**
 * @file WebConnector.cpp
 * @brief Implémentation du connecteur web
 *
 * @see WebConnector.h
 */

#include "WebConnector.h"
#include "../core/DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>
#include <QRegularExpression>
#include <QEventLoop>

using namespace dn::connectors;
using namespace dn::core;

WebConnector::WebConnector(QObject *parent)
    : DataConnector(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

WebConnector::~WebConnector()
{
}

void WebConnector::configure(const QMap<QString, QVariant>& params)
{
    if (params.contains("url")) {
        m_url = params["url"].toString();
    }
    if (params.contains("tableIndex")) {
        m_tableIndex = params["tableIndex"].toInt();
    }
}

std::unique_ptr<DataTable> WebConnector::load()
{
    return std::make_unique<DataTable>(parseHTMLFromUrl());
}

DataTable WebConnector::parseHTMLFromUrl()
{
    DataTable table;

    if (m_url.isEmpty()) {
        qWarning() << "No URL specified";
        return table;
    }

    QNetworkRequest req;
    req.setUrl(QUrl(m_url));
    req.setRawHeader("User-Agent", "Mozilla/5.0");

    QEventLoop loop;
    QNetworkReply* reply = m_networkManager->get(req);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return table;
    }

    QString html = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    qDebug() << "HTML fetched, size:" << html.size();

    table = parseHTML(html);

    table.autoDetectAndConvertTypes();

    qDebug() << "Web data loaded:" << table.rowCount() << "rows,"
             << table.columnCount() << "columns";

    return table;
}

DataTable WebConnector::parseHTML(const QString& html)
{
    DataTable table;

    QStringList tables = extractTables(html);

    if (tables.isEmpty()) {
        qWarning() << "No tables found in HTML";
        return table;
    }

    int tableIdx = m_tableIndex;
    if (tableIdx < 0) tableIdx = 0;
    if (tableIdx >= tables.size()) tableIdx = tables.size() - 1;

    QString targetTable = tables[tableIdx];
    qDebug() << "Using table at index:" << tableIdx << "from" << tables.size() << "tables";

    QStringList headers = parseTableHeaders(targetTable);
    if (!headers.isEmpty()) {
        table.setColumnNames(headers);
    } else {
        QStringList defaultHeaders;
        for (int i = 0; i < 10; ++i) {
            defaultHeaders.append(QString("Column%1").arg(i + 1));
        }
        table.setColumnNames(defaultHeaders);
    }

    QStringList rows = parseTableRows(targetTable);

    for (const QString& rowStr : rows) {
        QStringList cells = rowStr.split("\t");
        QList<QVariant> row;
        for (const QString& cell : cells) {
            row.append(cell.trimmed());
        }

        while (row.size() < table.columnCount()) {
            row.append(QString());
        }

        table.addRow(row);
    }

    return table;
}

QStringList WebConnector::extractTables(const QString& html)
{
    QStringList tables;

    QRegularExpression tableStart("<table[^>]*>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression tableEnd("</table>", QRegularExpression::CaseInsensitiveOption);

    int pos = 0;
    while (true) {
        QRegularExpressionMatch matchStart = tableStart.match(html, pos);
        if (!matchStart.hasMatch()) break;

        int tableStartPos = matchStart.capturedStart();

        int tableEndPos = html.indexOf(tableEnd, tableStartPos);
        if (tableEndPos == -1) break;

        QString tableContent = html.mid(tableStartPos, tableEndPos - tableStartPos + 8);
        tables.append(tableContent);

        pos = tableEndPos + 8;
    }

    qDebug() << "Found" << tables.size() << "tables";
    return tables;
}

QStringList WebConnector::parseTableHeaders(const QString& tableHtml)
{
    QStringList headers;

    QRegularExpression thRegex("<th[^>]*>([^<]*)</th>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = thRegex.globalMatch(tableHtml);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString headerText = match.captured(1).trimmed();
        headerText = headerText.remove(QRegularExpression("<[^>]*>"));
        headers.append(headerText);
    }

    if (headers.isEmpty()) {
        QRegularExpression tdRegex("<td[^>]*>([^<]*)</td>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it2 = tdRegex.globalMatch(tableHtml);

        while (it2.hasNext() && headers.size() < 5) {
            QRegularExpressionMatch match = it2.next();
            QString headerText = match.captured(1).trimmed();
            headerText = headerText.remove(QRegularExpression("<[^>]*>"));
            headers.append(headerText);
        }
    }

    return headers;
}

QStringList WebConnector::parseTableRows(const QString& tableHtml)
{
    QStringList rows;

    QRegularExpression trRegex("<tr[^>]*>(.*?)</tr>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = trRegex.globalMatch(tableHtml);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString trContent = match.captured(1);

        QStringList cells;
        QRegularExpression tdRegex("<t[dh][^>]*>([^<]*)</t[dh]>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator cellIt = tdRegex.globalMatch(trContent);

        while (cellIt.hasNext()) {
            QRegularExpressionMatch cellMatch = cellIt.next();
            QString cellText = cellMatch.captured(1).trimmed();
            cellText = cellText.remove(QRegularExpression("<[^>]*>"));
            cells.append(cellText);
        }

        if (!cells.isEmpty()) {
            rows.append(cells.join("\t"));
        }
    }

    return rows;
}

bool WebConnector::write(const DataTable& data)
{
    qWarning() << "Web connector does not support writing";
    return false;
}