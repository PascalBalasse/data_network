/**
 * @file GraphView.h
 * @brief Vue graphique du réseau de données
 *
 * Affiche le réseau sous forme de graphe avec:
 * - Nœuds (rectangles avec labels)
 * - Connexions (lignes entre nœuds)
 * - Layout automatique (algorithme en couches)
 * - Interactions (clic, glisser-déposer, menu contextuel)
 *
 * Parent: QGraphicsView
 *
 * Namespace: dn::ui
 */

#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include "../network/core/enums.h"
#include <QWidget>
#include <QHash>
#include <QWidget>
#include <QUuid>
#include "QGraphicsView"

using namespace dn::core;

namespace dn::ui {

    struct GraphNode
    {
        QUuid id;
        QString label;
        NodeType type;
        QPointF position;
        QGraphicsItem* shapeItem = nullptr;
        QGraphicsTextItem* textItem = nullptr;
        bool selected=false;

        // Pour le layout en couches
        int row = -1;
        int col = -1;
        int level = 0;
    };

    class GraphView : public QGraphicsView
    {
        Q_OBJECT

    public:
        explicit GraphView(QWidget *parent = nullptr);
        ~GraphView();

        /// Retourne les positions de tous les nœuds
        QHash<QUuid, QPointF> getNodePositions() const;

        /// Définit la position d'un nœud
        void setNodePosition(const QUuid& nodeId, const QPointF& position);

        /// Sauvegarde le layout
        QJsonObject saveLayout() const;

        /// Restaure le layout
        void restoreLayout(const QJsonObject& layout);

        /// Ajoute un nœud
        void addNode(const QUuid& id, dn::core::NodeType type, const QString& label);

        /// Ajoute un nœud avec connexion
        void addNodeWithConnection(const QUuid& parentId, const QUuid& nodeId, dn::core::NodeType type, const QString& label);

        /// Retire un nœud
        void removeNode(const QUuid& id);

        /// Met à jour le label
        void updateNodeLabel(const QUuid& id, const QString& label);

        /// Efface tout
        void clearGraph();

        /// Ajoute une connexion
        void addConnection(const QUuid& fromId, const QUuid& toId);

        /// Retire une connexion
        void removeConnection(const QUuid& fromId, const QUuid& toId);

        /// Efface les connexions
        void clearConnections();

        /// Centre la vue sur les nœuds
        void centerOnNodes();

        /// Rafraîchit l'affichage
        void refresh();
        void requestLayout();

        /// Retourne le nœud sélectionné
        QUuid getSelectedNodeId() const { return m_selectedNodeId; }
        void selectNewNode(const QUuid& id);

    public slots:
        /// Lance le layout automatique
        void autoLayout();

    signals:
        /// Émis quand un nœud est sélectionné
        void nodeSelected(const QUuid& nodeId);

        /// Émis quand un nœud est double-cliqué
        void nodeDoubleClicked(const QUuid& nodeId);

        /// Émis quand un nœud est déplacé
        void nodeMoved(const QUuid& nodeId, const QPointF& newPosition);

        /// Demande de créer un nœud source
        void sourceNodeRequested(const dn::core::ConnectorType& type );

        /// Demande de rafraîchir les données
        void dataRefreshRequested();

        /// Demande d'effacer le réseau
        void clearNetworkRequested();

        /// Demande d'ajouter une transformation
        void transformationRequested(const dn::core::TransformationType type);

        /// Demande d'éditer un nœud
        void editNodeRequested(const QUuid& nodeId);

        /// Demande de supprimer un nœud
        void deleteNodeRequested(const QUuid& nodeId);

        /// Demande de dupliquer un nœud
        void duplicateNodeRequested(const QUuid& nodeId);

        /// Demande de recalculer depuis un nœud
        void computeFromNodeRequested(const QUuid& nodeId);

        /// Demande de renommer un nœud
        void renameNodeRequested(const QUuid& nodeId);

        /// Demande de créer un nœud cible
        void targetNodeRequested(const dn::core::ConnectorType& type );

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void contextMenuEvent(QContextMenuEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
    private:
        //══════════════════════════════════════════════════════════════════
        // Layout automatique
        //══════════════════════════════════════════════════════════
        void computeNodeLevels();
        void assignLevelsFromSources();
        void assignLevelsFromDependencies();
        int getNodeDepth(const QUuid& id, QHash<QUuid, int>& memo);
        bool m_layoutPending = false;

        void assignRowsAndColumns();
        void resolveColumnCrossings();
        void minimizeCrossings();
        int calculateCrossings(const QMap<int, QVector<QUuid>>& columns);

        void applyPositions();
        QSizeF calculateNodeSize(const QString& label);

        //══════════════════════════════════════════════════════════════════
        // Menus contextuels
        //══════════════════════════════════════════════════════════════════
        void showEmptyAreaContextMenu(const QPoint& globalPos);
        void showNodeContextMenu(const QUuid& nodeId, const QPoint& globalPos);
        QUuid findNodeAtPosition(const QPointF& scenePos) const;

        //═══════════════════���═���════════════════════════════════════════════
        // Dessin
        //══════════════════════════════════════════════════════════════════
        QGraphicsItem* createShapeItem(dn::core::NodeType type, const QString& label);
        void updateNodePositions();
        void updateConnectionPositions();
        void drawConnection(const GraphNode& from, const GraphNode& to);

        //══════════════════════════════════════════════════════════════════
        // Données membres
        //══════════════════════════════════════════════════════════════════
        QHash<QUuid, GraphNode> m_nodes;
        QVector<QGraphicsLineItem*> m_connections;
        QHash<QUuid, QVector<QUuid>> m_adjacencyList;

        QUuid m_selectedNodeId;
        QPointF m_dragStartPos;
        QUuid m_draggingNodeId;
        bool m_isDragging;

        //══════════════════════════════════════════════════════════════════
        // Constantes
        //══════════════════════════════════════════════════════════════════
        static constexpr int NODE_PADDING_X = 20;
        static constexpr int NODE_PADDING_Y = 15;
        static constexpr int MIN_NODE_WIDTH = 100;
        static constexpr int MIN_NODE_HEIGHT = 50;
        static constexpr int COLUMN_SPACING = 60;
        static constexpr int ROW_SPACING = 120;
        static constexpr int START_X = 50;
        static constexpr int START_Y = 50;
    };

}

#endif // GRAPHVIEW_H
