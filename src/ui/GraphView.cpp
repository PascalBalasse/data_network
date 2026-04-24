#include "GraphView.h"
#include "../network/nodes/RuntimeNode.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QPainter>
#include <QFontMetrics>
#include <QQueue>
#include <QSet>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>
#include <QAction>
#include <QTimer>
#include <QCoreApplication>
#include <algorithm>


using namespace dn::ui;
using namespace dn::nodes;

//========================= CONSTRUCTEUR ET DESTRUCTEUR ======================================//

GraphView::GraphView(QWidget *parent)
    : QGraphicsView(parent)
    , m_isDragging(false)
{
    // Créer la scène si elle n'existe pas déjà

    QGraphicsScene* newScene = new QGraphicsScene(QRectF(0, 0, 600, 800), this);
    setScene(newScene);

    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    setBackgroundBrush(QBrush(Qt::white));

    // CHANGEMENT : Activer les barres de défilement
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // S'assurer que la vue se met à jour correctement
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // Centrer sur l'origine
    centerOn(0, 0);
}

GraphView::~GraphView()
{
    clearGraph();
}

//========================= GESTION DES NOEUDS ======================================//
// ajout d'un noeud
void GraphView::addNode(const QUuid& id, dn::core::NodeType type, const QString& label)
{
    if (m_nodes.contains(id)) {
        qWarning() << "Node already exists:" << id.toString();
        return;
    }

    // CRITICAL: Vérifier que la scène existe
    if (!scene()) {
        qCritical() << "ERROR: No scene in GraphView! Creating one...";
        QGraphicsScene* newScene = new QGraphicsScene(QRectF(-2000, -2000, 4000, 4000), this);
        setScene(newScene);
    }

    GraphNode node;
    node.id = id;
    node.label = label;
    node.type = type;

    // Position par défaut visible
    int nodeCount = m_nodes.size();
    node.position = QPointF(100 + (nodeCount % 5) * 200, 100 + (nodeCount / 5) * 150);

    QSizeF size = calculateNodeSize(label);

    node.shapeItem = createShapeItem(type, label);
    node.shapeItem->setPos(node.position);
    node.shapeItem->setData(0, id.toString());

    // Vérifier que l'item est visible
    node.shapeItem->setVisible(true);
    node.shapeItem->setZValue(1);

    node.textItem = new QGraphicsTextItem(label, node.shapeItem);
    node.textItem->setDefaultTextColor(Qt::white);
    node.textItem->setFont(QFont("Arial", 10));
    node.textItem->setVisible(true);

    QRectF textRect = node.textItem->boundingRect();
    qreal textX = (size.width() - textRect.width()) / 2;
    qreal textY = (size.height() - textRect.height()) / 2;
    node.textItem->setPos(textX, textY);

    scene()->addItem(node.shapeItem);
    // Forcer la mise à jour immédiate de l'item
    node.shapeItem->update();
    scene()->update();

    m_nodes[id] = node;

    // Forcer la mise à jour de la vue
    viewport()->update();

    // Centrer la vue sur les nœuds
    centerOnNodes();
    autoLayout();
}

void GraphView::addNodeWithConnection(const QUuid& parentId, const QUuid& nodeId, dn::core::NodeType type, const QString& label)
{
    if (m_nodes.contains(nodeId)) {
        qWarning() << "Node already exists:" << nodeId.toString();
        return;
    }

    if (!scene()) {
        QGraphicsScene* newScene = new QGraphicsScene(QRectF(-2000, -2000, 4000, 4000), this);
        setScene(newScene);
    }

    GraphNode node;
    node.id = nodeId;
    node.label = label;
    node.type = type;

    int nodeCount = m_nodes.size();
    node.position = QPointF(100 + (nodeCount % 5) * 200, 100 + (nodeCount / 5) * 150);

    QSizeF size = calculateNodeSize(label);

    node.shapeItem = createShapeItem(type, label);
    node.shapeItem->setPos(node.position);
    node.shapeItem->setData(0, nodeId.toString());
    node.shapeItem->setVisible(true);
    node.shapeItem->setZValue(1);

    node.textItem = new QGraphicsTextItem(label, node.shapeItem);
    node.textItem->setDefaultTextColor(Qt::white);
    node.textItem->setFont(QFont("Arial", 10));
    node.textItem->setVisible(true);

    QRectF textRect = node.textItem->boundingRect();
    qreal textX = (size.width() - textRect.width()) / 2;
    qreal textY = (size.height() - textRect.height()) / 2;
    node.textItem->setPos(textX, textY);

    scene()->addItem(node.shapeItem);
    node.shapeItem->update();
    scene()->update();

    m_nodes[nodeId] = node;
    viewport()->update();

    addConnection(parentId, nodeId);
    autoLayout();
}

QGraphicsItem* GraphView::createShapeItem(dn::core::NodeType type, const QString& label)
{
    QSizeF size = calculateNodeSize(label);
    QGraphicsItem* shape = nullptr;
    QBrush brush;
    QPen pen(Qt::black, 2);

    switch (type) {
    case dn::core::NodeType::Source:
        brush = QBrush(QColor(100, 200, 100));  // Vert
        shape = new QGraphicsEllipseItem(0, 0, size.width(), size.height());
        break;
    case dn::core::NodeType::Transform:
        brush = QBrush(QColor(100, 100, 200));  // Bleu
        shape = new QGraphicsRectItem(0, 0, size.width(), size.height());
        break;
    case dn::core::NodeType::Merge:
        brush = QBrush(QColor(200, 200, 100));  // Jaune
        shape = new QGraphicsRectItem(0, 0, size.width(), size.height());
        break;
    case dn::core::NodeType::Target:
        brush = QBrush(QColor(100, 200, 100));  // Vert
        shape = new QGraphicsEllipseItem(0, 0, size.width(), size.height());
        break;
    default:
        brush = QBrush(QColor(150, 150, 150));
        shape = new QGraphicsRectItem(0, 0, size.width(), size.height());
        break;
    }

    // Cast vers le type approprié pour setPen et setBrush
    if (auto* ellipse = dynamic_cast<QGraphicsEllipseItem*>(shape)) {
        ellipse->setPen(pen);
        ellipse->setBrush(brush);
    } else if (auto* rect = dynamic_cast<QGraphicsRectItem*>(shape)) {
        rect->setPen(pen);
        rect->setBrush(brush);
    }

    shape->setFlags(QGraphicsItem::ItemIsMovable |
                    QGraphicsItem::ItemIsSelectable |
                    QGraphicsItem::ItemSendsGeometryChanges);

    return shape;
}


void GraphView::removeNode(const QUuid& id)
{
    if (!m_nodes.contains(id)) {
        qWarning() << "Node not found:" << id.toString();
        return;
    }

    // Supprimer les connexions
    QVector<QGraphicsLineItem*> connectionsToRemove;
    for (QGraphicsLineItem* conn : m_connections) {
        QString fromStr = conn->data(0).toString();
        QString toStr = conn->data(1).toString();
        if (fromStr == id.toString() || toStr == id.toString()) {
            scene()->removeItem(conn);
            connectionsToRemove.append(conn);
        }
    }

    for (QGraphicsLineItem* conn : connectionsToRemove) {
        m_connections.removeAll(conn);
        delete conn;
    }

    // Supprimer de l'adjacence
    m_adjacencyList.remove(id);
    for (auto& targets : m_adjacencyList) {
        targets.removeAll(id);
    }

    // Supprimer le nœud
    GraphNode& node = m_nodes[id];
    scene()->removeItem(node.shapeItem);
    delete node.shapeItem;
    m_nodes.remove(id);

    if (m_selectedNodeId == id) {
        m_selectedNodeId = QUuid();
        emit nodeSelected(QUuid());
    }

    // Re-layout
    autoLayout();
}

void GraphView::updateNodeLabel(const QUuid& id, const QString& label)
{
    if (!m_nodes.contains(id)) {
        qWarning() << "Node not found:" << id.toString();
        return;
    }

    GraphNode& node = m_nodes[id];
    node.label = label;

    // Recalculer la taille
    QSizeF newSize = calculateNodeSize(label);
    QSizeF oldSize = node.shapeItem->boundingRect().size();

    if (newSize != oldSize) {
        // Recréer l'item avec la nouvelle taille
        QPointF oldPos = node.position;
        QGraphicsItem* newShape = createShapeItem(node.type, label);
        newShape->setPos(oldPos);
        newShape->setData(0, id.toString());

        // Nouveau texte
        QGraphicsTextItem* newText = new QGraphicsTextItem(label, newShape);
        newText->setDefaultTextColor(Qt::white);
        newText->setFont(QFont("Arial", 10));

        QRectF textRect = newText->boundingRect();
        qreal textX = (newSize.width() - textRect.width()) / 2;
        qreal textY = (newSize.height() - textRect.height()) / 2;
        newText->setPos(textX, textY);

        scene()->removeItem(node.shapeItem);
        delete node.shapeItem;

        node.shapeItem = newShape;
        node.textItem = newText;
        scene()->addItem(node.shapeItem);

        autoLayout();
    } else if (node.textItem) {
        node.textItem->setPlainText(label);
        QRectF textRect = node.textItem->boundingRect();
        QSizeF size = node.shapeItem->boundingRect().size();
        qreal textX = (size.width() - textRect.width()) / 2;
        qreal textY = (size.height() - textRect.height()) / 2;
        node.textItem->setPos(textX, textY);
    }
}

void GraphView::addConnection(const QUuid& fromId, const QUuid& toId)
{
    qDebug() << "GraphView::addConnection - from:" << fromId.toString() << "to:" << toId.toString();

    if (!m_nodes.contains(fromId)) {
        qWarning() << "Source node not found:" << fromId.toString();
        return;
    }

    if (!m_nodes.contains(toId)) {
        qWarning() << "Target node not found:" << toId.toString();
        return;
    }

    // Ajouter à la liste d'adjacence (de la source vers la cible)
    if (!m_adjacencyList.contains(fromId)) {
        m_adjacencyList[fromId] = QVector<QUuid>();
    }

    // Éviter les doublons
    if (!m_adjacencyList[fromId].contains(toId)) {
        m_adjacencyList[fromId].append(toId);
        qDebug() << "  Added to adjacency list:" << fromId.toString() << "->" << toId.toString();
    }

    // Debug: afficher toute l'adjacence
    qDebug() << "=== CURRENT ADJACENCY LIST ===";
    for (auto it = m_adjacencyList.begin(); it != m_adjacencyList.end(); ++it) {
        QString fromNode = m_nodes.contains(it.key()) ? m_nodes[it.key()].label : "Unknown";
        qDebug() << "  " << fromNode << "->" << it.value().size() << "children";
        for (const QUuid& child : it.value()) {
            QString childName = m_nodes.contains(child) ? m_nodes[child].label : "Unknown";
            qDebug() << "      " << childName;
        }
    }

    // Dessiner la connexion
    const GraphNode& from = m_nodes[fromId];
    const GraphNode& to = m_nodes[toId];

    qreal fromWidth = from.shapeItem->boundingRect().width();
    qreal fromHeight = from.shapeItem->boundingRect().height();
    qreal toWidth = to.shapeItem->boundingRect().width();

    QPointF fromPoint(
        from.position.x() + fromWidth / 2,
        from.position.y() + fromHeight
        );

    QPointF toPoint(
        to.position.x() + toWidth / 2,
        to.position.y()
        );

    QGraphicsLineItem* line = new QGraphicsLineItem(QLineF(fromPoint, toPoint));
    line->setPen(QPen(Qt::black, 2));
    line->setData(0, fromId.toString());
    line->setData(1, toId.toString());

    scene()->addItem(line);
    m_connections.append(line);

    requestLayout();
}


void GraphView::removeConnection(const QUuid& fromId, const QUuid& toId)
{
    // Supprimer de l'adjacence
    if (m_adjacencyList.contains(fromId)) {
        m_adjacencyList[fromId].removeAll(toId);
    }

    // Supprimer l'item graphique
    QVector<QGraphicsLineItem*> toRemove;
    for (QGraphicsLineItem* line : m_connections) {
        QString fromStr = line->data(0).toString();
        QString toStr = line->data(1).toString();
        if (fromStr == fromId.toString() && toStr == toId.toString()) {
            scene()->removeItem(line);
            toRemove.append(line);
        }
    }

    for (QGraphicsLineItem* line : toRemove) {
        m_connections.removeAll(line);
        delete line;
    }

    requestLayout();
}


//========================= DISPOSITION DES NOEUDS ======================================//

QSizeF GraphView::calculateNodeSize(const QString& label)
{
    QFontMetrics fm(QFont("Arial", 10));
    int textWidth = fm.horizontalAdvance(label);
    int textHeight = fm.height();

    qreal width = qMax(MIN_NODE_WIDTH, textWidth + NODE_PADDING_X * 2);
    qreal height = qMax(MIN_NODE_HEIGHT, textHeight + NODE_PADDING_Y * 2);

    return QSizeF(width, height);
}


void GraphView::clearGraph()
{
    if (scene()) {
        scene()->clear();
    }
    m_nodes.clear();
    m_connections.clear();
    m_adjacencyList.clear();
    m_selectedNodeId = QUuid();
    m_isDragging = false;
}

// ========== LAYOUT INTELLIGENT ==========

void GraphView::autoLayout()
{
    m_layoutPending = false;

    if (m_nodes.isEmpty())
        return;

    qDebug() << "=== AUTO LAYOUT START ===";

    computeNodeLevels();
    assignRowsAndColumns();
    minimizeCrossings();
    applyPositions();
    updateConnectionPositions();
    centerOnNodes();

    qDebug() << "=== AUTO LAYOUT END ===";
}
void GraphView::computeNodeLevels()
{
    // Réinitialiser les niveaux
    for (auto& node : m_nodes) {
        node.level = -1;
        node.row = -1;
        node.col = -1;
    }

    // Trouver tous les nœuds sources (ceux qui n'ont pas de parents)
    // Un parent est un nœud qui a une connexion vers ce nœud
    QVector<QUuid> sources;
    qDebug() << "=== computeNodeLevels - Looking for sources ===";
    qDebug() << "  m_adjacencyList contents:";
    for (auto it = m_adjacencyList.begin(); it != m_adjacencyList.end(); ++it) {
        QString parentName = m_nodes.contains(it.key()) ? m_nodes[it.key()].label : "Unknown";
        qDebug() << "    " << parentName << "->" << it.value().size() << "children:";
        for (const QUuid& childId : it.value()) {
            QString childName = m_nodes.contains(childId) ? m_nodes[childId].label : "Unknown";
            qDebug() << "      ->" << childName << "(" << childId.toString() << ")";
        }
    }

    for (auto& node : m_nodes) {
        bool hasParent = false;
        for (auto it = m_adjacencyList.begin(); it != m_adjacencyList.end(); ++it) {
            if (it.value().contains(node.id)) {
                hasParent = true;
                break;
            }
        }
        if (!hasParent) {
            sources.append(node.id);
            m_nodes[node.id].level = 0;
            qDebug() << "  Source found:" << node.label << "(" << node.id.toString() << ") level set to 0";
        }
    }

    // Si aucun nœud source n'est trouvé, prendre tous les nœuds comme niveau 0
    if (sources.isEmpty() && !m_nodes.isEmpty()) {
        qDebug() << "No sources found, setting all nodes to level 0";
        for (auto& node : m_nodes) {
            node.level = 0;
        }
        return;
    }

    // Propagation BFS à partir des sources
    QQueue<QUuid> queue;
    for (const QUuid& sourceId : sources) {
        queue.enqueue(sourceId);
    }

    while (!queue.isEmpty()) {
        QUuid currentId = queue.dequeue();
        int currentLevel = m_nodes[currentId].level;
        qDebug() << "  Processing:" << m_nodes[currentId].label << "at level" << currentLevel;

        if (m_adjacencyList.contains(currentId)) {
            for (const QUuid& childId : m_adjacencyList[currentId]) {
                int newLevel = currentLevel + 1;
                if (m_nodes.contains(childId) && (m_nodes[childId].level < newLevel || m_nodes[childId].level == -1)) {
                    m_nodes[childId].level = newLevel;
                    queue.enqueue(childId);
                    qDebug() << "    Setting child:" << m_nodes[childId].label << "to level:" << newLevel;
                }
            }
        }
    }

    // Pour les nœuds qui n'ont pas reçu de niveau (graphe non connecté)
    for (auto& node : m_nodes) {
        if (node.level == -1) {
            node.level = 0;
            qDebug() << "Orphan node:" << node.label << "set to level 0";
        }
    }

    // DEBUG: Afficher les niveaux calculés
    qDebug() << "=== FINAL LEVELS ===";
    for (auto& node : m_nodes) {
        qDebug() << "  Node:" << node.label << "type:" << static_cast<int>(node.type) << "level:" << node.level;
    }
}

void GraphView::assignLevelsFromSources()
{
    QQueue<QUuid> queue;

    // Les nœuds source sont niveau 0
    for (auto& node : m_nodes) {
        if (node.type == dn::core::NodeType::Source) {
            node.level = 0;
            queue.enqueue(node.id);
        }
    }

    // Propagation BFS
    while (!queue.isEmpty()) {
        QUuid currentId = queue.dequeue();
        int currentLevel = m_nodes[currentId].level;

        if (m_adjacencyList.contains(currentId)) {
            for (const QUuid& childId : m_adjacencyList[currentId]) {
                if (m_nodes[childId].level < currentLevel + 1) {
                    m_nodes[childId].level = currentLevel + 1;
                    queue.enqueue(childId);
                }
            }
        }
    }
}

void GraphView::assignLevelsFromDependencies()
{
    for (auto& node : m_nodes) {
        if (node.level == -1) {
            QHash<QUuid, int> memo;
            node.level = getNodeDepth(node.id, memo);
        }
    }
}

int GraphView::getNodeDepth(const QUuid& id, QHash<QUuid, int>& memo)
{
    if (memo.contains(id))
        return memo[id];

    if (!m_adjacencyList.contains(id) || m_adjacencyList[id].isEmpty()) {
        memo[id] = 0;
        return 0;
    }

    int maxDepth = 0;
    for (const QUuid& childId : m_adjacencyList[id]) {
        int depth = getNodeDepth(childId, memo);
        maxDepth = qMax(maxDepth, depth);
    }

    memo[id] = maxDepth + 1;
    return maxDepth + 1;
}

void GraphView::assignRowsAndColumns()
{
    qDebug() << "=== assignRowsAndColumns ===";

    // Grouper les nœuds par niveau (level = position Y)
    QMap<int, QVector<QUuid>> nodesByLevel;
    for (auto& node : m_nodes) {
        if (node.level >= 0) {
            nodesByLevel[node.level].append(node.id);
            qDebug() << "  Node" << node.label << "added to level" << node.level;
        }
    }

    qDebug() << "  Nodes by level:" << nodesByLevel.keys();

    // Pour chaque niveau, assigner les colonnes (position X)
    for (auto it = nodesByLevel.begin(); it != nodesByLevel.end(); ++it) {
        int level = it.key();
        QVector<QUuid>& nodeIds = it.value();
        qDebug() << "  Processing level" << level << "with" << nodeIds.size() << "nodes";

        // Trier les nœuds pour minimiser les croisements
        if (level > 0) {
            QHash<QUuid, int> parentColumnOrder;
            for (const QUuid& nodeId : nodeIds) {
                // Trouver le parent de ce nœud
                for (auto pit = m_adjacencyList.begin(); pit != m_adjacencyList.end(); ++pit) {
                    if (pit.value().contains(nodeId)) {
                        QUuid parentId = pit.key();
                        if (m_nodes.contains(parentId)) {
                            parentColumnOrder[nodeId] = m_nodes[parentId].col;
                            qDebug() << "    Node" << m_nodes[nodeId].label << "has parent" << m_nodes[parentId].label << "at col" << m_nodes[parentId].col;
                            break;
                        }
                    }
                }
            }

            // Trier par colonne du parent
            std::sort(nodeIds.begin(), nodeIds.end(),
                      [&parentColumnOrder](const QUuid& a, const QUuid& b) {
                          return parentColumnOrder.value(a, 0) < parentColumnOrder.value(b, 0);
                      });
        }

        // Assigner les colonnes (position X horizontale)
        for (int i = 0; i < nodeIds.size(); ++i) {
            m_nodes[nodeIds[i]].row = level;     // row = niveau (pour position Y)
            m_nodes[nodeIds[i]].col = i;         // col = position horizontale
            qDebug() << "    Assigned" << m_nodes[nodeIds[i]].label << "-> row:" << level << "col:" << i;
        }
    }
}

void GraphView::resolveColumnCrossings()
{
    // Minimiser les croisements entre niveaux adjacents
    for (int level = 0; level < 10; ++level) {
        QMap<int, QVector<QUuid>> columns;

        // Collecter les nœuds par colonne pour ce niveau
        for (auto& node : m_nodes) {
            if (node.row == level) {
                columns[node.col].append(node.id);
            }
        }

        // Calculer les croisements actuels
        int currentCrossings = calculateCrossings(columns);

        // Essayer d'échanger des colonnes adjacentes
        bool improved = true;
        while (improved) {
            improved = false;
            for (int col = 0; col < columns.size() - 1; ++col) {
                // Échanger les colonnes
                QVector<QUuid> temp = columns[col];
                columns[col] = columns[col + 1];
                columns[col + 1] = temp;

                int newCrossings = calculateCrossings(columns);
                if (newCrossings < currentCrossings) {
                    currentCrossings = newCrossings;
                    improved = true;
                    // Mettre à jour les colonnes des nœuds
                    for (QUuid id : columns[col]) {
                        m_nodes[id].col = col;
                    }
                    for (QUuid id : columns[col + 1]) {
                        m_nodes[id].col = col + 1;
                    }
                } else {
                    // Re-échanger
                    temp = columns[col];
                    columns[col] = columns[col + 1];
                    columns[col + 1] = temp;
                }
            }
        }
    }
}

int GraphView::calculateCrossings(const QMap<int, QVector<QUuid>>& columns)
{
    int crossings = 0;
    QList<int> colKeys = columns.keys();

    for (int i = 0; i < colKeys.size() - 1; ++i) {
        const QVector<QUuid>& leftCol = columns[colKeys[i]];
        const QVector<QUuid>& rightCol = columns[colKeys[i + 1]];

        for (int a = 0; a < leftCol.size(); ++a) {
            for (int b = a + 1; b < leftCol.size(); ++b) {
                // Vérifier si les connexions de leftCol[a] et leftCol[b] vers rightCol se croisent
                QVector<QUuid> targetsA = m_adjacencyList.value(leftCol[a]);
                QVector<QUuid> targetsB = m_adjacencyList.value(leftCol[b]);

                for (int c = 0; c < rightCol.size(); ++c) {
                    for (int d = c + 1; d < rightCol.size(); ++d) {
                        if (targetsA.contains(rightCol[c]) && targetsB.contains(rightCol[d])) {
                            crossings++;
                        }
                        if (targetsA.contains(rightCol[d]) && targetsB.contains(rightCol[c])) {
                            crossings++;
                        }
                    }
                }
            }
        }
    }

    return crossings;
}

void GraphView::minimizeCrossings()
{
    // Algorithme de barycentre pour réduire les croisements
    for (int iteration = 0; iteration < 10; ++iteration) {
        // Parcours descendant
        for (int level = 1; level <= 10; ++level) {
            QMap<QUuid, double> barycenters;

            for (auto& node : m_nodes) {
                if (node.row == level) {
                    double sum = 0;
                    int count = 0;

                    // Trouver les parents
                    for (const QUuid& parentId : m_adjacencyList.keys()) {
                        if (m_adjacencyList[parentId].contains(node.id)) {
                            if (m_nodes.contains(parentId)) {
                                sum += m_nodes[parentId].col;
                                count++;
                            }
                        }
                    }

                    if (count > 0) {
                        barycenters[node.id] = sum / count;
                    }
                }
            }

            // Trier les nœuds par barycentre
            QVector<QUuid> nodesAtLevel;
            for (auto& node : m_nodes) {
                if (node.row == level) {
                    nodesAtLevel.append(node.id);
                }
            }

            std::sort(nodesAtLevel.begin(), nodesAtLevel.end(),
                      [&barycenters](const QUuid& a, const QUuid& b) {
                          return barycenters.value(a, 0) < barycenters.value(b, 0);
                      });

            // Réassigner les colonnes
            for (int i = 0; i < nodesAtLevel.size(); ++i) {
                m_nodes[nodesAtLevel[i]].col = i;
            }
        }
    }
}

void GraphView::applyPositions()
{
    qDebug() << "=== applyPositions ===";

    if (m_nodes.isEmpty()) return;

    // Find children for each parent
    QMap<QUuid, QVector<QUuid>> parentToChildren;
    for (auto pit = m_adjacencyList.begin(); pit != m_adjacencyList.end(); ++pit) {
        parentToChildren[pit.key()] = pit.value();
    }

    // Find parent for each node
    QMap<QUuid, QUuid> childToParent;
    for (auto pit = m_adjacencyList.begin(); pit != m_adjacencyList.end(); ++pit) {
        for (const QUuid& childId : pit.value()) {
            childToParent[childId] = pit.key();
        }
    }

    // Group nodes by row
    QMap<int, QVector<QUuid>> nodesByRow;
    for (auto& node : m_nodes) {
        if (!node.shapeItem) continue;
        if (node.row < 0) node.row = 0;
        nodesByRow[node.row].append(node.id);
    }

    // Calculate max height per row for Y positions
    QMap<int, qreal> maxHeightPerRow;
    for (auto& node : m_nodes) {
        if (!node.shapeItem) continue;
        qreal height = node.shapeItem->boundingRect().height();
        if (height <= 0) height = MIN_NODE_HEIGHT;
        if (!maxHeightPerRow.contains(node.row) || height > maxHeightPerRow[node.row]) {
            maxHeightPerRow[node.row] = height;
        }
    }

    // Calculate Y positions
    QMap<int, qreal> rowYPositions;
    qreal currentY = START_Y;
    for (int row = 0; row <= nodesByRow.keys().last(); ++row) {
        rowYPositions[row] = currentY;
        currentY += maxHeightPerRow.value(row, MIN_NODE_HEIGHT) + ROW_SPACING;
    }

    // ===== PASS 1: Position root (parent) nodes first =====
    // Start from row 0 (top) so parents are positioned before children
    for (int row = 0; row <= nodesByRow.keys().last(); ++row) {
        if (!nodesByRow.contains(row)) continue;
        qreal y = rowYPositions[row];

        // Group nodes by their parent
        QMap<QUuid, QVector<QUuid>> nodesByParent;
        QVector<QUuid> rootNodes;

        for (const QUuid& id : nodesByRow[row]) {
            if (childToParent.contains(id)) {
                nodesByParent[childToParent[id]].append(id);
            } else {
                rootNodes.append(id);
            }
        }

        // Position each parent group
        for (auto pit = nodesByParent.begin(); pit != nodesByParent.end(); ++pit) {
            QUuid parentId = pit.key();
            QVector<QUuid>& nodes = pit.value();

            if (!m_nodes.contains(parentId)) continue;

            // Get parent center
            qreal parentWidth = m_nodes[parentId].shapeItem->boundingRect().width();
            if (parentWidth <= 0) parentWidth = MIN_NODE_WIDTH;
            qreal parentCenterX = m_nodes[parentId].position.x() + parentWidth / 2;

            // Calculate total width of children
            qreal totalWidth = 0;
            for (const QUuid& nid : nodes) {
                qreal w = m_nodes[nid].shapeItem->boundingRect().width();
                if (w <= 0) w = MIN_NODE_WIDTH;
                totalWidth += w;
            }
            totalWidth += (nodes.size() - 1) * COLUMN_SPACING;

            // Position children centered under parent
            qreal startX = parentCenterX - totalWidth / 2;
            for (const QUuid& nid : nodes) {
                qreal w = m_nodes[nid].shapeItem->boundingRect().width();
                if (w <= 0) w = MIN_NODE_WIDTH;
                m_nodes[nid].position = QPointF(startX, y);
                m_nodes[nid].shapeItem->setPos(m_nodes[nid].position);
                startX += w + COLUMN_SPACING;
            }
        }

        // Position root nodes (no parent) with spacing
        qreal x = START_X;
        if (!nodesByParent.isEmpty()) {
            // Find rightmost node to continue from
            qreal maxX = START_X;
            for (auto pit = nodesByParent.begin(); pit != nodesByParent.end(); ++pit) {
                for (const QUuid& nid : pit.value()) {
                    qreal right = m_nodes[nid].position.x() + m_nodes[nid].shapeItem->boundingRect().width();
                    maxX = qMax(maxX, right);
                }
            }
            x = maxX + COLUMN_SPACING;
        }

        for (const QUuid& nid : rootNodes) {
            qreal w = m_nodes[nid].shapeItem->boundingRect().width();
            if (w <= 0) w = MIN_NODE_WIDTH;
            m_nodes[nid].position = QPointF(x, y);
            m_nodes[nid].shapeItem->setPos(m_nodes[nid].position);
            x += w + COLUMN_SPACING;
        }
    }

    // Second pass: center parents under their children block
    for (int row = 0; row < nodesByRow.keys().last(); ++row) {
        if (!nodesByRow.contains(row)) continue;
        qreal y = rowYPositions[row];

        for (const QUuid& parentId : nodesByRow[row]) {
            if (!parentToChildren.contains(parentId) || parentToChildren[parentId].isEmpty()) continue;

            const QVector<QUuid>& children = parentToChildren[parentId];
            qreal minX = 1e9, maxX = -1e9;
            for (const QUuid& cid : children) {
                if (m_nodes.contains(cid)) {
                    minX = qMin(minX, m_nodes[cid].position.x());
                    maxX = qMax(maxX, m_nodes[cid].position.x() + m_nodes[cid].shapeItem->boundingRect().width());
                }
            }

            if (minX < 1e9) {
                GraphNode& parent = m_nodes[parentId];
                qreal parentWidth = parent.shapeItem->boundingRect().width();
                if (parentWidth <= 0) parentWidth = MIN_NODE_WIDTH;
                qreal childrenCenterX = (minX + maxX) / 2;
                parent.position.setX(childrenCenterX - parentWidth / 2);
                parent.position.setY(y);
                parent.shapeItem->setPos(parent.position);
            }
        }
    }

    // Third pass: fix overlaps between sibling groups on same row
    for (int row = 0; row <= nodesByRow.keys().last(); ++row) {
        if (!nodesByRow.contains(row)) continue;
        QVector<QUuid>& nodes = nodesByRow[row];

        // Group by parent
        QMap<QUuid, QVector<QUuid>> groups;
        QVector<QUuid> orphanNodes;
        for (const QUuid& id : nodes) {
            if (childToParent.contains(id) && m_nodes.contains(childToParent[id])) {
                groups[childToParent[id]].append(id);
            } else {
                orphanNodes.append(id);
            }
        }

        // Sort groups by leftmost position
        QVector<QPair<QUuid, qreal>> sortedGroups;
        for (auto pit = groups.begin(); pit != groups.end(); ++pit) {
            qreal minX = 1e9;
            for (const QUuid& cid : pit.value()) {
                minX = qMin(minX, m_nodes[cid].position.x());
            }
            sortedGroups.append({pit.key(), minX});
        }
        std::sort(sortedGroups.begin(), sortedGroups.end(),
                  [](const auto& a, const auto& b) { return a.second < b.second; });

        // Position groups with minimum spacing
        qreal currentX = START_X;

        for (const auto& pair : sortedGroups) {
            const QVector<QUuid>& group = groups[pair.first];

            // Shift group if needed
            qreal groupMinX = 1e9;
            for (const QUuid& cid : group) {
                groupMinX = qMin(groupMinX, m_nodes[cid].position.x());
            }

            qreal shift = currentX - groupMinX;
            if (shift > 0) {
                for (const QUuid& cid : group) {
                    m_nodes[cid].position.rx() += shift;
                    m_nodes[cid].shapeItem->setPos(m_nodes[cid].position);
                }
            }

            // Update currentX to end of this group
            qreal groupMaxX = -1e9;
            for (const QUuid& cid : group) {
                groupMaxX = qMax(groupMaxX, m_nodes[cid].position.x() + m_nodes[cid].shapeItem->boundingRect().width());
            }
            currentX = groupMaxX + COLUMN_SPACING;
        }

        // Position orphan nodes
        qreal orphanX = currentX;
        for (const QUuid& id : orphanNodes) {
            m_nodes[id].position.setX(orphanX);
            m_nodes[id].shapeItem->setPos(m_nodes[id].position);
            orphanX += m_nodes[id].shapeItem->boundingRect().width() + COLUMN_SPACING;
        }
    }

    // Update parent positions after children groups are fixed
    for (int row = 0; row <= nodesByRow.keys().last(); ++row) {
        if (!nodesByRow.contains(row)) continue;

        for (const QUuid& parentId : nodesByRow[row]) {
            if (!parentToChildren.contains(parentId) || parentToChildren[parentId].isEmpty()) continue;
            if (!m_nodes.contains(parentId)) continue;

            const QVector<QUuid>& children = parentToChildren[parentId];
            qreal minX = 1e9, maxX = -1e9;
            for (const QUuid& cid : children) {
                if (m_nodes.contains(cid)) {
                    minX = qMin(minX, m_nodes[cid].position.x());
                    maxX = qMax(maxX, m_nodes[cid].position.x() + m_nodes[cid].shapeItem->boundingRect().width());
                }
            }

            if (minX < 1e9) {
                GraphNode& parent = m_nodes[parentId];
                qreal parentWidth = parent.shapeItem->boundingRect().width();
                if (parentWidth <= 0) parentWidth = MIN_NODE_WIDTH;
                qreal childrenCenterX = (minX + maxX) / 2;
                parent.position.setX(childrenCenterX - parentWidth / 2);
                parent.shapeItem->setPos(parent.position);
            }
        }
    }

    // Debug output
    for (auto& node : m_nodes) {
        if (!node.shapeItem) continue;
        qDebug() << "  Node:" << node.label << "row:" << node.row
                 << "pos:(" << node.position.x() << "," << node.position.y() << ")";
    }

    // ===== PASS 3: Fix overlaps between all nodes on same row =====
    for (int row = 0; row <= nodesByRow.keys().last(); ++row) {
        if (!nodesByRow.contains(row)) continue;
        if (nodesByRow[row].size() <= 1) continue;

        // Sort by X position
        QVector<QUuid> sorted = nodesByRow[row];
        std::sort(sorted.begin(), sorted.end(),
            [this](const QUuid& a, const QUuid& b) {
                return m_nodes[a].position.x() < m_nodes[b].position.x();
            });

        // Fix overlaps
        qreal currentX = m_nodes[sorted[0]].position.x();
        for (int i = 0; i < sorted.size(); ++i) {
            QUuid nid = sorted[i];
            qreal w = m_nodes[nid].shapeItem->boundingRect().width();
            if (w <= 0) w = MIN_NODE_WIDTH;

            if (m_nodes[nid].position.x() < currentX) {
                m_nodes[nid].position.setX(currentX);
                m_nodes[nid].shapeItem->setPos(m_nodes[nid].position);
            }
            currentX = m_nodes[nid].position.x() + w + COLUMN_SPACING;
        }
    }

    centerOnNodes();
}



void GraphView::updateConnectionPositions()
{
    for (QGraphicsLineItem* line : m_connections) {
        QString fromStr = line->data(0).toString();
        QString toStr = line->data(1).toString();

        QUuid fromId(fromStr);
        QUuid toId(toStr);

        if (m_nodes.contains(fromId) && m_nodes.contains(toId)) {
            const GraphNode& from = m_nodes[fromId];
            const GraphNode& to = m_nodes[toId];

            qreal fromWidth = from.shapeItem->boundingRect().width();
            qreal fromHeight = from.shapeItem->boundingRect().height();
            qreal toWidth = to.shapeItem->boundingRect().width();

            // Bas du nœud source (centre horizontal)
            QPointF fromPoint(
                from.position.x() + fromWidth / 2,
                from.position.y() + fromHeight
                );

            // Haut du nœud cible (centre horizontal)
            QPointF toPoint(
                to.position.x() + toWidth / 2,
                to.position.y()
                );

            line->setLine(QLineF(fromPoint, toPoint));
        }
    }
}



void GraphView::refresh()
{
    viewport()->update();
    updateConnectionPositions();
}


void GraphView::centerOnNodes()
{
    if (m_nodes.isEmpty()) {
        centerOn(0, 0);
        return;
    }

    scene()->update();
    QCoreApplication::processEvents();

    QRectF boundingRect;
    bool hasValidRect = false;

    for (const auto& node : m_nodes) {
        if (!node.shapeItem) {
            continue;
        }

        QRectF nodeRect = node.shapeItem->sceneBoundingRect();
        if (nodeRect.isValid() && !nodeRect.isNull()) {
            boundingRect = boundingRect.united(nodeRect);
            hasValidRect = true;
        }
    }

    if (hasValidRect) {
        // Extend scene rect to cover all nodes + padding
        scene()->setSceneRect(boundingRect.adjusted(-100, -100, 100, 100));

        // Reset any existing transform before fitting
        resetTransform();
        fitInView(boundingRect.adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
    } else {
        centerOn(0, 0);
    }

    viewport()->repaint();
}

QHash<QUuid, QPointF> GraphView::getNodePositions() const
{
    QHash<QUuid, QPointF> positions;

    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
        positions[it.key()] = it.value().position;
    }

    return positions;
}

void GraphView::setNodePosition(const QUuid& nodeId, const QPointF& position)
{
    if (!m_nodes.contains(nodeId)) {
        qWarning() << "Node not found:" << nodeId.toString();
        return;
    }

    GraphNode& node = m_nodes[nodeId];
    node.position = position;
    node.shapeItem->setPos(position);

    updateConnectionPositions();
}

QJsonObject GraphView::saveLayout() const
{
    QJsonObject layout;
    QJsonArray nodesArray;

    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
        QJsonObject nodeObj;
        nodeObj["id"] = it.key().toString();

        QJsonObject posObj;
        posObj["x"] = it.value().position.x();
        posObj["y"] = it.value().position.y();
        nodeObj["position"] = posObj;

        nodesArray.append(nodeObj);
    }

    layout["nodes"] = nodesArray;
    return layout;
}


void GraphView::requestLayout()
{
    if (m_layoutPending || m_nodes.isEmpty()) {
        qDebug() << "requestLayout skipped (pending:" << m_layoutPending
                 << "empty:" << m_nodes.isEmpty() << ")";
        return;
    }

    m_layoutPending = true;
    qDebug() << "requestLayout - scheduling autoLayout";
    QMetaObject::invokeMethod(this, "autoLayout", Qt::QueuedConnection);
}

void GraphView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    // Optionnel : relayout après redimensionnement
    requestLayout();
}


void GraphView::restoreLayout(const QJsonObject& layout)
{
    QJsonArray nodesArray = layout["nodes"].toArray();

    for (const QJsonValue& nodeValue : nodesArray) {
        QJsonObject nodeObj = nodeValue.toObject();
        QUuid nodeId(nodeObj["id"].toString());

        if (nodeObj.contains("position")) {
            QJsonObject posObj = nodeObj["position"].toObject();
            QPointF position(posObj["x"].toDouble(), posObj["y"].toDouble());
            setNodePosition(nodeId, position);
        }
    }
}

// Méthode pour trouver un nœud à une position donnée
QUuid GraphView::findNodeAtPosition(const QPointF& scenePos) const
{
    QGraphicsItem* item = scene()->itemAt(scenePos, QTransform());
    if (item && item->data(0).isValid()) {
        return QUuid(item->data(0).toString());
    }
    return QUuid(); // Retourne un UUID nul si aucun nœud trouvé
}

// ========== EVENEMENTS DE LA SOURIS ET DU CLAVIER ===================
// sélection d'un noeud
void GraphView::mousePressEvent(QMouseEvent* event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem* item = scene()->itemAt(scenePos, QTransform());

    if (item && item->data(0).isValid()) {
        QUuid nodeId(item->data(0).toString());

        if (event->button() == Qt::LeftButton) {
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+Click pour multi-sélection (à implémenter)
            } else {
                selectNewNode(nodeId);
            }
            m_draggingNodeId = nodeId;
            m_dragStartPos = scenePos;
            m_isDragging = true;
        }
    } else if (event->button() == Qt::LeftButton) {
        selectNewNode(QUuid());
    }

    QGraphicsView::mousePressEvent(event);
}

void GraphView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isDragging && m_nodes.contains(m_draggingNodeId)) {
        GraphNode& node = m_nodes[m_draggingNodeId];
        QPointF newPos = node.shapeItem->pos();

        if (newPos != node.position) {
            node.position = newPos;
            emit nodeMoved(m_draggingNodeId, newPos);
            updateConnectionPositions();
        }
    }

    m_isDragging = false;
    m_draggingNodeId = QUuid();

    QGraphicsView::mouseReleaseEvent(event);
}

void GraphView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPointF scenePos = mapToScene(event->pos());
    QGraphicsItem* item = scene()->itemAt(scenePos, QTransform());

    if (item && item->data(0).isValid()) {
        QUuid nodeId(item->data(0).toString());
        emit editNodeRequested(nodeId);
    } else {
        QGraphicsView::mouseDoubleClickEvent(event);
    }
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && m_nodes.contains(m_draggingNodeId)) {
        QPointF currentPos = mapToScene(event->pos());
        QPointF delta = currentPos - m_dragStartPos;

        GraphNode& node = m_nodes[m_draggingNodeId];
        node.shapeItem->moveBy(delta.x(), delta.y());
        m_dragStartPos = currentPos;

        updateConnectionPositions();
    }

    QGraphicsView::mouseMoveEvent(event);
}

void GraphView::wheelEvent(QWheelEvent* event)
{
    qreal factor = 1.05;
    if (event->angleDelta().y() > 0) {
        scale(factor, factor);
    } else if (event->angleDelta().y() < 0) {
        scale(1.0 / factor, 1.0 / factor);
    }
}

void GraphView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete && !m_selectedNodeId.isNull()) {
        emit nodeSelected(m_selectedNodeId);
    } else if (event->key() == Qt::Key_L) {
        autoLayout();
    } else if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
        scale(1.2, 1.2);
    } else if (event->key() == Qt::Key_Minus) {
        scale(0.8, 0.8);
    } else if (event->key() == Qt::Key_0) {
        resetTransform();
        autoLayout();
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

// ========== MENUS CONTEXTUELS ==========

// Gestionnaire d'événement du menu contextuel
void GraphView::contextMenuEvent(QContextMenuEvent* event)
{
    QPointF scenePos = mapToScene(event->pos()); // retourne la positino du point où on a cliqué
    QUuid nodeId = findNodeAtPosition(scenePos); // identifie s'il y a un noeud à cette position

    if (nodeId.isNull()) {
        // Clic sur une zone vide
        showEmptyAreaContextMenu(event->globalPos());
    } else {
        // Clic sur un nœud
        showNodeContextMenu(nodeId, event->globalPos());
    }

    event->accept();
}

// Menu pour la zone vide

void GraphView::showEmptyAreaContextMenu(const QPoint& globalPos)
{
    QMenu contextMenu;

    // Menu "Ajouter une source de données"
    QMenu* sourceMenu = contextMenu.addMenu(tr("Ajouter une source de données"));

    // Sous-menu pour les fichiers
    QMenu* fromFileMenu = sourceMenu->addMenu(tr("À partir d'un fichier"));

    QAction* fromCsvAction = fromFileMenu->addAction(tr("CSV"));
    connect(fromCsvAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::CSV);
    });

    QAction* txtAction = fromFileMenu->addAction(tr("TXT (texte délimité)"));
    connect(txtAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::TXT);
    });

    QAction* excelAction = fromFileMenu->addAction(tr("Excel"));
    connect(excelAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::EXCEL);
    });

    QAction* pdfAction = fromFileMenu->addAction(tr("PDF"));
    connect(pdfAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::PDF);
    });

    QAction* jsonAction = fromFileMenu->addAction(tr("JSON"));
    connect(jsonAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::JSON);
    });

    QAction* xmlAction = fromFileMenu->addAction(tr("XML"));
    connect(xmlAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::XML);
    });

    QAction* fecAction = fromFileMenu->addAction(tr("FEC (Écritures comptables)"));
    connect(fecAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::FEC);
    });

    QAction* folderAction=fromFileMenu->addAction(tr("Dossier"));
    connect(folderAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::FOLDER);
    });

    sourceMenu->addSeparator();

    // Autres sources
    QAction* dbAction = sourceMenu->addAction(tr("À partir d'une base de données SQL"));
    connect(dbAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::SQL);
    });

    QAction* webAction = sourceMenu->addAction(tr("Page web (HTML)"));
    connect(webAction, &QAction::triggered, this, [this](){
        emit sourceNodeRequested(ConnectorType::WEB);
    });

    QAction* apiAction = sourceMenu->addAction(tr("API REST"));

    contextMenu.addSeparator();

    QMenu* refreshMenu=contextMenu.addMenu(tr("Rafraîchir "));
    QAction* refreshAction=refreshMenu->addAction(tr("toutes les sources de données"));
    if(m_nodes.empty()){
        refreshMenu->setEnabled(false);
    }
    connect(refreshAction, &QAction::triggered,this,&GraphView::dataRefreshRequested);

    contextMenu.addSeparator();

    QMenu* clearMenu = contextMenu.addMenu(tr("Supprimer "));
    QAction* clearAction=clearMenu->addAction(tr("Tous les noeuds"));
    if(m_nodes.empty()){
        clearMenu->setEnabled(false);
    }
    connect(clearAction, &QAction::triggered,this,&GraphView::clearNetworkRequested);

    contextMenu.exec(globalPos);
}

// Menu pour un nœud

void GraphView::showNodeContextMenu(const QUuid& nodeId, const QPoint& globalPos)
{
    // Récupérer le nœud pour obtenir son type et son nom
    RuntimeNode* node = nullptr;

    // Il faut un moyen d'accéder au réseau depuis GraphView
    // Soit en passant une référence au Network, soit en stockant les infos dans GraphNode
    // Pour l'instant, utilisons les données stockées dans m_nodes
    if (!m_nodes.contains(nodeId)) {
        return;
    }

    const GraphNode& graphNode = m_nodes[nodeId];

    if(graphNode.selected==true){
        QMenu contextMenu;

        // Titre du menu avec le nom du nœud
        QAction* titleAction = contextMenu.addAction(tr("Node: %1").arg(graphNode.label));
        titleAction->setEnabled(false);

        contextMenu.addSeparator();

        // Actions communes à tous les nœuds
        QAction* deleteAction = contextMenu.addAction(tr("Supprimer"));

        contextMenu.addSeparator();

        // Sous-menu pour les fichiers
        QMenu* transfMenu = contextMenu.addMenu(tr("Transformer les données"));
        QMenu* transfColMenu = transfMenu->addMenu(tr("Transformer les colonnes"));
        QAction* selectColumnsAction=transfColMenu->addAction(tr("Sélectionner les colonnes"));
        connect(selectColumnsAction,&QAction::triggered, this,[this](){
            emit transformationRequested(dn::core::TransformationType::SelectColumns);
        });

        QMenu* transfRowMenu = transfMenu->addMenu(tr("Transformer les lignes"));
        QAction* filterAction=transfRowMenu->addAction(tr("Filter les lignes"));

        connect(filterAction,&QAction::triggered, this,[this](){
            emit transformationRequested(dn::core::TransformationType::Filter);
        });

        contextMenu.addSeparator();

        // Actions spécifiques selon le type de nœud
        QAction* computeAction = nullptr;
        QAction* editAction = nullptr;

        switch (graphNode.type) {
        case NodeType::Source:
            editAction = contextMenu.addAction(tr("Modifier la source de données"));
            computeAction = contextMenu.addAction(tr("Rafraîchir les données"));
            break;

        case NodeType::Transform:
            editAction = contextMenu.addAction(tr("Modifier la transformation"));
            break;

        case NodeType::Merge:
            editAction = contextMenu.addAction(tr("Configurer la fusion"));

            break;

        case NodeType::Target:
            editAction = contextMenu.addAction(tr("Exporter les données"));
            break;
        }

        // Connecter les actions
        connect(deleteAction, &QAction::triggered, [this, nodeId]() {
            emit deleteNodeRequested(nodeId);
        });

        if (computeAction) {
            connect(computeAction, &QAction::triggered, [this, nodeId]() {
                emit computeFromNodeRequested(nodeId);
            });
        }

        if (editAction) {
            connect(editAction, &QAction::triggered, [this, nodeId]() {
                emit editNodeRequested(nodeId);
            });
        }

        contextMenu.addSeparator();

        // Menu "Ajouter une source de données"
        QMenu* targetMenu = contextMenu.addMenu(tr("Ajouter une cible de données"));

        // Sous-menu pour les fichiers
        QMenu* toFileMenu = targetMenu->addMenu(tr("Vers un fichier"));

        QAction* toCsvAction = toFileMenu->addAction(tr("CSV"));
        connect(toCsvAction, &QAction::triggered, this, [this](){
            emit targetNodeRequested(ConnectorType::CSV);
        });

QAction* toTxtAction = toFileMenu->addAction(tr("TXT (texte délimité)"));
    connect(toTxtAction, &QAction::triggered, this, [this](){
        emit targetNodeRequested(ConnectorType::TXT);
    });

    QAction* toExcelAction = toFileMenu->addAction(tr("Excel"));
    connect(toExcelAction, &QAction::triggered, this, [this](){
        emit targetNodeRequested(ConnectorType::EXCEL);
    });

    QAction* toXMLAction = toFileMenu->addAction(tr("XML"));
    connect(toXMLAction, &QAction::triggered, this, [this](){
        emit targetNodeRequested(ConnectorType::XML);
    });


    contextMenu.exec(globalPos);
    }
}

// repérer le noeud sélectionné
void GraphView::selectNewNode(const QUuid& id)
{
    // Désélectionner l'ancien noeud
    if (m_nodes.contains(m_selectedNodeId)) {
        QGraphicsItem* oldItem = m_nodes[m_selectedNodeId].shapeItem;
        if (auto* ellipse = dynamic_cast<QGraphicsEllipseItem*>(oldItem)) {
            ellipse->setPen(QPen(Qt::black, 2));
        } else if (auto* rect = dynamic_cast<QGraphicsRectItem*>(oldItem)) {
            rect->setPen(QPen(Qt::black, 2));
        }
    }

    GraphNode& unselectedNode = m_nodes[m_selectedNodeId];
    unselectedNode.selected=false;

    // Sélectionner le nouveau noeud
    m_selectedNodeId = id;

    if (m_nodes.contains(id)) {
        QGraphicsItem* newItem = m_nodes[id].shapeItem;
        if (auto* ellipse = dynamic_cast<QGraphicsEllipseItem*>(newItem)) {
            ellipse->setPen(QPen(Qt::red, 3));
        } else if (auto* rect = dynamic_cast<QGraphicsRectItem*>(newItem)) {
            rect->setPen(QPen(Qt::red, 3));
        }
        GraphNode& selectedNode = m_nodes[m_selectedNodeId];
        selectedNode.selected=true;
        emit nodeSelected(id); // émettre le signal à destination de MainWinow
    }
}

void GraphView::updateNodePositions()
{
    for (auto& node : m_nodes) {
        if (node.shapeItem) {
            node.shapeItem->setPos(node.position);
        }
    }
    updateConnectionPositions();
}