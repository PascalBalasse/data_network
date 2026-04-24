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

#include <QDate>
#include <QRegularExpression>

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
    m_columnTypes = types;
    // Note: ne convertit pas les valeurs existantes
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
    // Index invalide -> String par défaut
    if (colIndex < 0 || colIndex >= columnCount())
        return ColumnType::String;

    // Drapeaux pour chaque type (si tous les valeurs correspondent)
    bool allIntegers = true;
    bool allDoubles = true;
    bool allBooleans = true;
    bool allDates = true;
    bool allDateTimes = true;
    int nonEmptyCount = 0;

    // Parcourt toutes les lignes pour tester chaque type
    for (int row = 0; row < rowCount(); ++row) {
        QVariant value = this->value(row, colIndex);
        QString strValue = value.toString().trimmed();

        if (strValue.isEmpty()) {
            continue;  // Ignore les valeurs vides pour la détection
        }

        nonEmptyCount++;

        // Vérifier DateTime (le plus restrictif, testé en premier)
        if (allDateTimes) {
            QDateTime dt = QDateTime::fromString(strValue, Qt::ISODate);
            if (!dt.isValid()) {
                dt = QDateTime::fromString(strValue, "dd/MM/yyyy hh:mm:ss");
            }
            if (!dt.isValid()) {
                allDateTimes = false;
            }
        }

        // Vérifier Date (seulement si ce n'est pas un DateTime)
        if (allDates && !allDateTimes) {
            QDate d = QDate::fromString(strValue, Qt::ISODate);
            if (!d.isValid()) {
                d = QDate::fromString(strValue, "dd/MM/yyyy");
            }
            if (!d.isValid()) {
                allDates = false;
            }
        }

        // Vérifier Boolean (si pas date)
        if (allBooleans && !allDates && !allDateTimes) {
            QString lower = strValue.toLower();
            if (lower != "true" && lower != "false" &&
                lower != "yes" && lower != "no" &&
                lower != "1" && lower != "0") {
                allBooleans = false;
            }
        }

        // Vérifier Integer (si pas date, bool)
        if (allIntegers && !allDates && !allDateTimes && !allBooleans) {
            bool isInt;
            int intVal = strValue.toInt(&isInt);
            // Vérifie strict: "123" OK, "123.0" NON
            if (!isInt || QString::number(intVal) != strValue) {
                allIntegers = false;
            }
        }

        // Vérifier Double (si pas integer)
        if (allDoubles && !allIntegers && !allDates && !allDateTimes) {
            bool isDouble;
            double doubleVal = strValue.toDouble(&isDouble);
            if (!isDouble || QString::number(doubleVal, 'f', 10).startsWith(strValue)) {
                allDoubles = false;
            }
        }
    }

    // Colonne vide -> String par défaut
    if (nonEmptyCount == 0) {
        return ColumnType::String;
    }

    // Retourne le type le plus spécifique trouvé (hiérarchie: Int < Double < String)
    if (allIntegers) return ColumnType::Integer;
    if (allDoubles) return ColumnType::Double;
    if (allBooleans) return ColumnType::Boolean;
    if (allDateTimes) return ColumnType::DateTime;
    if (allDates) return ColumnType::Date;

    return ColumnType::String;
}

void DataTable::autoDetectAndConvertTypes()
{
    // Détecte et convertit chaque colonne independamment
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
        // Ajouter le nom de la colonne
        QStringList newColumnNames = m_columnNames;
        newColumnNames.append(columnName);
        setColumnNames(newColumnNames);
        
        // Ajouter le type de colonne
        QList<ColumnType> newColumnTypes = m_columnTypes;
        newColumnTypes.append(type);
        setColumnTypes(newColumnTypes);
        
        return true;
    }
    
    // Parser l'expression pour extraire les références de colonnes
    QSet<int> referencedColumns;
    QString parsedExpression = expression;
    QRegularExpression re("\\[([^\\]]+)\\]");  // Match [NomColonne]
    QRegularExpressionMatchIterator it = re.globalMatch(expression);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString colName = match.captured(1);
        int colIdx = columnIndex(colName);
        if (colIdx == -1) {
            // Référence à une colonne inexistante
            return false;
        }
        referencedColumns.insert(colIdx);
        // Remplacer la référence par un marqueur pour l'évaluation
        parsedExpression.replace(match.capturedStart(), match.capturedLength(), 
                                QString("COL_%1").arg(colIdx));
    }
    
    // Préparer l'expression pour l'évaluation en remplaçant les marqueurs
    // Par exemple: "[Col1] + [Col2]" devient "value(COL_0) + value(COL_1)"
    QString evalExpression = parsedExpression;
    QRegularExpression colMarkerRe("COL_(\\d+)");
    QRegularExpressionMatchIterator markerIt = colMarkerRe.globalMatch(parsedExpression);
    
    while (markerIt.hasNext()) {
        QRegularExpressionMatch match = markerIt.next();
        int colIdx = match.captured(1).toInt();
        evalExpression.replace(match.capturedStart(), match.capturedLength(),
                              QString("value(%1)").arg(colIdx));
    }
    
    // Calculer les valeurs pour chaque ligne
    QList<QVariant> newColumnValues;
    newColumnValues.reserve(rowCount());
    
    for (int row = 0; row < rowCount(); ++row) {
        // Créer une fonction lambda qui capture la ligne courante
        auto valueAt = [this, row](int colIndex) -> QVariant {
            return this->value(row, colIndex);
        };
        
        // Remplacer les appels value() dans l'expression par les valeurs réelles
        QString rowExpression = evalExpression;
        QRegularExpression valueRe("value\\((\\d+)\\)");
        QRegularExpressionMatchIterator valueIt = valueRe.globalMatch(evalExpression);
        
        while (valueIt.hasNext()) {
            QRegularExpressionMatch match = valueIt.next();
            int colIdx = match.captured(1).toInt();
            QVariant cellValue = valueAt(colIdx);
            
            // Convertir la valeur en représentation appropriée pour l'évaluation
            QString valueStr;
            if (cellValue.isNull()) {
                valueStr = "null";
            } else {
                switch (cellValue.type()) {
                    case QMetaType::Bool:
                        valueStr = cellValue.toBool() ? "true" : "false";
                        break;
                    case QMetaType::Int:
                        valueStr = QString::number(cellValue.toInt());
                        break;
                    case QMetaType::Double:
                        valueStr = QString::number(cellValue.toDouble());
                        break;
                    case QMetaType::QString:
                        valueStr = QString("\"%1\"").arg(cellValue.toString().replace('\"', "\\\""));
                        break;
                    case QMetaType::QDate:
                        valueStr = QString("\"%1\"").arg(cellValue.toDate().toString(Qt::ISODate));
                        break;
                    case QMetaType::QDateTime:
                        valueStr = QString("\"%1\"").arg(cellValue.toDateTime().toString(Qt::ISODate));
                        break;
                    default:
                        // Pour les autres types, convertir en chaîne
                        valueStr = QString("\"%1\"").arg(cellValue.toString().replace('\"', "\\\""));
                        break;
                }
            }
            
            rowExpression.replace(match.capturedStart(), match.capturedLength(), valueStr);
        }
        
        // Évaluer l'expression (implémentation simplifiée)
        // Dans une implémentation réelle, on utiliserait un parseur d'expressions comme QJSEngine
        // ou une bibliothèque comme muParser
        QVariant result = this->evaluateExpression(rowExpression);
        newColumnValues.append(result);
    }
    
    // Ajouter la nouvelle colonne au tableau
    // D'abord ajouter les noms et types
    QStringList newColumnNames = m_columnNames;
    newColumnNames.append(columnName);
    setColumnNames(newColumnNames);
    
    QList<ColumnType> newColumnTypes = m_columnTypes;
    newColumnTypes.append(type);
    setColumnTypes(newColumnTypes);
    
    // Ensuite ajouter les valeurs à chaque ligne
    for (int row = 0; row < rowCount(); ++row) {
        m_data[row].append(newColumnValues[row]);
    }
    
    // Convertir les valeurs vers le type spécifié
    int newColIdx = columnCount() - 1;
    convertColumnValues(newColIdx, type);
    
    return true;
}

// Méthode d'évaluation d'expression simplifiée
// Dans une version production, on utiliserait une bibliothèque d'évaluation d'expressions appropriée
QVariant DataTable::evaluateExpression(const QString& expression) const
{
    // Implémentation très basique pour démonstration
    // Une vraie implémentation aurait besoin d'un parseur d'expressions complet
    
    // Gestion des littéraux nuls
    if (expression.trimmed() == "null") {
        return QVariant();
    }
    
    // Gestion des booléens
    if (expression.trimmed() == "true") {
        return true;
    }
    if (expression.trimmed() == "false") {
        return false;
    }
    
    // Gestion des nombres entiers
    bool ok;
    int intValue = expression.toInt(&ok);
    if (ok && !expression.contains('.') && !expression.contains('/') && !expression.contains(':')) {
        return intValue;
    }
    
    // Gestion des nombres décimaux
    double doubleValue = expression.toDouble(&ok);
    if (ok) {
        return doubleValue;
    }
    
    // Gestion des chaînes (enlever les guillemets)
    if (expression.startsWith('\"') && expression.endsWith('\"') && expression.length() >= 2) {
        QString strValue = expression.mid(1, expression.length() - 2);
        strValue.replace("\\\"", "\"");
        return strValue;
    }
    
    // Si on arrive ici, l'expression n'a pas pu être évaluée
    // Dans une vraie implémentation, on lancerait une exception ou retournerait un d'erreur
    return QVariant(); // Retourner une valeur nulle indiquant une erreur
}
