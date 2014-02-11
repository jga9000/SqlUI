// This is for manipulating/deleting a column/row
#include "ButtonHeaderView.h"
#include <QDebug>
#include <QTableView>

ButtonHeaderView::ButtonHeaderView(Qt::Orientation orientation, ButtonPosition pos, QWidget *parent)
    : QHeaderView(orientation, parent),
      type(pos)
{
    pixMap = new QPixmap("images/standardbutton-closetab.png");
}

void ButtonHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    QRect drawingRect;
    drawingRect.setHeight(16);
    drawingRect.setWidth(16);
    drawingRect.moveTopRight(rect.topRight());
    if (type == Column) {
    }
    else if(type == Row)
    {
    }
    painter->drawPixmap(drawingRect, *pixMap);
}

void ButtonHeaderView::mousePressEvent(QMouseEvent *event)
{
    int colStartX = sectionPosition(logicalIndexAt(event->pos()));
    int colLen = sectionSize(logicalIndexAt(event->pos()));
    qDebug() << "colStartX:" << colStartX;
    qDebug() << "colLen:" << colLen;
    qDebug() << "event->pos().x():" << event->pos().x();

    if (orientation() == Qt::Horizontal &&
        event->pos().x() > colStartX+colLen-17 &&
        event->pos().y() < 16 )
    {
        updateSection(0);
        emit pressed(logicalIndexAt(event->pos()));
    }
    else if (orientation() == Qt::Vertical)
    {
        updateSection(0);
        emit pressed(logicalIndexAt(event->pos()));
    }
    else
        QHeaderView::mousePressEvent(event);
}
