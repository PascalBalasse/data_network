#include "DataTableModel.h"

#include <QDate>
#include <QFlags>

using namespace dn::ui;
using namespace dn::core;

DataTableModel::DataTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void DataTableModel::setTable(const DataTable* table)
{
    beginResetModel();
    m_table = table;
    endResetModel();
}

int DataTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_table)
        return 0;

    return m_table->rowCount();
}

int DataTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_table)
        return 0;

    return m_table->columnCount();
}

QVariant DataTableModel::data(const QModelIndex &index, int role) const
{
    if (!m_table || !index.isValid())
        return {};

    if (role == Qt::DisplayRole) {
        QVariant value = m_table->value(index.row(), index.column());

        // Formater selon le type
        ColumnType type = m_table->getColumnType(index.column());
        switch (type) {
        case ColumnType::Double:
            return QString::number(value.toDouble(), 'f', 2);
        case ColumnType::Date: {
            QDate date = value.toDate();
            if (date.isValid())
                return date.toString("dd/MM/yyyy");
            break;
        }
        case ColumnType::DateTime: {
            QDateTime datetime = value.toDateTime();
            if (datetime.isValid())
                return datetime.toString("dd/MM/yyyy hh:mm:ss");
            break;
        }
        case ColumnType::List:
            return "List";
        case ColumnType::Pair:
            return "Pair";
        case ColumnType::Table:
            return "Table";
        case ColumnType::Any:
            return "Any";
        default:
            break;
        }

        return value;
    }

    if (role == Qt::TextAlignmentRole) {
        ColumnType type = m_table->getColumnType(index.column());
        if (type == ColumnType::Integer || type == ColumnType::Double) {
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);  // ✅ OK
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);  // ✅ OK
    }

    if (role == Qt::ToolTipRole) {
        ColumnType type = m_table->getColumnType(index.column());
        QString typeName;
        switch (type) {
        case ColumnType::Integer: typeName = "Nombre entier"; break;
        case ColumnType::Double: typeName = "Nombre décimal"; break;
        case ColumnType::Boolean: typeName = "Booléen (Vrai/Faux)"; break;
        case ColumnType::Date: typeName = "Date"; break;
        case ColumnType::DateTime: typeName = "Date et heure"; break;
        case ColumnType::String: typeName = "Texte"; break;
        default: typeName = "Type inconnu"; break;
        }
        return QString("Type: %1").arg(typeName);
    }

    return {};
}

QVariant DataTableModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    if (!m_table)
        return {};

    if (orientation == Qt::Horizontal && section < m_table->columnCount()) {
        QString columnName = m_table->columnName(section);
        ColumnType type = m_table->getColumnType(section);

        if (role == Qt::DisplayRole) {
            return columnName;
        }

        if (role == Qt::UserRole) {
            switch (type) {
            case ColumnType::Integer: return "Nombre entier";
            case ColumnType::Double: return "Nombre décimal";
            case ColumnType::Boolean: return "Booléen";
            case ColumnType::Date: return "Date";
            case ColumnType::DateTime: return "Date et heure";
            case ColumnType::List: return "Liste";
            case ColumnType::Pair: return "Paire";
            case ColumnType::Table: return "Table";
            case ColumnType::Any: return "Any";
            default: return "Texte";
            }
        }

        return {};
    }

    if (orientation == Qt::Vertical) {
        if (role == Qt::DisplayRole)
            return section + 1;
        return {};
    }

    return QVariant();
}