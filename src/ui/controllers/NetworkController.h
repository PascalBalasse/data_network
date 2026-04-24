#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H

#include <QObject>
#include <QQueue>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>
#include "../network/Network.h"

namespace dn::ui {

class GraphView;

class NetworkController : public QObject
{
    Q_OBJECT

public:
    explicit NetworkController(QObject* parent = nullptr);
    ~NetworkController();

    void setGraphView(GraphView* gv) { m_graphView = gv; }

    dn::network::Network* network() const { return m_network.get(); }
    dn::nodes::RuntimeNode* selectedNode() const { return m_selectedNode; }
    bool isProcessing() const { return m_isProcessing; }

    dn::nodes::RuntimeNode* createSourceNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                                             const QString& name);
    dn::nodes::RuntimeNode* createTargetNode(std::unique_ptr<dn::connectors::DataConnector> connector,
                                             dn::nodes::RuntimeNode* parent,
                                             const QString& name);
    dn::nodes::RuntimeNode* createTransformationNode(std::unique_ptr<dn::transformations::Transformation> transformation,
                                                   dn::nodes::RuntimeNode* parent,
                                                   const QString& name);

    void setSelectedNode(dn::nodes::RuntimeNode* node);
    void clearNetwork();
    void removeNode(dn::nodes::RuntimeNode* node);
    void removeNodeAndChildren(dn::nodes::RuntimeNode* node);
    QVector<dn::nodes::RuntimeNode*> getDescendants(dn::nodes::RuntimeNode* node) const;

    bool saveToFile(const QString& fileName, const QHash<QUuid, QPointF>& nodePositions);
    bool loadFromFile(const QString& fileName, QHash<QUuid, QPointF>& nodePositions);
    bool validate() const;

    using Operation = std::function<void()>;

signals:
    void operationQueued();
    void operationStarted();
    void operationCompleted(bool success, const QString& message);
    void networkChanged();
    void nodeSelected(dn::nodes::RuntimeNode* node);
    void nodeAdded(dn::nodes::RuntimeNode* node);
    void nodeRemoved(dn::nodes::RuntimeNode* node);
    void connectionAdded(dn::nodes::RuntimeNode* from, dn::nodes::RuntimeNode* to);
    void selectionNeeded(const QString& reason);

public slots:
    void queueOperation(Operation operation);
    void completeCurrentOperation(bool success, const QString& message = QString());

private:
    void processNextOperation();
    void finalizeOperation(bool success);

    std::unique_ptr<dn::network::Network> m_network;
    GraphView* m_graphView = nullptr;
    dn::nodes::RuntimeNode* m_selectedNode = nullptr;
    QQueue<Operation> m_operationQueue;
    bool m_isProcessing = false;
};

}

#endif // NETWORKCONTROLLER_H