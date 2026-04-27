#ifndef TWOLINEHEADER_H
#define TWOLINEHEADER_H

#include <QHeaderView>
#include <QPainter>

class TwoLineHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit TwoLineHeader(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        setSectionResizeMode(QHeaderView::Interactive);
        setDefaultSectionSize(100);
        if (orientation == Qt::Horizontal) {
            setMinimumHeight(50);
        }
    }

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override
    {
        if (!model() || logicalIndex < 0)
            return;

        painter->save();

        QVariant columnName = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole);
        QVariant typeName = model()->headerData(logicalIndex, Qt::Horizontal, Qt::UserRole);

        QRect adjustedRect = rect.adjusted(2, 2, -2, -2);

        if (orientation() == Qt::Horizontal) {
            QFont nameFont = painter->font();
            nameFont.setBold(true);
            painter->setFont(nameFont);

            QRect nameRect = adjustedRect;
            nameRect.setHeight(adjustedRect.height() / 2);
            painter->drawText(nameRect, Qt::AlignCenter | Qt::TextWordWrap, columnName.toString());

            QFont typeFont = painter->font();
            typeFont.setBold(false);
            typeFont.setPointSize(typeFont.pointSize() - 1);
            painter->setFont(typeFont);

            QRect typeRect = adjustedRect;
            typeRect.setTop(nameRect.bottom());
            painter->drawText(typeRect, Qt::AlignCenter | Qt::TextWordWrap, typeName.toString());
        } else {
            painter->drawText(adjustedRect, Qt::AlignCenter, columnName.toString());
        }

        painter->restore();
    }

    QSize sizeHint() const override
    {
        QSize size = QHeaderView::sizeHint();
        if (orientation() == Qt::Horizontal) {
            size.setHeight(size.height() * 2);
        }
        return size;
    }
};

#endif // TWOLINEHEADER_H