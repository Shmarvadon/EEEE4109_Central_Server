#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    /*          Set up the UI stuffs            */    
    this->_poleList = new PoleDataModel( 42069, {2000,3000});

    _ui->treeView->setModel(this->_poleList);
    _ui->treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    _ui->treeView->setStyleSheet("QTreeView::item { border: none ; border-style: solid ; border-color: lightgray ;}");
}


MainWindow::~MainWindow()
{
    delete _ui;
    delete _poleList;
}
