/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_2;
    QTreeView *treeView;
    QStackedWidget *PoleStatusViewSection;
    QWidget *nopoleselectedview;
    QLabel *label;
    QWidget *poleselectedview;
    QStackedWidget *PoleDiagramView;
    QWidget *masterpole;
    QLabel *label_2;
    QWidget *slavepole;
    QLabel *label_3;
    QLabel *label_4;
    QFrame *frame;
    QWidget *layoutWidget_9;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_14;
    QSpacerItem *horizontalSpacer_9;
    QLineEdit *GateNumberInputBox;
    QWidget *layoutWidget_10;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_15;
    QSpacerItem *horizontalSpacer_10;
    QComboBox *GatePassageDirectionSelector;
    QLabel *label_13;
    QWidget *layoutWidget_11;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_20;
    QLabel *PoleKnockedIndicator;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_18;
    QLabel *PassageDirectionIndicator;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_16;
    QLabel *PowerStateIndicator;
    QPushButton *FindGateButton;
    QPushButton *PoleStatusCheckButton;
    QPushButton *ReCalibratePoleButton;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_5;
    QSpacerItem *horizontalSpacer;
    QLineEdit *PoleTypeBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_6;
    QSpacerItem *horizontalSpacer_2;
    QLineEdit *PolePartnerBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_7;
    QSpacerItem *horizontalSpacer_3;
    QLineEdit *PoleGateNumberBox;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_8;
    QSpacerItem *horizontalSpacer_4;
    QLineEdit *TouchSensitivityInputBox;
    QWidget *layoutWidget2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_9;
    QLabel *label_12;
    QSpacerItem *horizontalSpacer_8;
    QLineEdit *PoleHWIDBox;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_11;
    QSpacerItem *horizontalSpacer_7;
    QLineEdit *PoleBatteryBox;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_10;
    QSpacerItem *horizontalSpacer_6;
    QLineEdit *PoleIRFrequencyInputBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_9;
    QSpacerItem *horizontalSpacer_5;
    QLineEdit *IMUSensitivityInputBox;
    QWidget *layoutWidget3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_24;
    QLabel *IRBeamTriggeredIndicator;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_22;
    QLabel *IRCameraTriggeredIndicator;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_26;
    QLabel *VelostatTriggeredIndicator;
    QStatusBar *statusbar;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QMenu *menuPreferences;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->setEnabled(true);
        MainWindow->resize(1200, 1000);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(1200, 1000));
        MainWindow->setMaximumSize(QSize(1200, 1000));
        QFont font;
        font.setPointSize(15);
        MainWindow->setFont(font);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(0, 0, 1201, 951));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        treeView = new QTreeView(layoutWidget);
        treeView->setObjectName("treeView");
        treeView->setMaximumSize(QSize(510, 16777215));
        QFont font1;
        font1.setPointSize(12);
        treeView->setFont(font1);
        treeView->setLayoutDirection(Qt::LeftToRight);
        treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeView->setProperty("showDropIndicator", QVariant(false));
        treeView->setAlternatingRowColors(false);
        treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
        treeView->setTextElideMode(Qt::ElideMiddle);
        treeView->setIndentation(0);
        treeView->setRootIsDecorated(false);
        treeView->setUniformRowHeights(true);
        treeView->setItemsExpandable(false);
        treeView->setHeaderHidden(false);
        treeView->header()->setStretchLastSection(false);

        horizontalLayout_2->addWidget(treeView);

        PoleStatusViewSection = new QStackedWidget(layoutWidget);
        PoleStatusViewSection->setObjectName("PoleStatusViewSection");
        PoleStatusViewSection->setMinimumSize(QSize(450, 0));
        nopoleselectedview = new QWidget();
        nopoleselectedview->setObjectName("nopoleselectedview");
        label = new QLabel(nopoleselectedview);
        label->setObjectName("label");
        label->setGeometry(QRect(0, 90, 591, 751));
        QFont font2;
        font2.setPointSize(20);
        label->setFont(font2);
        label->setTextFormat(Qt::AutoText);
        label->setWordWrap(true);
        PoleStatusViewSection->addWidget(nopoleselectedview);
        poleselectedview = new QWidget();
        poleselectedview->setObjectName("poleselectedview");
        PoleDiagramView = new QStackedWidget(poleselectedview);
        PoleDiagramView->setObjectName("PoleDiagramView");
        PoleDiagramView->setGeometry(QRect(-10, 0, 201, 949));
        masterpole = new QWidget();
        masterpole->setObjectName("masterpole");
        label_2 = new QLabel(masterpole);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(0, 0, 191, 951));
        label_2->setAlignment(Qt::AlignCenter);
        PoleDiagramView->addWidget(masterpole);
        slavepole = new QWidget();
        slavepole->setObjectName("slavepole");
        label_3 = new QLabel(slavepole);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(0, 0, 191, 951));
        label_3->setLayoutDirection(Qt::RightToLeft);
        label_3->setAlignment(Qt::AlignCenter);
        PoleDiagramView->addWidget(slavepole);
        label_4 = new QLabel(poleselectedview);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(-10, 0, 671, 71));
        label_4->setFont(font2);
        label_4->setAlignment(Qt::AlignCenter);
        frame = new QFrame(poleselectedview);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(210, 500, 421, 151));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        layoutWidget_9 = new QWidget(frame);
        layoutWidget_9->setObjectName("layoutWidget_9");
        layoutWidget_9->setGeometry(QRect(30, 20, 361, 51));
        horizontalLayout_10 = new QHBoxLayout(layoutWidget_9);
        horizontalLayout_10->setObjectName("horizontalLayout_10");
        horizontalLayout_10->setContentsMargins(0, 0, 0, 0);
        label_14 = new QLabel(layoutWidget_9);
        label_14->setObjectName("label_14");
        label_14->setFont(font1);

        horizontalLayout_10->addWidget(label_14);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_10->addItem(horizontalSpacer_9);

        GateNumberInputBox = new QLineEdit(layoutWidget_9);
        GateNumberInputBox->setObjectName("GateNumberInputBox");
        sizePolicy.setHeightForWidth(GateNumberInputBox->sizePolicy().hasHeightForWidth());
        GateNumberInputBox->setSizePolicy(sizePolicy);
        GateNumberInputBox->setMaximumSize(QSize(150, 16777215));
        GateNumberInputBox->setBaseSize(QSize(50, 0));

        horizontalLayout_10->addWidget(GateNumberInputBox, 0, Qt::AlignLeft);

        layoutWidget_10 = new QWidget(frame);
        layoutWidget_10->setObjectName("layoutWidget_10");
        layoutWidget_10->setGeometry(QRect(30, 80, 361, 51));
        horizontalLayout_11 = new QHBoxLayout(layoutWidget_10);
        horizontalLayout_11->setObjectName("horizontalLayout_11");
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        label_15 = new QLabel(layoutWidget_10);
        label_15->setObjectName("label_15");
        label_15->setFont(font1);

        horizontalLayout_11->addWidget(label_15);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_10);

        GatePassageDirectionSelector = new QComboBox(layoutWidget_10);
        GatePassageDirectionSelector->setObjectName("GatePassageDirectionSelector");
        GatePassageDirectionSelector->setMinimumSize(QSize(108, 0));

        horizontalLayout_11->addWidget(GatePassageDirectionSelector);

        label_13 = new QLabel(poleselectedview);
        label_13->setObjectName("label_13");
        label_13->setGeometry(QRect(210, 460, 421, 31));
        label_13->setFont(font);
        label_13->setLayoutDirection(Qt::LeftToRight);
        label_13->setAlignment(Qt::AlignCenter);
        layoutWidget_11 = new QWidget(poleselectedview);
        layoutWidget_11->setObjectName("layoutWidget_11");
        layoutWidget_11->setGeometry(QRect(460, 710, 210, 201));
        verticalLayout_4 = new QVBoxLayout(layoutWidget_11);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName("horizontalLayout_14");
        label_20 = new QLabel(layoutWidget_11);
        label_20->setObjectName("label_20");
        label_20->setFont(font1);

        horizontalLayout_14->addWidget(label_20);

        PoleKnockedIndicator = new QLabel(layoutWidget_11);
        PoleKnockedIndicator->setObjectName("PoleKnockedIndicator");
        PoleKnockedIndicator->setMaximumSize(QSize(50, 50));
        PoleKnockedIndicator->setFont(font1);
        PoleKnockedIndicator->setScaledContents(true);
        PoleKnockedIndicator->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        PoleKnockedIndicator->setMargin(10);

        horizontalLayout_14->addWidget(PoleKnockedIndicator);


        verticalLayout_4->addLayout(horizontalLayout_14);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName("horizontalLayout_13");
        label_18 = new QLabel(layoutWidget_11);
        label_18->setObjectName("label_18");
        label_18->setFont(font1);

        horizontalLayout_13->addWidget(label_18);

        PassageDirectionIndicator = new QLabel(layoutWidget_11);
        PassageDirectionIndicator->setObjectName("PassageDirectionIndicator");
        PassageDirectionIndicator->setMaximumSize(QSize(50, 50));
        PassageDirectionIndicator->setFont(font1);
        PassageDirectionIndicator->setScaledContents(true);
        PassageDirectionIndicator->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        PassageDirectionIndicator->setMargin(10);

        horizontalLayout_13->addWidget(PassageDirectionIndicator);


        verticalLayout_4->addLayout(horizontalLayout_13);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName("horizontalLayout_12");
        label_16 = new QLabel(layoutWidget_11);
        label_16->setObjectName("label_16");
        label_16->setFont(font1);

        horizontalLayout_12->addWidget(label_16);

        PowerStateIndicator = new QLabel(layoutWidget_11);
        PowerStateIndicator->setObjectName("PowerStateIndicator");
        PowerStateIndicator->setMaximumSize(QSize(50, 50));
        PowerStateIndicator->setFont(font1);
        PowerStateIndicator->setScaledContents(true);
        PowerStateIndicator->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        PowerStateIndicator->setMargin(10);

        horizontalLayout_12->addWidget(PowerStateIndicator);


        verticalLayout_4->addLayout(horizontalLayout_12);

        FindGateButton = new QPushButton(poleselectedview);
        FindGateButton->setObjectName("FindGateButton");
        FindGateButton->setGeometry(QRect(210, 390, 101, 31));
        FindGateButton->setFont(font);
        PoleStatusCheckButton = new QPushButton(poleselectedview);
        PoleStatusCheckButton->setObjectName("PoleStatusCheckButton");
        PoleStatusCheckButton->setGeometry(QRect(350, 390, 121, 31));
        PoleStatusCheckButton->setFont(font);
        ReCalibratePoleButton = new QPushButton(poleselectedview);
        ReCalibratePoleButton->setObjectName("ReCalibratePoleButton");
        ReCalibratePoleButton->setGeometry(QRect(510, 390, 121, 31));
        ReCalibratePoleButton->setFont(font);
        layoutWidget1 = new QWidget(poleselectedview);
        layoutWidget1->setObjectName("layoutWidget1");
        layoutWidget1->setGeometry(QRect(210, 100, 202, 241));
        verticalLayout = new QVBoxLayout(layoutWidget1);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label_5 = new QLabel(layoutWidget1);
        label_5->setObjectName("label_5");
        label_5->setFont(font1);

        horizontalLayout->addWidget(label_5);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        PoleTypeBox = new QLineEdit(layoutWidget1);
        PoleTypeBox->setObjectName("PoleTypeBox");
        sizePolicy.setHeightForWidth(PoleTypeBox->sizePolicy().hasHeightForWidth());
        PoleTypeBox->setSizePolicy(sizePolicy);
        PoleTypeBox->setMaximumSize(QSize(70, 16777215));
        PoleTypeBox->setBaseSize(QSize(50, 0));
        PoleTypeBox->setReadOnly(true);

        horizontalLayout->addWidget(PoleTypeBox, 0, Qt::AlignLeft);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_6 = new QLabel(layoutWidget1);
        label_6->setObjectName("label_6");
        label_6->setFont(font1);

        horizontalLayout_3->addWidget(label_6);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        PolePartnerBox = new QLineEdit(layoutWidget1);
        PolePartnerBox->setObjectName("PolePartnerBox");
        sizePolicy.setHeightForWidth(PolePartnerBox->sizePolicy().hasHeightForWidth());
        PolePartnerBox->setSizePolicy(sizePolicy);
        PolePartnerBox->setMaximumSize(QSize(70, 16777215));
        PolePartnerBox->setBaseSize(QSize(50, 0));
        PolePartnerBox->setReadOnly(true);

        horizontalLayout_3->addWidget(PolePartnerBox, 0, Qt::AlignLeft);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        label_7 = new QLabel(layoutWidget1);
        label_7->setObjectName("label_7");
        label_7->setFont(font1);

        horizontalLayout_4->addWidget(label_7);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);

        PoleGateNumberBox = new QLineEdit(layoutWidget1);
        PoleGateNumberBox->setObjectName("PoleGateNumberBox");
        sizePolicy.setHeightForWidth(PoleGateNumberBox->sizePolicy().hasHeightForWidth());
        PoleGateNumberBox->setSizePolicy(sizePolicy);
        PoleGateNumberBox->setMaximumSize(QSize(70, 16777215));
        PoleGateNumberBox->setBaseSize(QSize(50, 0));

        horizontalLayout_4->addWidget(PoleGateNumberBox, 0, Qt::AlignLeft);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        label_8 = new QLabel(layoutWidget1);
        label_8->setObjectName("label_8");
        label_8->setFont(font1);

        horizontalLayout_5->addWidget(label_8);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);

        TouchSensitivityInputBox = new QLineEdit(layoutWidget1);
        TouchSensitivityInputBox->setObjectName("TouchSensitivityInputBox");
        sizePolicy.setHeightForWidth(TouchSensitivityInputBox->sizePolicy().hasHeightForWidth());
        TouchSensitivityInputBox->setSizePolicy(sizePolicy);
        TouchSensitivityInputBox->setMaximumSize(QSize(70, 16777215));
        TouchSensitivityInputBox->setBaseSize(QSize(50, 0));

        horizontalLayout_5->addWidget(TouchSensitivityInputBox, 0, Qt::AlignLeft);


        verticalLayout->addLayout(horizontalLayout_5);

        layoutWidget2 = new QWidget(poleselectedview);
        layoutWidget2->setObjectName("layoutWidget2");
        layoutWidget2->setGeometry(QRect(440, 100, 194, 241));
        verticalLayout_2 = new QVBoxLayout(layoutWidget2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName("horizontalLayout_9");
        label_12 = new QLabel(layoutWidget2);
        label_12->setObjectName("label_12");
        label_12->setFont(font1);

        horizontalLayout_9->addWidget(label_12);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_8);

        PoleHWIDBox = new QLineEdit(layoutWidget2);
        PoleHWIDBox->setObjectName("PoleHWIDBox");
        sizePolicy.setHeightForWidth(PoleHWIDBox->sizePolicy().hasHeightForWidth());
        PoleHWIDBox->setSizePolicy(sizePolicy);
        PoleHWIDBox->setMaximumSize(QSize(70, 16777215));
        PoleHWIDBox->setBaseSize(QSize(50, 0));
        PoleHWIDBox->setReadOnly(true);

        horizontalLayout_9->addWidget(PoleHWIDBox, 0, Qt::AlignLeft);


        verticalLayout_2->addLayout(horizontalLayout_9);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName("horizontalLayout_8");
        label_11 = new QLabel(layoutWidget2);
        label_11->setObjectName("label_11");
        label_11->setFont(font1);

        horizontalLayout_8->addWidget(label_11);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_7);

        PoleBatteryBox = new QLineEdit(layoutWidget2);
        PoleBatteryBox->setObjectName("PoleBatteryBox");
        sizePolicy.setHeightForWidth(PoleBatteryBox->sizePolicy().hasHeightForWidth());
        PoleBatteryBox->setSizePolicy(sizePolicy);
        PoleBatteryBox->setMaximumSize(QSize(70, 16777215));
        PoleBatteryBox->setBaseSize(QSize(50, 0));
        PoleBatteryBox->setReadOnly(true);

        horizontalLayout_8->addWidget(PoleBatteryBox, 0, Qt::AlignLeft);


        verticalLayout_2->addLayout(horizontalLayout_8);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        label_10 = new QLabel(layoutWidget2);
        label_10->setObjectName("label_10");
        label_10->setFont(font1);

        horizontalLayout_7->addWidget(label_10);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);

        PoleIRFrequencyInputBox = new QLineEdit(layoutWidget2);
        PoleIRFrequencyInputBox->setObjectName("PoleIRFrequencyInputBox");
        sizePolicy.setHeightForWidth(PoleIRFrequencyInputBox->sizePolicy().hasHeightForWidth());
        PoleIRFrequencyInputBox->setSizePolicy(sizePolicy);
        PoleIRFrequencyInputBox->setMaximumSize(QSize(70, 16777215));
        PoleIRFrequencyInputBox->setBaseSize(QSize(50, 0));

        horizontalLayout_7->addWidget(PoleIRFrequencyInputBox, 0, Qt::AlignLeft);


        verticalLayout_2->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        label_9 = new QLabel(layoutWidget2);
        label_9->setObjectName("label_9");
        label_9->setFont(font1);

        horizontalLayout_6->addWidget(label_9);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);

        IMUSensitivityInputBox = new QLineEdit(layoutWidget2);
        IMUSensitivityInputBox->setObjectName("IMUSensitivityInputBox");
        sizePolicy.setHeightForWidth(IMUSensitivityInputBox->sizePolicy().hasHeightForWidth());
        IMUSensitivityInputBox->setSizePolicy(sizePolicy);
        IMUSensitivityInputBox->setMaximumSize(QSize(70, 16777215));
        IMUSensitivityInputBox->setBaseSize(QSize(50, 0));

        horizontalLayout_6->addWidget(IMUSensitivityInputBox, 0, Qt::AlignLeft);


        verticalLayout_2->addLayout(horizontalLayout_6);

        layoutWidget3 = new QWidget(poleselectedview);
        layoutWidget3->setObjectName("layoutWidget3");
        layoutWidget3->setGeometry(QRect(210, 710, 225, 201));
        verticalLayout_3 = new QVBoxLayout(layoutWidget3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName("horizontalLayout_16");
        label_24 = new QLabel(layoutWidget3);
        label_24->setObjectName("label_24");
        label_24->setFont(font1);

        horizontalLayout_16->addWidget(label_24);

        IRBeamTriggeredIndicator = new QLabel(layoutWidget3);
        IRBeamTriggeredIndicator->setObjectName("IRBeamTriggeredIndicator");
        IRBeamTriggeredIndicator->setMaximumSize(QSize(50, 50));
        IRBeamTriggeredIndicator->setFont(font1);
        IRBeamTriggeredIndicator->setScaledContents(true);
        IRBeamTriggeredIndicator->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        IRBeamTriggeredIndicator->setMargin(10);

        horizontalLayout_16->addWidget(IRBeamTriggeredIndicator);


        verticalLayout_3->addLayout(horizontalLayout_16);

        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName("horizontalLayout_15");
        label_22 = new QLabel(layoutWidget3);
        label_22->setObjectName("label_22");
        label_22->setFont(font1);

        horizontalLayout_15->addWidget(label_22);

        IRCameraTriggeredIndicator = new QLabel(layoutWidget3);
        IRCameraTriggeredIndicator->setObjectName("IRCameraTriggeredIndicator");
        IRCameraTriggeredIndicator->setMaximumSize(QSize(50, 50));
        IRCameraTriggeredIndicator->setFont(font1);
        IRCameraTriggeredIndicator->setScaledContents(true);
        IRCameraTriggeredIndicator->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        IRCameraTriggeredIndicator->setMargin(10);

        horizontalLayout_15->addWidget(IRCameraTriggeredIndicator);


        verticalLayout_3->addLayout(horizontalLayout_15);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName("horizontalLayout_17");
        label_26 = new QLabel(layoutWidget3);
        label_26->setObjectName("label_26");
        label_26->setFont(font1);

        horizontalLayout_17->addWidget(label_26);

        VelostatTriggeredIndicator = new QLabel(layoutWidget3);
        VelostatTriggeredIndicator->setObjectName("VelostatTriggeredIndicator");
        VelostatTriggeredIndicator->setMaximumSize(QSize(50, 50));
        VelostatTriggeredIndicator->setFont(font1);
        VelostatTriggeredIndicator->setScaledContents(true);
        VelostatTriggeredIndicator->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        VelostatTriggeredIndicator->setMargin(10);

        horizontalLayout_17->addWidget(VelostatTriggeredIndicator);


        verticalLayout_3->addLayout(horizontalLayout_17);

        PoleStatusViewSection->addWidget(poleselectedview);

        horizontalLayout_2->addWidget(PoleStatusViewSection);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1200, 26));
        menubar->setFont(font1);
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName("menuHelp");
        menuPreferences = new QMenu(menubar);
        menuPreferences->setObjectName("menuPreferences");
        MainWindow->setMenuBar(menubar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menubar->addAction(menuPreferences->menuAction());

        retranslateUi(MainWindow);

        PoleStatusViewSection->setCurrentIndex(1);
        PoleDiagramView->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "This is the default text for when nothing is selected and program has only just been opened.", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p><img src=\":/PoleDiagram/EEEE4109_Master_Pole_UI_Drawing.png\"/></p></body></html>", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p><img src=\":/PoleDiagram/EEEE4109_Slave_Pole_UI_Drawing.png\"/></p></body></html>", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Pole <PoleID>", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow", "Gate Number", nullptr));
        label_15->setText(QCoreApplication::translate("MainWindow", "Passage Direction", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "Gate Configuration", nullptr));
        label_20->setText(QCoreApplication::translate("MainWindow", "Pole Knocked:", nullptr));
        PoleKnockedIndicator->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        label_18->setText(QCoreApplication::translate("MainWindow", "PassageDirection:", nullptr));
        PassageDirectionIndicator->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        label_16->setText(QCoreApplication::translate("MainWindow", "Power State:", nullptr));
        PowerStateIndicator->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        FindGateButton->setText(QCoreApplication::translate("MainWindow", "Find Gate", nullptr));
        PoleStatusCheckButton->setText(QCoreApplication::translate("MainWindow", "Status check", nullptr));
        ReCalibratePoleButton->setText(QCoreApplication::translate("MainWindow", "Re-Calibrate", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Pole Type:", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Pole Partner:", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Gate Number:", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Touch sensitivity:", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Pole HWID:", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Pole Battery:", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "IR Frequency::", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "IMU Sensitivity:", nullptr));
        label_24->setText(QCoreApplication::translate("MainWindow", "IR Beam Triggered:", nullptr));
        IRBeamTriggeredIndicator->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        label_22->setText(QCoreApplication::translate("MainWindow", "IR Camera Triggered:", nullptr));
        IRCameraTriggeredIndicator->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        label_26->setText(QCoreApplication::translate("MainWindow", "Velostat Triggered:", nullptr));
        VelostatTriggeredIndicator->setText(QCoreApplication::translate("MainWindow", "None", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        menuPreferences->setTitle(QCoreApplication::translate("MainWindow", "Preferences", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
