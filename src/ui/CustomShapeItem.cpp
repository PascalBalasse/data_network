#include "CustomShapeItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>

using namespace dn::ui;

CustomShapeItem::CustomShapeItem(const QString& id, const QString& label,
                                 const QPointF& center, qreal width, qreal height,
                                 ShapeType shape, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_id(id)
    , m_label(label)
    , m_center(center)
    , m_width(width)
    , m_height(height)
    , m_shape(shape)
    , m_margin(8.0)
    , m_isSelected(false)
    , m_isHovered(false)
{
    // Create text item
    m_textItem = new QGraphicsSimpleTextItem(this);
    m_textItem->setText(label);

    // Set default font
    QFont font("Arial", 10);
    m_textItem->setFont(font);
    m_textItem->setBrush(Qt::black);

    // Set default pens and brushes
    m_pen = QPen(Qt::black, 2);
    m_brush = QBrush(QColor(200, 200, 255));  // Light blue

    m_selectedPen = QPen(Qt::red, 3);
    m_hoverPen = QPen(Qt::green, 2);

    // Enable hover events
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    updateTextPosition();
    updateBoundingRect();
    setPos(m_center.x() - m_width / 2, m_center.y() - m_height / 2);
}

QRectF CustomShapeItem::boundingRect() const
{
    return m_boundingRect;
}

void CustomShapeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                            QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // Choose pen based on state
    QPen currentPen = m_pen;
    if (m_isSelected) {
        currentPen = m_selectedPen;
    } else if (m_isHovered) {
        currentPen = m_hoverPen;
    }

    painter->setPen(currentPen);
    painter->setBrush(m_brush);

    // Draw the shape
    drawShape(painter);

    // Draw label if needed (text is automatically drawn by child item)
    // The text item is a child so it will be drawn automatically
}

void CustomShapeItem::drawShape(QPainter* painter)
{
    QRectF rect(0, 0, m_width, m_height);

    if (m_shape == ShapeType::Rectangle) {
        painter->drawRect(rect);
    } else { // Ellipse
        painter->drawEllipse(rect);
    }
}

void CustomShapeItem::updateTextPosition()
{
    if (!m_textItem) return;

    QRectF textRect = m_textItem->boundingRect();

    // Calculate position to center text within the shape
    qreal textX = (m_width - textRect.width()) / 2;
    qreal textY = (m_height - textRect.height()) / 2;

    m_textItem->setPos(textX, textY);
}

void CustomShapeItem::updateBoundingRect()
{
    prepareGeometryChange();
    m_boundingRect = QRectF(0, 0, m_width, m_height);
}

// Setters implementation
void CustomShapeItem::setLabel(const QString& label)
{
    m_label = label;
    m_textItem->setText(label);
    updateTextPosition();
    update();
}

void CustomShapeItem::setCenter(const QPointF& center)
{
    m_center = center;
    setPos(m_center.x() - m_width / 2, m_center.y() - m_height / 2);
    update();
}

void CustomShapeItem::setWidth(qreal width)
{
    m_width = width;
    updateBoundingRect();
    updateTextPosition();
    setCenter(m_center); // Re-center
    update();
}

void CustomShapeItem::setHeight(qreal height)
{
    m_height = height;
    updateBoundingRect();
    updateTextPosition();
    setCenter(m_center); // Re-center
    update();
}

void CustomShapeItem::setSize(qreal width, qreal height)
{
    m_width = width;
    m_height = height;
    updateBoundingRect();
    updateTextPosition();
    setCenter(m_center); // Re-center
    update();
}

void CustomShapeItem::setShape(ShapeType shape)
{
    m_shape = shape;
    update();
}

void CustomShapeItem::setPosition(const QPointF& center)
{
    setCenter(center);
}

// Styling methods
void CustomShapeItem::setPen(const QPen& pen)
{
    m_pen = pen;
    update();
}

void CustomShapeItem::setBrush(const QBrush& brush)
{
    m_brush = brush;
    update();
}

void CustomShapeItem::setLabelFont(const QFont& font)
{
    m_textItem->setFont(font);
    updateTextPosition();
    update();
}

void CustomShapeItem::setLabelColor(const QColor& color)
{
    m_textItem->setBrush(color);
    update();
}

void CustomShapeItem::setMargin(qreal margin)
{
    m_margin = margin;
    updateTextPosition();
    update();
}

// Selection and interaction
void CustomShapeItem::setSelected(bool selected)
{
    m_isSelected = selected;
    update();
}

// Event handlers
void CustomShapeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isSelected = !m_isSelected;
        update();
        event->accept();
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void CustomShapeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}

void CustomShapeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    m_isHovered = true;
    update();
    QGraphicsItem::hoverEnterEvent(event);
}

void CustomShapeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    m_isHovered = false;
    update();
    QGraphicsItem::hoverLeaveEvent(event);
}