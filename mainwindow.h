#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include<QTableView>
#include"spreadsheetmodel.h"
#include "toolbarManger.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCopy();
    void onCut();
    void onPaste();

    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();


    void onCurrentChanged(const QModelIndex& current, const QModelIndex&);


    void onRowHeaderContextMenu(const QPoint& pos);
    void onColHeaderContextMenu(const QPoint& pos);


private:
    void setupFileMenu();
    void setCurrentFile(const QString& path);
    void resetViewSizes();
    void setupHeaderContextMenus();
    int  askCount(const QString& title, const QString& label, int defaultVal = 1);


    QModelIndexList selectedIndexes() const;

    Ui::MainWindow *ui;
    SpreadsheetModel* model;
    QTableView*       m_view  = nullptr;
    //SpreadsheetModel* m_model = nullptr;
    ToolbarManger*         tb      = nullptr;
    QString           m_clipText;          // 内部剪贴板缓冲
    QString           m_currentFile;

};
#endif // MAINWINDOW_H
