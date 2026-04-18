#include "RuntimeNode.h"

using namespace dn::nodes;

RuntimeNode::RuntimeNode(QObject *parent)
    : QObject(parent)
    , m_id(QUuid::createUuid())
{
}

RuntimeNode::RuntimeNode(RuntimeNode* parent, QObject *qtParent)
    : QObject(qtParent)
    , m_id(QUuid::createUuid())
{
    if (parent) {
        setupParentConnection(parent);
    }
}

RuntimeNode::RuntimeNode(const QVector<RuntimeNode*>& parents, QObject *qtParent)
    : QObject(qtParent)
    , m_id(QUuid::createUuid())
{
    setupParentsConnections(parents);
}

void RuntimeNode::setupParentConnection(RuntimeNode* parent)
{
    m_inputs.resize(1, nullptr);
    setInput(parent);
}

void RuntimeNode::setupParentsConnections(const QVector<RuntimeNode*>& parents)
{
    m_inputs.resize(parents.size(), nullptr);
    for (int i = 0; i < parents.size(); ++i) {
        if (parents[i]) {
            setInput(i, parents[i]);
        }
    }
}

void RuntimeNode::setParameter(const QString& key, const QString& value)
{
    m_params[key] = value;
    emit parameterChanged(key, value);
}

QString RuntimeNode::getParameter(const QString& key) const
{
    return m_params.value(key);
}

void RuntimeNode::setInput(RuntimeNode* node)
{
    setInput(0, node);
}

RuntimeNode* RuntimeNode::getInput() const
{
    return getInput(0);
}

void RuntimeNode::setInput(int index, RuntimeNode* node)
{
    // Déconnecter l'ancien
    if (index < m_inputs.size() && m_inputs[index]) {
        disconnect(m_inputs[index], &RuntimeNode::tableChanged,
                   this, &RuntimeNode::compute);
        emit inputDisconnected(index);
    }

    // Agrandir si nécessaire
    while (m_inputs.size() <= index) {
        m_inputs.append(nullptr);
    }

    // Connecter le nouveau
    m_inputs[index] = node;
    if (node) {
        connect(node, &RuntimeNode::tableChanged,
                this, &RuntimeNode::compute);
        emit inputConnected(index, node);
    }

   // update();
}

RuntimeNode* RuntimeNode::getInput(int index) const
{
    if (index < 0 || index >= m_inputs.size())
        return nullptr;
    return m_inputs[index];
}



QVector<const dn::core::DataTable*> RuntimeNode::getInputTables(){
    QVector<const dn::core::DataTable*> inputTables;

    for (RuntimeNode* input : m_inputs) {
        if (input) {
            inputTables.append(&input->getCachedResult());
        } else {
            inputTables.append(nullptr);
        }
    }
    return inputTables;
}

void RuntimeNode::setNodeLevel(int level){
    m_nodeLevel=level;
};
