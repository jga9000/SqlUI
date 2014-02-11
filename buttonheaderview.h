#ifndef BUTTONHEADERVIEW_H
#define BUTTONHEADERVIEW_H

#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>

class ButtonHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    enum ButtonPosition {
        Column,
        Row
    };
    explicit ButtonHeaderView(Qt::Orientation orientation, ButtonPosition pos, QWidget *parent = 0);

signals:
    void pressed(int);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent *event);

private:

    ButtonPosition type;
    QPixmap* pixMap;
    QPushButton *btn;
};

#endif // BUTTONHEADERVIEW_H
