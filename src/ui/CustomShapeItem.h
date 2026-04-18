/**
 * @file CustomShapeItem.h
 * @brief Item graphique réutilisable pour les nœuds
 *
 * Item graphique personnalisé avec:
 * - Formes: rectangle ou ellipse
 * - Labels texto
 * - Interactions: sélection, survol, glisser
 * - Style personnalisable (pen, brush, police)
 *
 * Namespace: dn::ui
 */

#ifndef CUSTOMSHAPEITEM_H
#define CUSTOMSHAPEITEM_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QString>

namespace dn::ui {

enum class ShapeType {
    Rectangle,
    Ellipse
};

class CustomShapeItem : public QGraphicsItem
{
public:
    CustomShapeItem(const QString& id, const QString& label,
                    const QPointF& center, qreal width, qreal height,
                    ShapeType shape = ShapeType::Rectangle,
                    QGraphicsItem* parent = nullptr);

    //══════════════════════════════════════════════════════════════════
    // Getters
    //══════════════════════════════════════════════════════════════════
    QString getId() const { return m_id; }
    QString getLabel() const { return m_label; }
    QPointF getCenter() const { return m_center; }
    qreal getWidth() const { return m_width; }
    qreal getHeight() const { return m_height; }
        ShapeType getShapeType() const { return m_shape; }
        QRectF getBoundingRect() const { return boundingRect(); }

        //══════════════════════════════════════════════════════════════════
        // Setters
        //══════════════════════════════════════════════════════════════════
        void setLabel(const QString& label);
        void setCenter(const QPointF& center);
        void setWidth(qreal width);
        void setHeight(qreal height);
        void setShape(ShapeType shape);
        void setSize(qreal width, qreal height);
        void setPosition(const QPointF& center);

        //══════════════════════════════════════════════════════════════════
        // Style
        //══════════════════════════════════════════════════════════════════
        void setPen(const QPen& pen);
        void setBrush(const QBrush& brush);
        void setLabelFont(const QFont& font);
        void setLabelColor(const QColor& color);
        void setMargin(qreal margin);

        //══════════════════════════════════════════════════════════════════
        // Sélection
        //══════════════════════════════════════════════════════════════════
        void setSelected(bool selected);
        bool isSelected() const { return m_isSelected; }

        //══════════════════════════════════════════════════════════════════
        // Méthodes réimplementées
        //══════════════════════════════════════════════════════════════════
        QRectF boundingRect() const override;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                   QWidget* widget = nullptr) override;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    private:
        void updateTextPosition();
        void updateBoundingRect();
        void drawShape(QPainter* painter);

        //══════════════════════════════════════════════════════════════════
        // Variables membres
        //══════════════════════════════════════════════════════════════════
        QString m_id;
        QString m_label;
        QPointF m_center;
        qreal m_width;
        qreal m_height;
        ShapeType m_shape;

        // Composants graphiques
        QGraphicsSimpleTextItem* m_textItem;

        // Style
        QPen m_pen;
        QBrush m_brush;
        QPen m_selectedPen;
        QPen m_hoverPen;
        qreal m_margin;

        // État
        bool m_isSelected;
        bool m_isHovered;

        //Valeurs mises en cache
        QRectF m_boundingRect;
    };
}
#endif // CUSTOMSHAPEITEM_H
