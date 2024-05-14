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
    connect(_ui->PoleIRFrequencyInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleIRFrequencyInput);

    connect(_ui->TouchSensitivityInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleTouchSensitivityInput);
    connect(_ui->IMUSensitivityInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleIMUSensitivityInput);

    connect(_ui->FindGateButton, &QPushButton::clicked, this, &MainWindow::handleFindPolePartnerButtonClicked);
    connect(_ui->GateNumberInputBox, &QLineEdit::returnPressed, this, &MainWindow::handleGateNumberInput);

    _ui->PoleStatusViewSection->setCurrentIndex(0);


    FreeConsole();

    // create a separate new console window
    AllocConsole();

    // attach the new console to this application’s process
    AttachConsole(GetCurrentProcessId());

    SetConsoleOutputCP(65001);

    // reopen the std I/O streams to redirect I/O to the new console
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    freopen("CON", "r", stdin);
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

    //throw std::runtime_error("");

    Pole* pPole = static_cast<Pole*>(selectedIndex.internalPointer());

    if (pPole->getPoleType() == (uint8_t)LEDPole)   _ui->PoleDiagramView->setCurrentIndex(0);
    if (pPole->getPoleType() == 2)  _ui->PoleDiagramView->setCurrentIndex(1);

    std::string newPageTitle = "Pole " + std::to_string(pPole->getPoleSessionID());
    _ui->label_4->setText(QString(newPageTitle.c_str()));

    _ui->PassageDirectionIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Grey_Circle.png"));
    _ui->PoleKnockedIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Grey_Circle.png"));
    _ui->VelostatTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Grey_Circle.png"));
    _ui->IRBeamTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Grey_Circle.png"));
    _ui->PowerStateIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Grey_Circle.png"));
    _ui->IRCameraTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Grey_Circle.png"));

    // Grey out the Gate configuration options if the selected pole does not have an assigned partner.
    (pPole->getGate() == nullptr) ? _ui->GateNumberInputBox->setDisabled(true) : _ui->GateNumberInputBox->setDisabled(false);
    (pPole->getGate() == nullptr) ? _ui->GatePassageDirectionSelector->setDisabled(true) :   _ui->GatePassageDirectionSelector->setDisabled(false);

    //throw std::runtime_error("");

    // Save the currently selected pole.
    _currentlySelectedPole = pPole;

    // Connect signals to slots :)

    connect(pPole, &Pole::VisualisePoleType, _ui->PoleTypeBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualisePoleHWID, _ui->PoleHWIDBox, &QLineEdit::setText);

    //throw std::runtime_error("");

    connect(pPole, &Pole::VisualisePolePartner, _ui->PolePartnerBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualisePoleBattery, _ui->PoleBatteryBox, &QLineEdit::setText);

    //throw std::runtime_error("");

    connect(pPole, &Pole::VisualisePoleGateNumber, _ui->PoleGateNumberBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualiseIRBeamFrequency, _ui->PoleIRFrequencyInputBox, &QLineEdit::setText);

    //throw std::runtime_error("");

    connect(pPole, &Pole::VisualiseTouchSensitivity, _ui->TouchSensitivityInputBox, &QLineEdit::setText);
    connect(pPole, &Pole::VisualiseIMUSensitivity, _ui->IMUSensitivityInputBox, &QLineEdit::setText);

    //throw std::runtime_error("");

    connect(pPole, &Pole::updatePoleEventsVisualIndicators, this, &MainWindow::updatePoleEventsVisualIndicators);
    connect(pPole, &Pole::setPowerStateIndicator, this, &MainWindow::setPowerStateIndicator);

    //throw std::runtime_error("");

    // Tell the selected pole to set things up.
    pPole->setUISelection(true);

    //throw std::runtime_error("");

}

void MainWindow::updatePoleEventsVisualIndicators(events eventsData) {

    if (_currentlySelectedPole->getPoleType() == LEDPole) {
        (eventsData & IRCameraTriggered) ? _ui->IRCameraTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Green_Circle.png")) : _ui->IRCameraTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Red_Circle.png"));

        (eventsData & VelostatTriggered) ? _ui->VelostatTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Green_Circle.png")) : _ui->VelostatTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Red_Circle.png"));

        (eventsData & Knocked) ? _ui->PoleKnockedIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Green_Circle.png")) : _ui->PoleKnockedIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Red_Circle.png"));

        if (eventsData & KayakerPassageDirectionLTR) _ui->PassageDirectionIndicator->setPixmap(QPixmap(":/Events/Green_Up_Arrow.png"));
        if (eventsData & KayakerPassageDirectionRTL) _ui->PassageDirectionIndicator->setPixmap(QPixmap(":/Events/Green_Down_Arrow.png"));
        if (eventsData & KayakerPassageDirectionNone) _ui->PassageDirectionIndicator->setPixmap(QPixmap(":/Events/Grey_Circle.png"));
    }

    if (_currentlySelectedPole->getPoleType() == PhotoDiodePole) {
        (eventsData & IRBeamTriggered) ? _ui->IRBeamTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Green_Circle.png")) : _ui->IRBeamTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Red_Circle.png"));

        (eventsData & VelostatTriggered) ? _ui->VelostatTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Green_Circle.png")) : _ui->VelostatTriggeredIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Red_Circle.png"));

        (eventsData & Knocked) ? _ui->PoleKnockedIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Green_Circle.png")) : _ui->PoleKnockedIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Red_Circle.png"));
    }
}

void MainWindow::setPowerStateIndicator(pps::PolePowerState powerState) {
    

    if (powerState & pps::HighPower) {
        _ui->PowerStateIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/High_Icon.png")); return;
    }
    if (powerState & pps::MediumPower) {
        _ui->PowerStateIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Medium_Icon.png")); return;
    }
    if (powerState == pps::Hibernating) {
        _ui->PowerStateIndicator->setPixmap(QPixmap(":/EventsIndicatorIcons/Sleep_Icon.png")); return;
    }
}

void MainWindow::handleIRFrequencyInput() {
    if (_currentlySelectedPole != nullptr) _currentlySelectedPole->setIRFrequency(_ui->PoleGateNumberBox->text().toInt());
}

void MainWindow::handleTouchSensitivityInput() {
    if (_currentlySelectedPole != nullptr) _currentlySelectedPole->setVelostatSensitivity(_ui->VelostatTriggeredIndicator->text().toInt());
}

void MainWindow::handleIMUSensitivityInput() {
    if (_currentlySelectedPole != nullptr) _currentlySelectedPole->setIMUSensitivity(_ui->IMUSensitivityInputBox->text().toInt());
}

void MainWindow::handleGateNumberInput() {
    
    if (_currentlySelectedPole->getGate() != nullptr)  _currentlySelectedPole->getGate()->setGatePosition(_ui->GateNumberInputBox->text().toInt());

}