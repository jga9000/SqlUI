// This makes each column's header editable

#include <QTableView>
#include <QPoint>
#include <QHeaderView>

#include "headereditor.h"

HeaderTextEditor::HeaderTextEditor(QTableView *tableView,
                                 int topx,
                                 int topy,
                                 int border,
                                 QWidget *parent) :
    QLineEdit(parent),
    view(tableView),
    topx(topx),
    topy(topy),
    border(border)
{
    //QValidator *validator = new QDoubleValidator(this);
    //setValidator(validator);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, SIGNAL(returnPressed()), this, SLOT(headerValue()));
}

void HeaderTextEditor::setLabelValue(Qt::Orientation orientation, int section, QString value)
{
    setText(value);
    curOrientation = orientation;
    curSection = section;
}

void HeaderTextEditor::headerValue()
{
    //double value = text().toDouble();
    emit updateHeader(text(), curSection, curOrientation);
}

void HeaderTextEditor::keyPressEvent ( QKeyEvent * ev )
{
    if (ev->key() == Qt::Key_Escape)
    {
        hide();
    }
    QLineEdit::keyPressEvent(ev);
}

void HeaderTextEditor::focusOutEvent ( QFocusEvent * event )
{
    hide();
    QLineEdit::focusOutEvent(event);
}


void HeaderTextEditor::xHeader(int section)
{
    QPoint posHeaderX = QPoint(view->horizontalHeader()->geometry().x(),
                        view->horizontalHeader()->geometry().y());
    QPoint posStart = QPoint(topx, topy);
    QPoint posView = QPoint(view->geometry().x(), view->geometry().y());
    QPoint posSection = QPoint(view->horizontalHeader()->sectionPosition(section),
                        -border);
    showHeaderEditBox(section, Qt::Horizontal, QRect(posSection + posHeaderX + posStart + posView,
                                               QSize(view->horizontalHeader()->sectionSize(section),
                                               view->horizontalHeader()->height())));
}

void HeaderTextEditor::yHeader(int section)
{
    QPoint posHeaderY = QPoint(view->verticalHeader()->geometry().x(),
                        view->verticalHeader()->geometry().y());
    QPoint posStart = QPoint(topx, topy);
    QPoint posView = QPoint(view->geometry().x(), view->geometry().y());
    QPoint posSection = QPoint(0,
                        view->verticalHeader()->sectionPosition(section)-border);
    showHeaderEditBox(section, Qt::Vertical, QRect(posSection + posHeaderY + posStart + posView,
                                             QSize(view->verticalHeader()->width(),
                                             view->verticalHeader()->sectionSize(section))));
}

void HeaderTextEditor::updateData(QString newValue, int section, Qt::Orientation orientation)
{
    view->model()->setHeaderData(section, orientation, newValue, Qt::DisplayRole);
    hide();
}

void HeaderTextEditor::showHeaderEditBox(int section, Qt::Orientation orientation, QRect rectSection)
{
    QString curValue = QVariant(view->model()->headerData(section, orientation, Qt::DisplayRole)).toString();
    setGeometry(rectSection);
    setLabelValue(orientation, section, curValue);
    connect(this, SIGNAL(updateHeader(QString, int, Qt::Orientation)), this, SLOT(updateData(QString, int, Qt::Orientation)));
    show();
    setFocus();
    selectAll();
}
