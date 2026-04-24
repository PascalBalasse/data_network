#include "NetworkController.h"
#include "../network/Network.h"
#include "../ui/GraphView.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QSaveFile>
#include <QFile>
#include <QDebug>

using namespace dn::nodes;
using namespace dn::network;
using namespace dn::transformations;
using namespace dn::connectors;

namespace dn::ui {

NetworkController::NetworkController(QObject* parent)
    : QObject(parent)
    , m_network(std::make_unique<Network>(this))
{
    connect(m_network.get(), &Network::nodeAdded, this, &NetworkController::nodeAdded);
    connect(m_network.get(), &Network::nodeRemoved, this, &NetworkController::nodeRemoved);
    connect(m_network.get(), &Network::networkChanged, this, &NetworkController::networkChanged);
}

NetworkController::~NetworkController() = default;

void NetworkController::queueOperation(Operation operation)
{
    m_operationQueue.enqueue(operation);
    emit operationQueued();
    processNextOperation();
}

void NetworkController::processNextOperation()
{
    if (m_isProcessing || m_operationQueue.isEmpty())
        return;

    m_isProcessing = true;
    emit operationStarted();

    Operation operation = m_operationQueue.dequeue();
    operation();
}

void NetworkController::completeCurrentOperation(bool success, const QString& message)
{
    finalizeOperation(success);
    emit operationCompleted(success, message);
}

void NetworkController::finalizeOperation(bool success)
{
    Q_UNUSED(success)
    m_isProcessing = false;
    processNextOperation();
}

RuntimeNode* NetworkController::createSourceNode(std::unique_ptr<DataConnector> connector, const QString& name)
{
    auto* node = m_network->createSourceNode(std::move(connector), name);
    m_selectedNode = node;
    emit nodeAdded(node);
    emit nodeSelected(node);
    return node;
}

RuntimeNode* NetworkController::createTargetNode(std::unique_ptr<DataConnector> connector,
                                              RuntimeNode* parent,
                                              const QString& name)
{
    if (!m_selectedNode && !parent) {
        emit selectionNeeded("No node selected for target");
        return nullptr;
    }
    RuntimeNode* source = parent ? parent : m_selectedNode;
    auto* node = m_network->createTargetNode(std::move(connector), source, name);
    m_network->addConnection(source, node);
    m_selectedNode = node;
    emit nodeAdded(node);
    emit nodeSelected(node);
    emit connectionAdded(source, node);
    return node;
}

RuntimeNode* NetworkController::createTransformationNode(
    std::unique_ptr<Transformation> transformation,
    RuntimeNode* parent,
    const QString& name)
{
    if (!m_selectedNode && !parent) {
        emit selectionNeeded("No node selected for transformation");
        return nullptr;
    }
    RuntimeNode* source = parent ? parent : m_selectedNode;
    auto* node = m_network->createTransformationNode(std::move(transformation), source, name);
    m_network->addConnection(source, node);
    m_selectedNode = node;
    emit nodeAdded(node);
    emit nodeSelected(node);
    emit connectionAdded(source, node);
    return node;
}

void NetworkController::setSelectedNode(RuntimeNode* node)
{
    m_selectedNode = node;
    emit nodeSelected(node);
}

void NetworkController::clearNetwork()
{
    auto nodes = m_network->getAllNodes();
    for (RuntimeNode* node : nodes) {
        m_network->removeNode(node);
    }
    m_selectedNode = nullptr;
    m_isProcessing = false;
}

void NetworkController::removeNode(RuntimeNode* node)
{
    if (m_selectedNode == node) {
        m_selectedNode = nullptr;
    }
    m_network->removeNode(node);
}

void NetworkController::removeNodeAndChildren(RuntimeNode* node)
{
    if (!node)
        return;

    QVector<RuntimeNode*> toRemove;
    QQueue<RuntimeNode*> queue;
    queue.enqueue(node);

    while (!queue.isEmpty()) {
        RuntimeNode* current = queue.dequeue();
        if (!toRemove.contains(current)) {
            toRemove.append(current);
            QVector<RuntimeNode*> successors = m_network->getSuccessors(current);
            for (RuntimeNode* succ : successors) {
                queue.enqueue(succ);
            }
        }
    }

    for (RuntimeNode* n : toRemove) {
        if (m_selectedNode == n) {
            m_selectedNode = nullptr;
        }
        m_network->removeNode(n);
    }
}

QVector<RuntimeNode*> NetworkController::getDescendants(RuntimeNode* node) const
{
    QVector<RuntimeNode*> descendants;
    if (!node)
        return descendants;

    QQueue<RuntimeNode*> queue;
    queue.enqueue(node);
    QSet<QUuid> visited;

    while (!queue.isEmpty()) {
        RuntimeNode* current = queue.dequeue();
        if (visited.contains(current->getId()))
            continue;
        visited.insert(current->getId());

        QVector<RuntimeNode*> successors = m_network->getSuccessors(current);
        for (RuntimeNode* succ : successors) {
            if (!visited.contains(succ->getId())) {
                descendants.append(succ);
                queue.enqueue(succ);
            }
        }
    }

    return descendants;
}

bool NetworkController::saveToFile(const QString& fileName, const QHash<QUuid, QPointF>& nodePositions)
{
    if (!m_network->validate()) {
        return false;
    }

    QJsonObject json = m_network->toJson();

    QJsonArray nodesArray = json["nodes"].toArray();
    for (int i = 0; i < nodesArray.size(); ++i) {
        QJsonObject nodeObj = nodesArray[i].toObject();
        QUuid nodeId(nodeObj["id"].toString());

        if (nodePositions.contains(nodeId)) {
            QPointF pos = nodePositions[nodeId];
            QJsonObject posObj;
            posObj["x"] = pos.x();
            posObj["y"] = pos.y();
            nodeObj["position"] = posObj;
            nodesArray[i] = nodeObj;
        }
    }
    json["nodes"] = nodesArray;

    QSaveFile file(fileName);
    qDebug() << "saveToFile: saving to" << fileName;
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    qDebug() << "saveToFile: written, committing...";
    bool ok = file.commit();
    qDebug() << "saveToFile: done, ok:" << ok;

    return ok;
}

bool NetworkController::loadFromFile(const QString& fileName, QHash<QUuid, QPointF>& nodePositions)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return false;

    clearNetwork();

    QJsonObject json = doc.object();
    if (!m_network->fromJson(json))
        return false;

    QJsonArray nodesArray = json["nodes"].toArray();
    for (const QJsonValue& nodeValue : nodesArray) {
        QJsonObject nodeObj = nodeValue.toObject();
        if (nodeObj.contains("position")) {
            QUuid nodeId(nodeObj["id"].toString());
            QJsonObject posObj = nodeObj["position"].toObject();
            QPointF position(posObj["x"].toDouble(), posObj["y"].toDouble());
            nodePositions[nodeId] = position;
        }
    }

    QVector<RuntimeNode*> roots = m_network->findRootNodes();
    if (!roots.isEmpty()) {
        m_selectedNode = roots.first();
    }

    return true;
}

bool NetworkController::validate() const
{
    return m_network->validate();
}

}