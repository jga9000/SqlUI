#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "sql\tinysqliteapi.h"

class QTableView;
class HeaderTextEditor;
class QStandardItemModel;
class QStandardItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_sql_itemCount(TinySqlApiServerError error, int count);

private slots:
    void on_addRowButton_clicked();
    void on_addColumnButton_clicked();
    void on_dimmer_clicked();
    void on_newTableButton_clicked();
    void on_loadTableButton_clicked();
    void on_dataChanged(QStandardItem*);
    void on_columnDeleted(int);
    void on_rowDeleted(int);
    void on_sql_initialized(TinySqlApiServerError error);
    void on_sql_readall(TinySqlApiServerError error, QList< QList<QVariant> > itemList);
    void on_sql_tables(TinySqlApiServerError error, QList<QVariant> tables);
    void on_sql_columns(TinySqlApiServerError error, QList<QVariant> columns);
    void on_columnNameChanged(QString newValue, int index);
    void on_deleteButton_clicked();
    void on_searchButton_clicked();
    void on_loadFileButton_clicked();

private:
    void createNewTable(int rows, int columns);
    void initCells(int initColumns);
    void createSqlTable(QString name, bool createEmpty);
    void recreateSqlTable();
    QVariant rowData(int column, int row);
    QVariant columnData(int column);
    void resizeEvent(QResizeEvent *event);
    void loadStyleSheet(const QString &sheetName);

private:
    QTableView *view;
    HeaderTextEditor *headerEdit;
    QStandardItemModel *model;
    Ui::MainWindow *ui;
    TinySqlApi *sqlAPI;
    QList<QVariant> primaryKeys;
    int border;
    bool modelCreated;
    bool rowsChanged;
    bool loading;
    int totalColumns;
    int totalRows;
    QString tableName;
    int loadedRow;
    int loadedColumn;
};

#endif // MAINWINDOW_H
