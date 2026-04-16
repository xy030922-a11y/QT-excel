#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spreadsheetmodel.h"
#include "celldelegate.h"

#include <QTableView>
#include <QMenuBar>
#include <QMenu>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model=new SpreadsheetModel(50,50,this);
    m_view=new QTableView(this);
    m_view->setModel(model);
    m_view->setItemDelegate(new CellDelegate(this));

    m_view->horizontalHeader()->setDefaultSectionSize(150);
    m_view->verticalHeader()->setDefaultSectionSize(50);
    m_view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_view->setGridStyle(Qt::SolidLine);

    setCentralWidget(m_view);

    tb=new ToolbarManger(this,this);
    tb->setupToolbar();


//------------------------------------------------------------------------------------
    connect(tb,&ToolbarManger::copyRequested,this,&MainWindow::onCopy);
    connect(tb,&ToolbarManger::cutRequested,this,&MainWindow::onCut);
    connect(tb,&ToolbarManger::pasteRequested,this,&MainWindow::onPaste);

    connect(tb,&ToolbarManger::boldToggled,this,[this](bool v){
        model->setBold(m_view->selectionModel()->selectedIndexes(),v);
    });
    connect(tb,&ToolbarManger::italicToggled,this,[this](bool v){
        model->setItalic(m_view->selectionModel()->selectedIndexes(),v);
    });
    connect(tb,&ToolbarManger::underlineToggled,this,[this](bool v){
        model->setUnderline(m_view->selectionModel()->selectedIndexes(),v);
    });
    connect(tb,&ToolbarManger::fontFamilyChanged,this,[this](const QString& f){
        model->setFontFamily(m_view->selectionModel()->selectedIndexes(),f);
    });
    connect(tb,&ToolbarManger::fontSizeChanged,this,[this](int pt){
        model->setFontSize(m_view->selectionModel()->selectedIndexes(),pt);
    });
    connect(tb,&ToolbarManger::fontColorChanged,this,[this](const QColor& c){
        model->setFontColor(m_view->selectionModel()->selectedIndexes(),c);
    });
    connect(tb,&ToolbarManger::bgColorChanged,this,[this](const QColor& c){
        model->setBgColor(m_view->selectionModel()->selectedIndexes(),c);
    });
    connect(tb,&ToolbarManger::alignChanged,this,[this](Qt::Alignment a){
        model->setAlignment(m_view->selectionModel()->selectedIndexes(),a);
    });

    connect(m_view->selectionModel(),&QItemSelectionModel::currentChanged,
            this,&MainWindow::onCurrentChanged);
//------------------------------------------------------------------------------------

    setupHeaderContextMenus();
    setupFileMenu();
    setCurrentFile(QString());
    resize(800,600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QModelIndexList MainWindow::selectedIndexes() const {
    return m_view->selectionModel()->selectedIndexes();
}
//当选中单元格变动时     toolbar显示对应的字体样式
void MainWindow::onCurrentChanged(const QModelIndex& current, const QModelIndex&) {
    if(!current.isValid())  return;
    const CellFormat fmt=model->formatAt(current);

    QSignalBlocker b1(tb->actBold),b2(tb->actItalic),b3(tb->actUnderline);
    QSignalBlocker b4(tb->fontCombo),b5(tb->sizeCombo);

    tb->actBold->setChecked(fmt.bold);
    tb->actItalic->setChecked(fmt.italic);
    tb->actUnderline->setChecked(fmt.underline);
    tb->fontCombo->setCurrentFont(fmt.font);
    tb->sizeCombo->setCurrentText(QString::number(fmt.font.pointSize() > 0
                                                         ? fmt.font.pointSize() : 11));
}
//用来接受 insert 行/列 数
int  MainWindow::askCount(const QString& title, const QString& label, int defaultVal ){
    bool ok;
    int n=QInputDialog::getInt(this,title,label,defaultVal,1,500,1,&ok);
    return ok? n : 0;
}
//右键菜单
void MainWindow::setupHeaderContextMenus(){
    m_view->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_view->verticalHeader(),&QWidget::customContextMenuRequested,
            this,&MainWindow::onRowHeaderContextMenu);

    m_view->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_view->horizontalHeader(),&QWidget::customContextMenuRequested,
            this,&MainWindow::onColHeaderContextMenu);

}
//行头    右键
void MainWindow::onRowHeaderContextMenu(const QPoint& pos){
    int row=m_view->verticalHeader()->logicalIndexAt(pos);
    if(row<0)   return;

    auto* selModel=m_view->selectionModel();
    QSet<int> rowSet;
    for(const auto& idx : selModel->selectedRows()){
        rowSet.insert(idx.row());
    }
    //如果右键行不在选中集合   只操作右键行
    if(!rowSet.contains(row))   rowSet={row};
    QList<int>  rowList(rowSet.begin(),rowSet.end());
    std::sort(rowList.begin(),rowList.end(),std::greater<int>());

    const int selCount=rowList.size();
    const QString rowDesc= selCount>1 ?
                            tr("%1 行").arg(selCount)
                            :tr("第 %1 行").arg(selCount);

    QMenu menu(this);
//  右键修改行宽
    if(selCount>0){
        menu.addAction(tr("设置行高： "),[this,&rowList]{
          bool ok;
          int h=QInputDialog::getInt(this,tr("设置行高"),
                                     tr("行高（像素）： "),
                                     m_view->verticalHeader()->sectionSize(rowList.first()),
                                     5,500,1,&ok);
          if(ok){
              for(const auto& row : rowList){
                  m_view->verticalHeader()->resizeSection(row,h);
              }
          }
        });
        menu.addSeparator();
    }
//insert
    menu.addAction(tr("在上方插入行："),[this,row]{
        int n=askCount(tr("向上插入行"),tr("插入行数： ")) ;
        if(n>0) model->insertRows(row,n);
    });
    menu.addAction(tr("在下方插入行："),[this,row]{
        int n=askCount(tr("向下插入行"),tr("插入行数： ")) ;
        if(n>0) model->insertRows(row+1,n);
    });
    menu.addSeparator();
//delete
    menu.addAction(tr("删除 %1").arg(rowDesc),[this,&rowList]{
        for(auto r: rowList){
            model->removeRows(r,1);
        }
    });
    menu.exec(m_view->verticalHeader()->viewport()->mapToGlobal(pos));
}
//列头    右键
void MainWindow::onColHeaderContextMenu(const QPoint& pos){
    int col=m_view->horizontalHeader()->logicalIndexAt(pos);
    if(col<0)   return;

    auto* selModel=m_view->selectionModel();
    QSet<int>   colSet;
    for(const auto& idx : selModel->selectedColumns()){
        colSet.insert(idx.column());
    }
    if(!colSet.contains(col))    colSet={col};
    QList<int>  colList(colSet.begin(),colSet.end());
    std::sort(colList.begin(),colList.end(),std::greater<int>());

    const int selCount=colList.size();
    const QString colDesc=selCount > 1
            ? tr("%1 列").arg(selCount)
            : tr("第 %1 列").arg(col + 1);

    QMenu menu(this);

    if(selCount>0){
        menu.addAction(tr("设置列宽"), [this,&colList]{
            bool ok;
            int w=QInputDialog::getInt(this, tr("设置列宽"),tr("列宽（像素）："),m_view->horizontalHeader()->sectionSize(colList.first()),5, 1000, 1, &ok);
            if(ok){
                for(const auto& col : colList){
                    m_view->horizontalHeader()->resizeSection(col,w);
                }
            }
        });
        menu.addSeparator();
    }

// 向左插入列
       menu.addAction(tr("向左插入列..."), [this, col]{
           int n = askCount(tr("向左插入列"), tr("插入列数："));
           if(n > 0) model->insertCols(col, n);
       });

// 向右插入列
       menu.addAction(tr("向右插入列..."), [this, col]{
           int n = askCount(tr("向右插入列"), tr("插入列数："));
           if(n > 0) model->insertCols(col + 1, n);
       });

       menu.addSeparator();

// 删除选中列（从大到小，避免索引偏移）
       menu.addAction(tr("删除 %1").arg(colDesc), [this, colList]{
           for(int c : colList)
               model->removeCols(c, 1);
       });

       menu.exec(m_view->horizontalHeader()->viewport()->mapToGlobal(pos));


}



//-------------------------------------------------------------------
void MainWindow::onCopy(){
    auto indexes=m_view->selectionModel()->selectedIndexes();
    if(indexes.isEmpty())   return;

    std::sort(indexes.begin(),indexes.end(),
             [](const QModelIndex& a,const QModelIndex& b){
                return a.row()!=b.row() ? a.row()<b.row() : a.column()<b.column();
    });

    auto cRow=indexes.first().row();
    auto lastCol=indexes.first().column();
    QString text;
    for(const auto& idx : indexes){
        if(idx.row()!=cRow){
            text+="\n";
            cRow=idx.row();
            lastCol=idx.column();
        }
        else if(idx.column()!=lastCol){
            text+="\t";
        }
        text+=idx.data(Qt::DisplayRole).toString();
    }

    QApplication::clipboard()->setText(text);
    m_clipText = text;   // 同时存内部缓冲（paste 用）

}
void MainWindow::onCut(){
    onCopy();

    for(const auto& idx : m_view->selectionModel()->selectedIndexes()){
        model->setData(idx,QString(),Qt::EditRole);
    }
}
void MainWindow::onPaste(){
    const QString text=QApplication::clipboard()->text();

    QModelIndex topLeft=m_view->currentIndex();
    if(!topLeft.isValid())  return;

    int startRow=topLeft.row();
    int startCol=topLeft.column();

    const QStringList rows=text.split("\n");
    for(int r=0;r<rows.size();r++){
        const QStringList cols=rows[r].split("\t");
        for(int c=0;c<cols.size();c++){
            auto idx=model->index(startRow+r,startCol+c);
            if(idx.isValid()){
                model->setData(idx,cols[c],Qt::EditRole);
            }
        }
    }
}
//-------------------------------------------------------------------
// File 菜单
void MainWindow::setupFileMenu(){
    QMenu* fileMenu=menuBar()->addMenu(tr("File"));

    auto* actNew=new QAction(tr("New File"),this);
    connect(actNew,&QAction::triggered,this,&MainWindow::onNew);

    auto* actOpen=new QAction(tr("Open File"),this);
    connect(actOpen,&QAction::triggered,this,&MainWindow::onOpen);

    auto* actSave=new QAction(tr("Save File"),this);
    actSave->setShortcut(QKeySequence::Save);
    connect(actSave,&QAction::triggered,this,&MainWindow::onSave);

    auto* actSaveas=new QAction(tr("Save as"),this);
    connect(actSaveas,&QAction::triggered,this,&MainWindow::onSaveAs);

    fileMenu->addAction(actNew);
    fileMenu->addAction(actOpen);
    fileMenu->addAction(actSave);
    fileMenu->addAction(actSaveas);
}
//title
void MainWindow::setCurrentFile(const QString& path){
    m_currentFile=path;
    QString title=path.isEmpty() ? tr("NewExcel") : QFileInfo(path).fileName();
    setWindowTitle(title + "--QT Excel");

}

void MainWindow::onNew(){
    auto choice=QMessageBox::question(this,tr("New File"),
                                      tr("Clear All and New File"),
                                        QMessageBox::Yes | QMessageBox::No);
    if(choice!=QMessageBox::Yes)    return;
    model->clearAll();
    resetViewSizes();
    setCurrentFile(QString());

}
void MainWindow::onOpen(){
    QString path=QFileDialog::getOpenFileName(this,tr("Open File"),QString(),
                                              tr("CSV File (*.csv);;All File (*)"));
    if(path.isEmpty())  return;

    if(!model->loadCSV(path)){
        QMessageBox::warning(this,tr("Error"),tr("Can not load file:\n")+path);
        return;
    }
    resetViewSizes();
    setCurrentFile(path);
}
void MainWindow::onSave(){
    if(m_currentFile.isEmpty()){
        onSaveAs();
        return;
    }
    if(!model->saveCSV(m_currentFile)){
        QMessageBox::warning(this,tr("Error"),tr("Save False:\n")+m_currentFile);
        return;
    }
}
void MainWindow::onSaveAs(){
    QString path=QFileDialog::getSaveFileName(
                this,tr("Save As"),
                QString(),
                tr("CSV File (*.csv);;All File (*)"));
    if(path.isEmpty())  return;

    if(!path.endsWith(".csv",Qt::CaseInsensitive)){
        path+=".csv";
    }
    if(!model->saveCSV(path)){
        QMessageBox::warning(this,tr("Error"),tr("Save As False:\n")+m_currentFile);
        return;
    }
}
//重置行宽 列宽
void MainWindow::resetViewSizes(){
    constexpr int col_w=150,row_h=50;
    auto* hh=m_view->horizontalHeader();
    auto* vh=m_view->verticalHeader();
    hh->setDefaultSectionSize(col_w);
    vh->setDefaultSectionSize(row_h);
    for(int c=0;c<model->columnCount();c++){
        hh->resizeSection(c,col_w);
    }
    for(int r=0;r<model->rowCount();r++){
        vh->resizeSection(r,row_h);
    }
}
