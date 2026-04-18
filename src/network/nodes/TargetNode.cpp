
/**
 * @file TargetNode.cpp
 * @brief Implémentation du nœud cible
 *
 * Écrit les données vers un connecteur.
 *
 * @see TargetNode.h
 */

#include "TargetNode.h"
#include "../core/enums.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include <QMessageBox>

using namespace dn::nodes;
using namespace dn::core;

TargetNode::TargetNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                       RuntimeNode* parent,
                       const QString& name,
                       QObject *qtParent)
    : RuntimeNode(parent, qtParent)
    , m_connector(std::move(connector))
{
    setNodeType(dn::core::NodeType::Target);
    setName(name);
}

/// Calcule et met à jour le niveau du nœud
void TargetNode::compute(){
    QVector<const dn::core::DataTable*> inputTables=getInputTables();
    setCachedResult(inputTables);

    RuntimeNode* inputNode=m_inputs[0];
    int level=inputNode->getNodeLevel();
    setNodeLevel(level+1);
}

/// Met en cache les données et lance l'export
void TargetNode::setCachedResult(const QVector<const DataTable*>& inputs)
{
    m_cachedInput = *inputs[0];
    exportData();
}

/// Configure le connecteur
void TargetNode::configureConnector(const QMap<QString, QVariant>& params)
{
    m_connector->configure(params);
}

/// Effectue l'export vers le connecteur
bool TargetNode::exportData()
{
    if (!m_connector->supportsWriting()) {
        emit exportCompleted(false, "Ce connector ne supporte pas l'écriture");
        return false;
    }

    if (m_cachedInput.rowCount() == 0) {
        emit exportCompleted(false, "Aucune donnée à exporter");
        return false;
    }

    bool success = m_connector->write(m_cachedInput);

    if (success) {
        emit exportCompleted(true, QString("Export réussi : %1 lignes").arg(m_cachedInput.rowCount()));
    } else {
        emit exportCompleted(false, "Erreur lors de l'export");
    }

    return success;
}