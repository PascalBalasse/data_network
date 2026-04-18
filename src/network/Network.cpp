#include "Network.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QDebug>
#include <queue>

using namespace dn::network;
using namespace dn::transformations;

Network::Network(QObject *parent)
    : QObject(parent)
{
}

Network::~Network()
{
    // Nettoyer tous les nœuds
    qDeleteAll(m_nodes);
    m_nodes.clear();
    m_nodeMap.clear();
}

// ========== CRÉATION DE NŒUDS ==========

SourceNode* Network::createSourceNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                                      const QString& name)
{
    auto* node = new SourceNode(std::move(connector), name, this);
    node->compute();
    m_nodes.append(node);
    m_nodeMap[node->getId()] = node;

    return node;
}

TransformationNode* Network::createTransformationNode(
    dn::transformations::Transformation* transform,
    RuntimeNode* parent,
    const QString& name)
{
    auto* node = new TransformationNode(transform, parent, name, this);
    node->compute();
    m_nodes.append(node);
    m_nodeMap[node->getId()] = node;

    return node;
}

MergeNode* Network::createMergeNode(
    const QVector<RuntimeNode*>& parents,
    MergeType type,
    const QStringList& joinKeys,
    const QString& name)
{
    auto* node = new MergeNode(parents, type, joinKeys, name, this);
    node->compute();
    m_nodes.append(node);
    m_nodeMap[node->getId()] = node;

    return node;
}

TargetNode* Network::createTargetNode(
    std::unique_ptr<dn::connectors::DataConnector> connector,
    RuntimeNode* parent,
    const QString& name)
{
    auto* node = new TargetNode(std::move(connector), parent, name, this);
    node->compute();
    m_nodes.append(node);
    m_nodeMap[node->getId()] = node;

    return node;
}

void Network::removeNode(RuntimeNode* node)
{
    if (!node)
        return;

    // Déconnecter tous les nœuds qui pointent vers celui-ci
    for (RuntimeNode* other : m_nodes) {
        for (int i = 0; i < other->getInputsCount(); ++i) {
            if (other->getInput(i) == node) {
                other->setInput(i, nullptr);
                //emit connectionRemoved(other, i);
            }
        }
    }

    // Supprimer le nœud
    m_nodeMap.remove(node->getId());
    m_nodes.removeAll(node);

    emit nodeRemoved(node);
    emit networkChanged();

    node->deleteLater();
}

RuntimeNode* Network::findNode(QUuid id) const
{
    return m_nodeMap.value(id, nullptr);
}


bool Network::disconnectNodes(RuntimeNode* target, int inputSlot)
{
    if (!target)
        return false;

    if (inputSlot < 0 || inputSlot >= target->getInputsCount())
        return false;

    if (target->getInput(inputSlot)) {
        target->setInput(inputSlot, nullptr);
       // emit connectionRemoved(target, inputSlot);
        emit networkChanged();
        return true;
    }

    return false;
}

void Network::computeAll()
{
    QVector<RuntimeNode*> order = computeTopologicalOrder();

    qDebug() << "Computing network in order:" << order.size() << "nodes";

    for (RuntimeNode* node : order) {
        node->compute();
    }
}

void Network::computeFrom(RuntimeNode* startNode)
{
    if (!startNode)
        return;

    // Collecter tous les nœuds dépendants (downstream)
    QSet<RuntimeNode*> toCompute;
    std::queue<RuntimeNode*> queue;
    queue.push(startNode);
    toCompute.insert(startNode);

    while (!queue.empty()) {
        RuntimeNode* current = queue.front();
        queue.pop();

        // Trouver tous les nœuds qui dépendent de current
        for (RuntimeNode* node : m_nodes) {
            for (int i = 0; i < node->getInputsCount(); ++i) {
                if (node->getInput(i) == current && !toCompute.contains(node)) {
                    toCompute.insert(node);
                    queue.push(node);
                }
            }
        }
    }

    // Calculer dans l'ordre topologique
    QVector<RuntimeNode*> order = computeTopologicalOrder();
    for (RuntimeNode* node : order) {
        if (toCompute.contains(node)) {
            node->compute();
        }
    }
}

void Network::computeNode(RuntimeNode* node)
{
    if (node) {
        node->compute();
    }
}

QVector<RuntimeNode*> Network::findRootNodes() const
{
    QVector<RuntimeNode*> roots;

    for (RuntimeNode* node : m_nodes) {
        // Un nœud racine n'a aucune entrée connectée
        bool hasInput = false;
        for (int i = 0; i < node->getInputsCount(); ++i) {
            if (node->getInput(i) != nullptr) {
                hasInput = true;
                break;
            }
        }
        if (!hasInput) {
            roots.append(node);
        }
    }

    return roots;
}

QVector<RuntimeNode*> Network::findLeafNodes() const
{
    QVector<RuntimeNode*> leaves;

    for (RuntimeNode* node : m_nodes) {
        // Vérifier si ce nœud est utilisé comme entrée par d'autres
        bool isUsed = false;
        for (RuntimeNode* other : m_nodes) {
            for (int i = 0; i < other->getInputsCount(); ++i) {
                if (other->getInput(i) == node) {
                    isUsed = true;
                    break;
                }
            }
        }
        if (!isUsed) {
            leaves.append(node);
        }
    }

    return leaves;
}

bool Network::validate() const
{
    // Vérifier les cycles
    if (hasCycle()) {
        qWarning() << "Network validation failed: cycle detected";
        return false;
    }

    // Vérifier que tous les nœuds ont leurs entrées valides
    for (RuntimeNode* node : m_nodes) {
        for (int i = 0; i < node->inputCount(); ++i) {
            RuntimeNode* input = node->getInput(i);
            if (!input && node->inputCount() > 0) {
                // Pour un MergeNode, on autorise les entrées nulles
                if (dynamic_cast<MergeNode*>(node)) {
                    continue; // MergeNode peut avoir des entrées non connectées
                }
                qWarning() << "Node" << node->getName() << "has missing input at slot" << i;
                return false;
            }
        }
    }

    return true;
}

QVector<RuntimeNode*> Network::computeTopologicalOrder() const
{
    QVector<RuntimeNode*> order;
    QSet<RuntimeNode*> visited;

    for (RuntimeNode* node : m_nodes) {
        if (!visited.contains(node)) {
            collectDependencies(node, visited, order);
        }
    }

    return order;
}

void Network::collectDependencies(RuntimeNode* node, QSet<RuntimeNode*>& visited,
                                  QVector<RuntimeNode*>& order) const
{
    visited.insert(node);

    // Collecter d'abord toutes les dépendances (entrées)
    for (int i = 0; i < node->getInputsCount(); ++i) {
        RuntimeNode* input = node->getInput(i);
        if (input && !visited.contains(input)) {
            collectDependencies(input, visited, order);
        }
    }

    order.append(node);
}

bool Network::hasCycle() const
{
    QSet<RuntimeNode*> visited;
    QSet<RuntimeNode*> recursionStack;

    for (RuntimeNode* node : m_nodes) {
        if (hasCycleUtil(node, visited, recursionStack)) {
            return true;
        }
    }
    return false;
}

bool Network::hasCycleUtil(RuntimeNode* node, QSet<RuntimeNode*>& visited,
                           QSet<RuntimeNode*>& recursionStack) const
{
    if (!node)
        return false;

    if (recursionStack.contains(node))
        return true;

    if (visited.contains(node))
        return false;

    visited.insert(node);
    recursionStack.insert(node);

    for (int i = 0; i < node->getInputsCount(); ++i) {
        RuntimeNode* input = node->getInput(i);
        if (input && hasCycleUtil(input, visited, recursionStack)) {
            return true;
        }
    }

    recursionStack.remove(node);
    return false;
}

QJsonObject Network::toJson() const
{
    QJsonObject json;
    json["version"] = "1.0";

    QJsonArray nodesArray;
    QHash<const RuntimeNode*, int> nodeIndex;

    // Sauvegarder les nœuds
    int index = 0;
    for (RuntimeNode* node : m_nodes) {
        QJsonObject nodeObj;
        nodeObj["id"] = node->getId().toString();
        nodeObj["name"] = node->getName();
        nodeObj["type"] = static_cast<int>(node->getNodeType());

        // Sauvegarder les paramètres
        QJsonObject paramsObj;
        for (auto it = node->getAllParameters().begin();
             it != node->getAllParameters().end(); ++it) {
            paramsObj[it.key()] = it.value();
        }
        nodeObj["parameters"] = paramsObj;

        nodesArray.append(nodeObj);
        nodeIndex[node] = index++;
    }

    json["nodes"] = nodesArray;

    // Sauvegarder les connexions
    QJsonArray connectionsArray;
    for (RuntimeNode* node : m_nodes) {
        for (int i = 0; i < node->getInputsCount(); ++i) {
            RuntimeNode* input = node->getInput(i);
            if (input && nodeIndex.contains(input)) {
                QJsonObject connObj;
                connObj["from"] = nodeIndex[input];
                connObj["to"] = nodeIndex[node];
                connObj["slot"] = i;
                connectionsArray.append(connObj);
            }
        }
    }
    json["connections"] = connectionsArray;

    return json;
}

bool Network::fromJson(const QJsonObject& json)
{

    return true;
}

RuntimeNode* Network::createNodeFromJson(NodeType type,
                                         const QString& name,
                                         const QJsonObject& params,
                                         const QPointF& position)
{
    RuntimeNode* node = nullptr;

    return node;
}


void Network::debugPrint() const
{
    qDebug() << "========================================";
    qDebug() << "NETWORK DEBUG (" << m_nodes.size() << "nodes)";
    qDebug() << "========================================";

    for (RuntimeNode* node : m_nodes) {
        qDebug() << "Node:" << node->getName()
        << "(ID:" << node->getId().toString() << ")";
        qDebug() << "  Type:" << static_cast<int>(node->getNodeType());
        qDebug() << "  Inputs:" << node->getInputsCount();

        for (int i = 0; i < node->getInputsCount(); ++i) {
            RuntimeNode* input = node->getInput(i);
            if (input) {
                qDebug() << "    Input[" << i << "]:" << input->getName();
            } else {
                qDebug() << "    Input[" << i << "]: <null>";
            }
        }

        qDebug() << "  Parameters:" << node->getAllParameters();
        qDebug() << "  Result rows:" << node->getCachedResult().rowCount();
        qDebug() << "---";
    }

    qDebug() << "========================================";
}

