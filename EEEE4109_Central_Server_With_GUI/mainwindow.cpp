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


    connect(_ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeViewClicked);

    _ui->PoleStatusViewSection->setCurrentIndex(0);

}


MainWindow::~MainWindow()
{
    delete _ui;
    delete _poleList;
}


void MainWindow::handleTreeViewClicked() {

    //Deselect the current pole.
    if (_currentlySelectedPole != nullptr) _currentlySelectedPole->setUISelection(false);

    // Update the view pannel to the correct UI elements for this pole type.
    QModelIndex selectedIndex = _ui->treeView->currentIndex();
    _ui->PoleStatusViewSection->setCurrentIndex(1);

    Pole* pPole = static_cast<Pole*>(selectedIndex.internalPointer());

    if (pPole->getPoleType() == (uint8_t)LEDPole)   _ui->PoleDiagramView->setCurrentIndex(0);
    if (pPole->getPoleType() == 2)  _ui->PoleDiagramView->setCurrentIndex(1);

    pPole->setUISelection(true);

    std::string newPageTitle = "Pole " + std::to_string(pPole->getPoleSessionID());
    _ui->label_4->setText(QString(newPageTitle.c_str()));

    // Save the currently selected pole.
    _currentlySelectedPole = pPole;

    // Connect signals to slots :)

    connect(pPole, &Pole::VisualisePoleType, _ui->PoleTypeBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualisePoleHWID, _ui->PoleHWIDBox, &QLineEdit::setText);

    connect(pPole, &Pole::VisualisePolePartner, _ui->PolePartnerBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualisePoleBattery, _ui->PoleBatteryBox, &QLineEdit::setText);

    connect(pPole, &Pole::VisualisePolePosition, _ui->PolePositionBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualiseIRBeamFrequency, _ui->PoleIRFrequencyInputBox, &QLineEdit::setText);

    connect(pPole, &Pole::VisualiseTouchSensitivity, _ui->TouchSensitivityInputBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualiseIMUSensitivity, _ui->IMUSensitivityInputBox, &QLineEdit::setText);


    
    connect(_ui->PolePositionBox, &QLineEdit::returnPressed, this, &MainWindow::handlePolePositionInput);
    connect(_ui->PoleIRFrequencyInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleIRFrequencyInput);

    connect(_ui->TouchSensitivityInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleTouchSensitivityInput);
    connect(_ui->IMUSensitivityInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleIMUSensitivityInput);
    
    
    //_ui->IMUSensitivityInputBox->setText()
}

