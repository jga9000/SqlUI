#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ButtonHeaderView.h"
#include "headereditor.h"

const QString windowTitleText = QString("TinyEdit");
const QString defaultHeaderText = QString("Unnamed");
const QString defaultUniqNamePrefix = QString("UnnamedGx67E9zb");
const QString defaultEmptyValue = QString("");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    border(1),
    ui(new Ui::MainWindow)
{
    headerEdit = 0;
    model = 0;
    modelCreated = false;
    loading = false;

    setWindowTitle(windowTitleText);
    sqlAPI = new TinySqlApi("", this);

    ui->setupUi(this);

    view = ui->mainTableView;
    ui->addRowButton->setVisible(false);
    ui->dimmer->setVisible(true);

    ui->dimmer->setStyleSheet( "background-color: rgba( 255, 255, 255, 90% );" );

    createNewTable(1, 2);

    loadStyleSheet("Gray");

    connect(sqlAPI, SIGNAL(tinySqlApiServiceInitialized(TinySqlApiServerError)),
            this, SLOT(on_sql_initialized(TinySqlApiServerError)) );

    connect(sqlAPI, SIGNAL(tinySqlApiRead(TinySqlApiServerError, QList< QList<QVariant> > )),
            this, SLOT(on_sql_readall(TinySqlApiServerError, QList<QList<QVariant> >)) );

    connect(sqlAPI, SIGNAL(tinySqlApiTablesRes(TinySqlApiServerError, QList<QVariant> )),
            this, SLOT(on_sql_tables(TinySqlApiServerError, QList<QVariant>)) );

    connect(sqlAPI, SIGNAL(tinySqlApiColumnsRes(TinySqlApiServerError, QList<QVariant> )),
            this, SLOT(on_sql_columns(TinySqlApiServerError, QList<QVariant>)) );

    connect(sqlAPI, SIGNAL(tinySqlApiItemCount(TinySqlApiServerError, int)),
            this, SLOT(on_sql_itemCount(TinySqlApiServerError, int)) );

    connect(model, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(on_dataChanged(QStandardItem*)));

    connect(headerEdit, SIGNAL(updateHeader(QString, int, Qt::Orientation)),
            this, SLOT(on_columnNameChanged(QString, int)));

    // Following will lead to readTables->count()->readAll procedure
    sqlAPI->readTables();
    view->setGeometry(QRect(10, 20, 500, 30));
}

void MainWindow::loadStyleSheet(const QString &sheetName)
{
    QFile file(":/qss/" + sheetName.toLower() + ".qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QString::fromLatin1(file.readAll());
    qApp->setStyleSheet(styleSheet);
}

void MainWindow::initCells(int initColumns)
{
    for (int i(0); i<initColumns; i++) {
        QStandardItem *item = new QStandardItem;
        item->setData("", Qt::DisplayRole);
        // This updates the default row values
        model->setItem(0, i, item);
        // This updates the default column values (2)
        model->setHorizontalHeaderItem(i, new QStandardItem(defaultHeaderText));
    }
    for (int i(0); i<model->rowCount(); i++) {
        model->setHeaderData(i, Qt::Vertical, QString("%1    ").arg(i+1));
    }
}
// TODO: remove existing model etc, before creating new one?
void MainWindow::createNewTable(int initRows, int initColumns)
{
    model = new QStandardItemModel(initRows, initColumns, this);

    // This adds close button to each column header
    ButtonHeaderView *tableColHeader = new ButtonHeaderView(Qt::Horizontal,
                                                            ButtonHeaderView::Column,
                                                            view);
    view->setHorizontalHeader(tableColHeader);

    initCells(initColumns);

    ButtonHeaderView *tableRowHeader = new ButtonHeaderView(Qt::Vertical,
                                                            ButtonHeaderView::Row,
                                                            view);
    view->setVerticalHeader(tableRowHeader);

    connect(tableColHeader, SIGNAL(pressed(int)),
            this, SLOT(on_columnDeleted(int)));
    connect(tableRowHeader, SIGNAL(pressed(int)),
            this, SLOT(on_rowDeleted(int)));

    view->setModel(model);

    // Following makes each column's header editable
    headerEdit = new HeaderTextEditor(view,
                                      ui->scrollArea->pos().x(),
                                      ui->scrollArea->pos().y(),
                                      border,
                                      this);
    headerEdit->hide();
    connect(view->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)),
            headerEdit, SLOT(xHeader(int)));
    connect(view->verticalHeader(), SIGNAL(sectionDoubleClicked(int)),
            headerEdit, SLOT(yHeader(int)));

    view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    view->setMinimumSize(view->geometry().size().width(),
                         view->geometry().size().height());

    view->setSortingEnabled(true);
    ui->addRowButton->move(view->geometry().right()-ui->addRowButton->size().width(),
                           view->geometry().bottom());


    ui->addColumnButton->move(view->geometry().right()-ui->addColumnButton->size().width(),
                              view->geometry().top());

    ui->dimmer->move(ui->scrollArea->pos());
    modelCreated = true;
}

MainWindow::~MainWindow()
{
    delete sqlAPI;
    delete ui;
}

void MainWindow::on_sql_initialized(TinySqlApiServerError error)
{
    if (error==NoError) {
        qDebug() << "on_sql_initialized, ok";
    }
    else{
        qDebug() << "on_sql_initialized, error:" << error;
    }
}

void MainWindow::on_addRowButton_clicked()
{
    int rows = model->rowCount();
    if (model) {
        model->insertRow(rows);
        updateGeometry();
    }
}

void MainWindow::on_addColumnButton_clicked()
{
    int columns = model->columnCount();
    model->insertColumn(columns);
    model->setHorizontalHeaderItem(columns, new QStandardItem(defaultHeaderText));

    recreateSqlTable();
    updateGeometry();
}

void MainWindow::on_dimmer_clicked()
{
    on_newTableButton_clicked();
}

void MainWindow::on_newTableButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "New table",
                                         "Name it:", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        ui->dimmer->setVisible(false);
        ui->addRowButton->setVisible(true);
        if(model) {
            model->clear();
            initCells(2);
            createSqlTable(text, true);
        }
    ui->comboBox->addItem(text);
    ui->comboBox->setCurrentText(text);
    }
}

void MainWindow::on_loadTableButton_clicked()
{
    qDebug() << "on_loadTableButton_clicked";

    ui->dimmer->setVisible(false);
    ui->addRowButton->setVisible(true);
    if (model) {
        model->clear();
    }
    QString currentText = ui->comboBox->currentText();
    ui->comboBox->setCurrentText(currentText);

    qDebug() << "Loading table:" << currentText;
    sqlAPI->setTable(currentText);

    loading = true;
    tableName = currentText;
    sqlAPI->readColumns();
}

void MainWindow::on_deleteButton_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("Selected table will be removed from database.");
    msgBox.setInformativeText(QString("Delete %1?").arg(ui->comboBox->currentText()));
    msgBox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok)
    {
        sqlAPI->deleteAll(ui->comboBox->currentText());

        qDebug() << "currentText:" << ui->comboBox->currentText();
        qDebug() << "tableName:" << tableName;

        if (ui->comboBox->currentText() == tableName) {
            ui->comboBox->removeItem(ui->comboBox->currentIndex());
            ui->comboBox->setCurrentText("");
            model->clear();
            ui->addRowButton->setVisible(false);
            ui->dimmer->setVisible(true);
        }
    }
}

void MainWindow::on_searchButton_clicked()
{
    qDebug() << "on_searchButton_clicked";
    bool ok;
    QString text = QInputDialog::getText(this, "Search",
                                         "Search item:",
                                         QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {

        // Copy values from original model
        int r(0);
        QList< QList<QStandardItem*> > foundColumns;
        for (int c(0); c<model->columnCount(); c++)
        {
            QList<QStandardItem*> foundItems = model->findItems(text,
                                                                Qt::MatchContains,
                                                                c);

            if (foundItems.count()>0) {
                foundColumns << foundItems;
                if (foundItems.count()>r) {
                    r = foundItems.count();
                }
            }
        }
        // Create new table showing results
        QStandardItemModel* resultsModel = new QStandardItemModel(r,
                                                                  foundColumns.count(),
                                                                  this);

        // This updates the column values (c)
        for (int c(0); c<foundColumns.count(); c++)
        {
            QList<QStandardItem*> foundItems = foundColumns.at(c);
            QStandardItem* item = model->item(0, foundItems.at(0)->column());
            qDebug() << "found column number" << foundItems.at(0)->column();
            QString headerString = model->headerData(foundItems.at(0)->column(),
                                                     Qt::Horizontal,
                                                     Qt::DisplayRole).toString();
            qDebug() << "header value" << headerString;
            resultsModel->setHorizontalHeaderItem(c,
                                                  new QStandardItem(headerString));
        }
        // This updates the row values
        r=0;
        int c(0);
        foreach(QList<QStandardItem*> foundItems, foundColumns)
        {
            foreach(QStandardItem* foundItem, foundItems)
            {
                QStandardItem *resultedItem = new QStandardItem();
                resultedItem->setData(foundItem->data(Qt::DisplayRole).toString(),
                                      Qt::DisplayRole);
                qDebug() << "row value" << resultedItem->data(Qt::DisplayRole).toString();
                resultsModel->setItem(r, c, resultedItem);

                qDebug() << QString("column:%1 row:%2").arg(c).arg(r);
                r++;
            }
            c++;
        }
        QTableView *miniView = new QTableView(view);
        miniView->setObjectName(QStringLiteral("Search results"));
        miniView->setGeometry(QRect(30, 40, 350, 200));
        miniView->setFrameShadow(QFrame::Sunken);
        miniView->setLineWidth(1);
        miniView->setMidLineWidth(1);
        miniView->setAutoScrollMargin(16);
        miniView->setProperty("showDropIndicator", QVariant(false));
        miniView->setShowGrid(true);
        miniView->setModel(resultsModel);

        miniView->setSizePolicy(QSizePolicy::MinimumExpanding,
                                QSizePolicy::MinimumExpanding);
        miniView->setMinimumSize(miniView->geometry().size().width(),
                                 miniView->geometry().size().height());
        miniView->setSortingEnabled(true);
        miniView->show();
        miniView->setFocus();

        QMessageBox msgBox;
        msgBox.setText(QString("Found %1 %2 times").arg(text).arg(foundColumns.count()));
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        delete miniView;
    }
}

void MainWindow::on_loadFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    if(fileName != "") {
        sqlAPI->changeDB(fileName);

        ui->comboBox->setCurrentText("");
        ui->comboBox->clear();
        model->clear();
        ui->addRowButton->setVisible(false);
        ui->dimmer->setVisible(true);
        view->reset();
        //view->setGeometry(QRect(10, 20, 500, 30));
        //createNewTable(1, 2);
        // Following will lead to readTables->count()->readAll procedure
        sqlAPI->readTables();
    }
}

void MainWindow::on_sql_itemCount(TinySqlApiServerError error, int count)
{
    if (error==NoError) {
        qDebug() << "on_sql_itemcount, count:" << count;
        totalRows = count;
        sqlAPI->readAll(totalColumns);
    }
    else{
        qDebug() << "on_sql_itemcount, error:" << error;
        loading = false;
    }
}

void MainWindow::on_sql_readall(TinySqlApiServerError error,
                                QList< QList<QVariant> > itemList)
{
    if (error==NoError) {
        qDebug() << "on_sql_readall, ok";

        foreach( QList<QVariant> columns, itemList ) {
            qDebug() << "new row, columns:" << columns.count();
            primaryKeys.append(columns.at(0));

            loadedColumn = 0;
            foreach( QVariant cell, columns ) {
                qDebug() << "cell:" << cell.toString();
                QStandardItem *singleItem = new QStandardItem();

                if (loadedColumn==0 && cell.toString().mid(0,
                                                     defaultUniqNamePrefix.length()) ==
                    defaultUniqNamePrefix) {
                    singleItem->setData("", Qt::DisplayRole);
                }
                else{
                    singleItem->setData(cell.toString(), Qt::DisplayRole);
                }
                qDebug() << QString("setData, row:%1, col:%2").arg(loadedRow).arg(loadedColumn);
                model->setItem(loadedRow, loadedColumn++, singleItem);
            }
            loadedRow++;
        }
    }
    else{
        qDebug() << "on_sql_readall, error:" << error;
    }
    if (loadedRow>=totalRows) {
        qDebug() << "everything read";
        loading = false;
    }
}

void MainWindow::on_sql_tables(TinySqlApiServerError error,
                               QList<QVariant> tables)
{
    if (error==NoError) {
        qDebug() << "on_sql_tables, ok";
        foreach( QVariant name, tables ) {
            qDebug() << "Adding table:" << name.toString();
            ui->comboBox->addItem(name.toString());
        }
    }
    else{
        qDebug() << "on_sql_tables, error:" << error;
    }
}

void MainWindow::on_sql_columns(TinySqlApiServerError error,
                                QList<QVariant> columns)
{
    if (error==NoError) {
        qDebug() << "on_sql_columns, ok";
        int row(0), column(0), count(0);
        // server reply contains values in 6 columns, every 2nd row has col name
        foreach (QVariant name, columns) {
            qDebug() << QString("index:%1, value:%2").arg(row*6+column).arg(name.toString());
            if (column == 1) {
                // In server reply every 2nd value contains name
                qDebug() << "new column name:" << name.toString();

                if (count==0) {
                    qDebug() << "Setting primary key value";
                    sqlAPI->setPrimaryKey(name.toString());
                }
                // Check default text, do not show it
                if (name.toString().mid(0, defaultUniqNamePrefix.length()) ==
                    defaultUniqNamePrefix) {
                    model->setHorizontalHeaderItem(count,
                                                   new QStandardItem(defaultHeaderText));
                }
                else{
                    model->setHorizontalHeaderItem(count,
                                                   new QStandardItem(name.toString()));
                }
                count++;
            }
            column++;
            if (column>5) {
                row++;
                column=0;
            }
        }
        totalColumns=count;
        qDebug() << "Columns now:" << count;
        primaryKeys.clear();
        loadedColumn=0;
        loadedRow=0;
        sqlAPI->count();
    }
    else{
        qDebug() << "on_sql_columns, error:" << error;
    }
}

void MainWindow::on_columnDeleted(int column)
{
    qDebug() << "on_columnDeleted, column:" << column;
    model->removeColumn(column);
    if (model->columnCount()==0) {
        model->clear();
        ui->addRowButton->setVisible(false);
        ui->dimmer->setVisible(true);
    }
    recreateSqlTable();
}

void MainWindow::on_rowDeleted(int row)
{
    qDebug() << "on_rowDeleted, row:" << row;
    QVariant data = rowData(0, row);

    sqlAPI->deleteItem(data);
    model->removeRow(row);
    if (model->rowCount()==0) {
        model->clear();
        ui->addRowButton->setVisible(false);
        ui->dimmer->setVisible(true);
    }
}

void MainWindow::on_columnNameChanged(QString newValue, int index)
{
    qDebug() << QString("on_columnNameChanged, index:%1, value:%2").arg(index).arg(newValue);
    model->setHorizontalHeaderItem(index,
                                   new QStandardItem(newValue));
    qDebug() << columnData(index);
    recreateSqlTable();
}

void MainWindow::on_dataChanged(QStandardItem* item)
{
    if (loading){
        return;
    }
    qDebug() << "on_dataChanged:" << QString("Row %1, Column %2")
               .arg(item->index().row())
               .arg(item->index().column());

    QList<QVariant> changedRow;
    int r = item->index().row();
    bool changed = false;

    for( int c(0); c<model->columnCount(); c++)
    {
        QStandardItem* item = model->item(r, c);
        if (!item || item->data(Qt::DisplayRole) == "")
        {
            if (c==0) {
                qDebug() << "Empty primary key changed to default";
                changedRow.append(QString("%1%2").arg(defaultUniqNamePrefix).arg(r));
            }
            else{
                qDebug() << "(!item)";
                changedRow.append(defaultEmptyValue);
            }
        }
        else
        {
            QVariant data = rowData(c, r);
            qDebug() << "value:" << data.toString();
            changedRow.append(data);
            changed = true;
        }
    }
    if (changed) {
        if (item->index().column() == 0 &&
            primaryKeys.count() > item->index().row() &&
            primaryKeys.at(item->index().row()) != "" ) {
            qDebug() << "primary key changed, deleting row first";
            sqlAPI->deleteItem(primaryKeys.at(item->index().row()));
            primaryKeys.removeAt(item->index().row());
        }
        primaryKeys.insert(item->index().row(), item->data());
        qDebug() << "writing changed row";
        QVariant variantList(changedRow);
        sqlAPI->writeItem(variantList);
    }
}

QVariant MainWindow::rowData(int column, int row)
{
    QStandardItem* item = model->item(row, column);
    if(!item) {
        return QString(defaultEmptyValue);
    }
    QVariant rowData(item->data(Qt::DisplayRole));

    if (rowData.toString() == "")
    {
        if (column==0) {
            // Primary key shall never be empty, use unique string
            rowData = QString("%1%2").arg(defaultUniqNamePrefix).arg(row);
        }
    }

    qDebug() << QString("rowData, column %1 row %2 value:%3").arg(column).arg(row).arg(rowData.toString());
    return rowData;
}

QVariant MainWindow::columnData(int column)
{
    QVariant columnData(model->headerData(column, Qt::Horizontal, Qt::DisplayRole));
    if (columnData.toString() == "" || columnData.toString() == defaultHeaderText)
    {
        qDebug() << QString("Column currently:%1 (changing to default value)")
                    .arg(columnData.toString());
        columnData = QString("%1%2").arg(defaultUniqNamePrefix).arg(column);
    }
    qDebug() << QString("columnData, column %1 value:%2").arg(column).arg(columnData.toString());
    return columnData;
}

void MainWindow::createSqlTable(QString name, bool createEmpty)
{
    qDebug() << "createSqlTable:" << name;
    // Identifier is first row and column value
    TinySqlApiInitializer identifier(columnData(0).type(),
                                     columnData(0).toString(),
                                     255);
    qDebug() << "id name:" << identifier.name();

    QList<TinySqlApiInitializer> initializers;

    // Initializers should contain every columns in first row (except first)
    if( createEmpty)
    {
        TinySqlApiInitializer initializer1(columnData(1).type(),
                                           columnData(1).toString(), 255);
        initializers.append(initializer1);
        qDebug() << "initializer1.name:" << initializer1.name();
    }
    else
    {
        for( int c(1); c<model->columnCount(); c++) {
            TinySqlApiInitializer initializer(columnData(c).type(),
                                              columnData(c).toString(), 255);
            initializers.append(initializer);
            qDebug() << QString("initializer(%1).name:%2").arg(c).arg(initializer.name());
        }
    }

    if (name != "") {
        tableName = name;
        sqlAPI->setTable(name);
    }

    sqlAPI->initialize(identifier, initializers);
}

void MainWindow::recreateSqlTable()
{
    qDebug() << "recreateSqlTable:" << tableName;
    sqlAPI->deleteAll();

    createSqlTable(tableName, false);

    // Write every item back again
    for( int r=0; r<model->rowCount(); r++) {
        QList<QVariant> row;
        for( int c(0); c<model->columnCount(); c++) {
            qDebug() << "appending one";
            row.append(rowData(c, r));
        }
        QVariant variantList(row);
        qDebug() << "writing values";
        sqlAPI->writeItem(variantList);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();

    int marg = view->x();

    ui->scrollArea->resize(size.width(),
                           size.height());
    ui->scrollArea->move(0,50);

    view->resize( size.width(),
                  size.height()-100 );
    view->move(0, 0);

    ui->dimmer->resize(ui->scrollArea->width(),
                       ui->scrollArea->height());
    ui->dimmer->move(0, 50);

    ui->addRowButton->move(view->geometry().right()-ui->addRowButton->size().width()-100,
                          view->geometry().bottom()-55);

    ui->addColumnButton->move(view->geometry().right()-ui->addColumnButton->size().width()-20,
                          view->geometry().top()+37);
    QWidget::resizeEvent(event);
}
