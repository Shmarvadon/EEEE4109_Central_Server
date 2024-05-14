#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "polelist.h"


#include <QDebug>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void handleTreeViewClicked();

    void handleIRFrequencyInput();
    void handleTouchSensitivityInput();
    void handleIMUSensitivityInput();

    void handleGateNumberInput();

    void handleFindPolePartnerButtonClicked(bool checked) { if (_currentlySelectedPole != nullptr) _poleList->findPartnerToPole(_currentlySelectedPole); };

    // From Pole class 


    void updatePoleEventsVisualIndicators(events eventsdata);
    void setPowerStateIndicator(pps::PolePowerState powerState);

    void grayOutPoleControls(bool a);

private:
    Ui::MainWindow *_ui;
    PoleDataModel* _poleList;
    Pole* _currentlySelectedPole = nullptr;
};
#endif // MAINWINDOW_H
