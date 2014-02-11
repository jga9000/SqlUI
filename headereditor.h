#ifndef MYEDITHEADERBOX_H
#define MYEDITHEADERBOX_H

#include <QtGui>
#include <QLineEdit>

class QTableView;

class HeaderTextEditor : public QLineEdit
{
Q_OBJECT

public:
    HeaderTextEditor(QTableView *tableView, int topx, int topy, int border, QWidget *parent = 0);
    void setLabelValue(Qt::Orientation orientation, int section, QString value);

protected slots:
    void headerValue();
    void xHeader(int section);
    void yHeader(int section);
    void updateData(QString newValue, int section, Qt::Orientation orientation);

signals:
    void updateHeader(QString newValue, int section, Qt::Orientation o);

protected:
    void keyPressEvent(QKeyEvent *ev);
    void focusOutEvent(QFocusEvent *event);
    void showHeaderEditBox(int section, Qt::Orientation orientation, QRect rectSection);

private:
    Qt::Orientation curOrientation;
    int curSection;
    QTableView* view;
    int topx;
    int topy;
    int border;
};

#endif // MYEDITHEADERBOX_H
