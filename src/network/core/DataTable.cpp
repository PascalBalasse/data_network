/**
 * @file DataTable.cpp
 * @brief Implémentation des méthodes de DataTable
 *
 * Détails d'implémentation :
 * - Détection automatique des types par analyse des valeurs
 * - Conversion de chaînes vers différents types (Int, Double, Bool, Date, DateTime)
 * - Gestion des types complexes via QMetaType
 *
 * @see DataTable.h pour la documentation de l'API publique
 */

#include "DataTable.h"
#include "ExpressionEvaluator.h"
#include <QRegularExpression>
#include <QDateTime>

using namespace dn::core;

int DataTable::rowCount() const
{
    return m_data.size();
}

int DataTable::columnCount() const
{
    return m_columnNames.size();
}

QVariant DataTable::value(int row, int col) const
{
    // Vérification des bounds pour éviter les crashes
    if (row < 0 || row >= m_data.size())
        return {};

    const auto& r = m_data.at(row);
    if (col < 0 || col >= r.size())
        return {};

    return r.at(col);
}


int DataTable::columnIndex(const QString& name) const
{
    // Recherche linéaire du nom (pour petits tableaux, sinon utiliser QHash)
    return m_columnNames.indexOf(name);
}

QString DataTable::columnName(int col) const
{
    if (col < 0 || col >= m_columnNames.size())
        return {};

    return m_columnNames.at(col);
}



void DataTable::setColumnNames(const QStringList &names)
{
    m_columnNames = names;

    // Initialise les types à String par défaut pour chaque colonne
    m_columnTypes.clear();
    for (int i = 0; i < names.size(); ++i) {
        m_columnTypes.append(ColumnType::String);
    }
}

void DataTable::setColumnTypes(const QList<ColumnType> &types)
{
    if (types.size() != m_columnNames.size()) {
        qWarning() << "setColumnTypes: size mismatch" << types.size() << "!=" << m_columnNames.size();
        qWarning() << "  Column names:" << m_columnNames;
        return;
    }
    m_columnTypes = types;
}

void DataTable::addRow(const QList<QVariant> &row)
{
    // Ajout par copie (plus simple mais moins performant)
    m_data.append(row);
}

void DataTable::addRow(QList<QVariant> &&row)
{
    // Ajout par déplacement (move semantics pour éviter la copie)
    m_data.append(std::move(row));
}

void DataTable::clear()
{
    // Réinitialisation complète : utile pour réutiliser l'objet
    m_data.clear();
    m_columnNames.clear();
    m_columnTypes.clear();
}

ColumnType DataTable::getColumnType(int colIndex) const
{
    // Retourne String par défaut si index invalide
    if (colIndex < 0 || colIndex >= m_columnTypes.size())
        return ColumnType::String;
    return m_columnTypes[colIndex];
}

void DataTable::setColumnType(int colIndex, ColumnType type)
{
    // Définit le type sans convertir les valeurs existantes
    if (colIndex >= 0 && colIndex < m_columnTypes.size()) {
        m_columnTypes[colIndex] = type;
    }
}

int DataTable::countEmptyRows(int colIndex) const
{
    // Compte les valeurs nulles ou vides (après trim)
    int count = 0;
    for (int row = 0; row < rowCount(); ++row) {
        QVariant value = this->value(row, colIndex);
        if (value.isNull() || value.toString().trimmed().isEmpty()) {
            count++;
        }
    }
    return count;
}

QVariant DataTable::convertToType(const QVariant& value, ColumnType targetType) const
{
    // Valeur nulle ou chaîne vide -> retourne nul
    if (value.isNull()) {
        return QVariant();
    }

    QString strValue = value.toString().trimmed();
    if (strValue.isEmpty()) {
        return QVariant();
    }

    switch (targetType) {
    case ColumnType::Integer: {
        // Tentative de conversion directe en entier
        bool ok;
        int intValue = strValue.toInt(&ok);
        if (ok) return intValue;

        // Sinon essaie depuis double puis truncate
        double doubleValue = strValue.toDouble(&ok);
        if (ok) return static_cast<int>(doubleValue);

        return QVariant();  // Échec
    }

    case ColumnType::Double: {
        bool ok;
        double doubleValue = strValue.toDouble(&ok);
        if (ok) return doubleValue;
        return QVariant();
    }

    case ColumnType::Boolean: {
        // Accepte true/yes/1 et false/no/0 (case-insensitive)
        if (strValue.compare("true", Qt::CaseInsensitive) == 0 ||
            strValue.compare("yes", Qt::CaseInsensitive) == 0 ||
            strValue.compare("1", Qt::CaseInsensitive) == 0) {
            return true;
        }
        if (strValue.compare("false", Qt::CaseInsensitive) == 0 ||
            strValue.compare("no", Qt::CaseInsensitive) == 0 ||
            strValue.compare("0", Qt::CaseInsensitive) == 0) {
            return false;
        }
        return QVariant();
    }

    case ColumnType::Date: {
        // Essaye plusieurs formats de date courants
        QDate date = QDate::fromString(strValue, Qt::ISODate);
        if (date.isValid()) return date;

        date = QDate::fromString(strValue, "dd/MM/yyyy");
        if (date.isValid()) return date;

        date = QDate::fromString(strValue, "MM/dd/yyyy");
        if (date.isValid()) return date;

        return QVariant();
    }

    case ColumnType::DateTime: {
        // ISO format puis format français
        QDateTime datetime = QDateTime::fromString(strValue, Qt::ISODate);
        if (datetime.isValid()) return datetime;

        datetime = QDateTime::fromString(strValue, "dd/MM/yyyy hh:mm:ss");
        if (datetime.isValid()) return datetime;

        return QVariant();
    }

    default:
        // Pour les types non convertibles (String, List, Pair, Table)
        return value;
    }
}

ColumnType DataTable::detectColumnType(int colIndex) const
{
    if (colIndex < 0 || colIndex >= columnCount())
        return ColumnType::String;

    bool allIntegers = true;
    bool allDoubles = true;
    bool allBooleans = true;
    bool allDates = true;
    bool allDateTimes = true;
    int nonEmptyCount = 0;

    for (int row = 0; row < rowCount(); ++row) {
        QVariant value = this->value(row, colIndex);
        QString strValue = value.toString().trimmed();

        if (strValue.isEmpty())
            continue;

        nonEmptyCount++;

        if (allDateTimes) {
            QDateTime dt = QDateTime::fromString(strValue, Qt::ISODate);
            if (!dt.isValid())
                dt = QDateTime::fromString(strValue, "dd/MM/yyyy hh:mm:ss");
            if (!dt.isValid())
                allDateTimes = false;
        }

        if (allDates && !allDateTimes) {
            QDate d = QDate::fromString(strValue, Qt::ISODate);
            if (!d.isValid())
                d = QDate::fromString(strValue, "dd/MM/yyyy");
            if (!d.isValid())
                allDates = false;
        }

        if (allBooleans && !allDates && !allDateTimes) {
            QString lower = strValue.toLower();
            if (lower != "true" && lower != "false" &&
                lower != "yes" && lower != "no" &&
                lower != "1" && lower != "0")
                allBooleans = false;
        }

        if (allIntegers && !allDates && !allDateTimes && !allBooleans) {
            bool ok;
            int intVal = strValue.toInt(&ok);
            if (!ok) {
                allIntegers = false;
            } else {
                QString roundTrip = QString::number(intVal);
                if (roundTrip != strValue)
                    allIntegers = false;
            }
        }

        if (allDoubles && !allIntegers && !allDates && !allDateTimes) {
            bool ok;
            strValue.toDouble(&ok);
            if (!ok)
                allDoubles = false;
        }
    }

    if (nonEmptyCount == 0)
        return ColumnType::String;

    if (allDateTimes) return ColumnType::DateTime;
    if (allDates) return ColumnType::Date;
    if (allBooleans) return ColumnType::Boolean;
    if (allIntegers) return ColumnType::Integer;
    if (allDoubles) return ColumnType::Double;
    return ColumnType::String;
}

void DataTable::autoDetectAndConvertTypes()
{
    for (int col = 0; col < columnCount(); ++col) {
        ColumnType detectedType = detectColumnType(col);
        if (detectedType != ColumnType::String) {
            setColumnType(col, detectedType);
            convertColumnValues(col, detectedType);
        } else {
            setColumnType(col, ColumnType::String);
        }
    }
}

void DataTable::forceColumnType(int colIndex, ColumnType type, bool convertValues)
{
    if (colIndex < 0 || colIndex >= columnCount())
        return;

    setColumnType(colIndex, type);

    if (convertValues) {
        convertColumnValues(colIndex, type);
    }
}

void DataTable::convertColumnValues(int colIndex, ColumnType newType)
{
    // Parcourt et convertit chaque valeur de la colonne
    for (int row = 0; row < rowCount(); ++row) {
        QVariant currentValue = this->value(row, colIndex);
        if (!currentValue.isNull()) {
            QVariant converted = convertToType(currentValue, newType);
            if (converted.isValid()) {
                // Met à jour directement dans les données
                m_data[row][colIndex] = converted;
            }
        }
    }
}

bool DataTable::isComplexType(ColumnType type) const
{
    // List, Pair, Table sont des types hiérarchiques
    return type == ColumnType::List || type == ColumnType::Pair || type == ColumnType::Table;
}

bool DataTable::isListValue(const QVariant& value)
{
    // QVariantList correspond au type ColumnType::List
    return value.typeId() == QMetaType::QVariantList;
}

bool DataTable::isPairValue(const QVariant& value)
{
    // QVariantMap correspond au type ColumnType::Pair
    return value.typeId() == QMetaType::QVariantMap;
}

bool DataTable::isTableValue(const QVariant& value)
{
    // Vérifie le type utilisateur pour DataTable imbriquée
    return value.userType() == qMetaTypeId<DataTable>();
}

//════════════════════════════════════════════════════════════════════════════
// Colonnes calculées
//═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Ajoute une colonne calculée basée sur une expression
 * @param columnName Nom de la nouvelle colonne
 * @param expression Expression utilisant les références de colonnes comme [NomColonne]
 * @param type Type de données de la colonne résultante
 * @return true si succès, false si erreur dans l'expression
 */
bool DataTable::addCalculatedColumn(const QString& columnName,
                                   const QString& expression,
                                   ColumnType type)
{
    // Valider les entrées
    if (columnName.isEmpty() || expression.isEmpty()) {
        return false;
    }
    
    // Vérifier que la colonne n'existe pas déjà
    if (columnIndex(columnName) != -1) {
        return false; // Colonne avec ce nom existe déjà
    }
    
    // Si pas de lignes, juste ajouter la colonne vide
    if (rowCount() == 0) {
        m_columnNames.append(columnName);
        m_columnTypes.append(type);
        return true;
    }
    
    // Parser l'expression pour extraire les références de colonnes
    QMap<QString, int> columnRefs;
    QString jsExpression = expression;
    
    qDebug() << "Parsing expression:" << jsExpression;
    
    int searchPos = 0;
    while (true) {
        int bracketStart = jsExpression.indexOf('[', searchPos);
        if (bracketStart == -1) break;
        int bracketEnd = jsExpression.indexOf(']', bracketStart);
        if (bracketEnd == -1) break;
        
        QString colName = jsExpression.mid(bracketStart + 1, bracketEnd - bracketStart - 1);
        int colIdx = columnIndex(colName);
        if (colIdx == -1) {
            qDebug() << "Column not found:" << colName;
            return false;
        }
        
        QString safeName = colName;
        safeName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
        columnRefs[colName] = colIdx;
        jsExpression.replace(bracketStart, bracketEnd - bracketStart + 1, safeName);
        searchPos = bracketStart + safeName.length();
        qDebug() << "After replacement:" << jsExpression << "searchPos:" << searchPos;
    }
    
    qDebug() << "Final expression for evaluator:" << jsExpression;
    qDebug() << "Column refs:" << columnRefs;
    
    // Créer l'évaluateur et calculer pour chaque ligne
    QList<QVariant> newColumnValues;
    newColumnValues.reserve(rowCount());
    
    qDebug() << "addCalculatedColumn: rowCount=" << rowCount() << "expression=" << expression;
    
    for (int row = 0; row < rowCount(); ++row) {
        ExpressionEvaluator evaluator;
        
        for (auto it = columnRefs.constBegin(); it != columnRefs.constEnd(); ++it) {
            QString safeName = it.key();
            safeName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
            evaluator.setVariable(safeName, value(row, it.value()));
        }
        
        bool ok;
        QVariant result = evaluator.evaluate(jsExpression, &ok);
        if (!ok) {
            qWarning() << "Expression evaluation failed for row" << row << ":" << evaluator.lastError();
            result = QVariant();
        }
        qDebug() << "Row" << row << "result:" << result << "ok:" << ok;
        newColumnValues.append(result);
    }
    
    // Ajouter la nouvelle colonne
    m_columnNames.append(columnName);
    m_columnTypes.append(ColumnType::Integer);
    
    for (int row = 0; row < rowCount(); ++row) {
        m_data[row].append(newColumnValues[row]);
    }
    
    qDebug() << "After appending data, column" << columnName << "values:";
    for (int row = 0; row < rowCount(); ++row) {
        qDebug() << "  Row" << row << ":" << value(row, columnCount() - 1);
    }
    
    ColumnType finalType = detectColumnType(columnCount() - 1);
    m_columnTypes[columnCount() - 1] = finalType;
    
    return true;
}

// Méthode d'évaluation d'expression simplifiée
QVariant DataTable::evaluateExpression(const QString& expression) const
{
    QString expr = expression.trimmed();
    
    // Gestion des littéraux nuls
    if (expr == "null") {
        return QVariant();
    }
    
    // Gestion des booléens
    if (expr == "true") {
        return true;
    }
    if (expr == "false") {
        return false;
    }
    
    // Gestion des chaînes (enlever les guillemets)
    if (expr.startsWith('"') && expr.endsWith('"') && expr.length() >= 2) {
        QString strValue = expr.mid(1, expr.length() - 2);
        strValue.replace("\\\"", "\"");
        return strValue;
    }
    
    // Gestion des nombres entiers
    bool ok;
    int intValue = expr.toInt(&ok);
    if (ok) {
        return intValue;
    }
    
    // Gestion des nombres décimaux
    double doubleValue = expr.toDouble(&ok);
    if (ok) {
        return doubleValue;
    }
    
    // Recherche d'opérateurs arithmétiques avec précédence correcte
    // Règle: * / % ont précédence sur + -
    // Trouver le premier * / % (de gauche à droite)
    
    // Chercher * / % (priorité haute)
    int mulDivPos = -1;
    for (int i = 0; i < expr.length(); ++i) {
        if (expr[i] == '*' || expr[i] == '/' || expr[i] == '%') {
            mulDivPos = i;
            break;
        }
    }
    
    if (mulDivPos > 0) {
        QString leftStr = expr.left(mulDivPos).trimmed();
        // Le côté droit: s'arrêter au premier + ou - (plus basse priorité)
        int rightEnd = expr.length();
        for (int i = mulDivPos + 1; i < expr.length(); ++i) {
            if (expr[i] == '+' || expr[i] == '-') {
                rightEnd = i;
                break;
            }
        }
        QString rightStr = expr.mid(mulDivPos + 1, rightEnd - mulDivPos - 1).trimmed();
        QVariant leftVal = evaluateExpression(leftStr);
        QVariant rightVal = evaluateExpression(rightStr);
        QVariant mulResult = evaluateBinaryOp(leftVal, rightVal, expr[mulDivPos]);
        
        // S'il reste quelque chose après (comme "+6"), l'ajouter au résultat
        if (rightEnd < expr.length()) {
            QString remaining = expr.mid(rightEnd).trimmed();
            if (!remaining.isEmpty()) {
                QVariant remainingVal = evaluateExpression(remaining);
                QChar op = expr[rightEnd];
                return evaluateBinaryOp(mulResult, remainingVal, op);
            }
        }
        return mulResult;
    }
    
    // Pas de * / %, chercher + - (priorité basse)
    int addSubPos = -1;
    for (int i = 0; i < expr.length(); ++i) {
        if (expr[i] == '+' || expr[i] == '-') {
            addSubPos = i;
            break;
        }
    }
    
    if (addSubPos > 0) {
        QString leftStr = expr.left(addSubPos).trimmed();
        QString rightStr = expr.mid(addSubPos + 1).trimmed();
        QVariant leftVal = evaluateExpression(leftStr);
        QVariant rightVal = evaluateExpression(rightStr);
        return evaluateBinaryOp(leftVal, rightVal, expr[addSubPos]);
    }
    
    return QVariant();
}

int DataTable::findOperatorOutsideParens(const QString& expr, QChar op) const
{
    int parenDepth = 0;
    for (int i = 0; i < expr.length(); ++i) {
        QChar c = expr[i];
        if (c == '(') {
            parenDepth++;
        } else if (c == ')') {
            parenDepth = qMax(0, parenDepth - 1);
        } else if (parenDepth == 0 && c == op) {
            return i;
        }
    }
    return -1;
}

QVariant DataTable::evaluateBinaryOp(const QVariant& left, const QVariant& right, QChar op) const
{
    if (left.isNull() || right.isNull()) {
        qWarning() << "evaluateBinaryOp: null operand" << left << right;
        return QVariant();
    }
    
    bool leftOk, rightOk;
    double leftNum = toDouble(left, &leftOk);
    double rightNum = toDouble(right, &rightOk);
    
    if (!leftOk || !rightOk) {
        qWarning() << "evaluateBinaryOp: conversion failed" << left << right << leftOk << rightOk;
        return QVariant();
    }
    
    switch (op.unicode()) {
    case '+':
        if (left.typeId() == QMetaType::Int && right.typeId() == QMetaType::Int) {
            return left.toInt() + right.toInt();
        }
        return leftNum + rightNum;
    case '-':
        if (left.typeId() == QMetaType::Int && right.typeId() == QMetaType::Int) {
            return left.toInt() - right.toInt();
        }
        return leftNum - rightNum;
    case '*':
        if (left.typeId() == QMetaType::Int && right.typeId() == QMetaType::Int) {
            return left.toInt() * right.toInt();
        }
        return leftNum * rightNum;
    case '/':
        if (rightNum == 0.0) {
            return QVariant();
        }
        if (left.typeId() == QMetaType::Int && right.typeId() == QMetaType::Int && left.toInt() % right.toInt() == 0) {
            return left.toInt() / right.toInt();
        }
        return leftNum / rightNum;
    case '%':
        if (left.typeId() == QMetaType::Int && right.typeId() == QMetaType::Int && right.toInt() != 0) {
            return left.toInt() % right.toInt();
        }
        return QVariant();
    default:
        return QVariant();
    }
}

double DataTable::toDouble(const QVariant& value, bool* ok) const
{
    if (value.typeId() == QMetaType::Double) {
        *ok = true;
        return value.toDouble();
    }
    if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
        *ok = true;
        return value.toDouble();
    }
    if (value.typeId() == QMetaType::Bool) {
        *ok = true;
        return value.toBool() ? 1.0 : 0.0;
}
    *ok = false;
    return 0.0;
}
