/**
 * @file SourceNode.cpp
 * @brief Implémentation du nœud source
 *
 * Charge les données depuis un connecteur et les met en cache.
 *
 * @see SourceNode.h
 */

#include "SourceNode.h"

using namespace dn::nodes;
using namespace dn::connectors;

SourceNode::SourceNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                       const QString& name,
                       QObject *parent)
    : RuntimeNode(parent)
    , m_connector(std::move(connector))
{
    setNodeType(dn::core::NodeType::Source);
    setName(name);
}


void SourceNode::compute() {
    DataTable m_sourceData=loadData();
    QVector<const dn::core::DataTable*> inputTables;
    inputTables.append(&m_sourceData);
    setCachedResult(inputTables);

    setNodeLevel(0);
    emit tableChanged(m_cachedResult);
}


DataTable SourceNode::loadData()
{
    auto dataPtr = m_connector->load();
    if (dataPtr) {
        return *dataPtr;  // Dereference the unique_ptr to get the DataTable
    }
    return dn::core::DataTable();
}

void SourceNode::setCachedResult(const QVector<const dn::core::DataTable*>& inputs){
    m_cachedResult=*inputs[0];
}
