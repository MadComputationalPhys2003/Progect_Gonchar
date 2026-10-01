/********************************************************************************
** Form generated from reading UI file 'main_window_1.ui'
**
** Created by: Qt User Interface Compiler version 6.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_WINDOW_1_H
#define UI_MAIN_WINDOW_1_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow_1
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *SystemResults;
    QWidget *tab;
    QWidget *tab_2;
    QGridLayout *gridLayout;
    QGroupBox *parametersGroupBox;
    QPushButton *calculateButton;
    QProgressBar *simulationProgressBar;
    QLabel *calculationStatusLabel;
    QWidget *widget;
    QGridLayout *gridLayout_3;
    QLabel *label_9;
    QDoubleSpinBox *exchangeIntegralSpinBox;
    QWidget *widget1;
    QGridLayout *gridLayout_6;
    QDoubleSpinBox *spinLengthSpinBox;
    QLabel *label_8;
    QWidget *widget2;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QComboBox *dimensionComboBox;
    QSpinBox *lxSpinBox;
    QSpinBox *lySpinBox;
    QSpinBox *lzSpinBox;
    QWidget *widget3;
    QGridLayout *gridLayout_7;
    QComboBox *initializationComboBox;
    QLabel *label_12;
    QSplitter *splitter;
    QLabel *label_11;
    QSpinBox *maxSweepsSpinBox;
    QLabel *label_10;
    QDoubleSpinBox *epsilonSpinBox;
    QWidget *tab_4;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QLabel *initialEnergyLabel;
    QLineEdit *resultsLineEdit;
    QLabel *finalEnergyLabel;
    QLineEdit *finalEnergyLabel1;
    QLabel *analyticalEnergyLabel;
    QLineEdit *analyticalEnergyLineEdit;
    QLabel *energyDifferenceLabel;
    QLineEdit *energyDifferenceLineEdit;
    QLabel *completedIterationsLabel;
    QLineEdit *completedIterationsLineEdit;
    QLabel *stoppingReasonLabel;
    QLineEdit *stoppingReasonLineEdit;
    QWidget *tab_3;
    QVBoxLayout *verticalLayout_2;
    QGraphicsView *graphicsView;
    QWidget *tab_5;
    QOpenGLWidget *spinView;
    QWidget *layoutWidget;
    QGridLayout *gridLayout_5;
    QLabel *label_5;
    QSpinBox *iterationSpinBox;
    QPushButton *finalFrameButton;
    QLabel *label_6;
    QSpinBox *nodeIndexSpinBox;
    QLabel *spinDirectionLabel;
    QLabel *finalSpinDirectionLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow_1)
    {
        if (MainWindow_1->objectName().isEmpty())
            MainWindow_1->setObjectName("MainWindow_1");
        MainWindow_1->resize(815, 660);
        centralwidget = new QWidget(MainWindow_1);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        SystemResults = new QTabWidget(centralwidget);
        SystemResults->setObjectName("SystemResults");
        SystemResults->setMouseTracking(true);
        SystemResults->setTabletTracking(true);
        SystemResults->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        tab = new QWidget();
        tab->setObjectName("tab");
        SystemResults->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        gridLayout = new QGridLayout(tab_2);
        gridLayout->setObjectName("gridLayout");
        parametersGroupBox = new QGroupBox(tab_2);
        parametersGroupBox->setObjectName("parametersGroupBox");
        calculateButton = new QPushButton(parametersGroupBox);
        calculateButton->setObjectName("calculateButton");
        calculateButton->setGeometry(QRect(282, 480, 111, 29));
        simulationProgressBar = new QProgressBar(parametersGroupBox);
        simulationProgressBar->setObjectName("simulationProgressBar");
        simulationProgressBar->setGeometry(QRect(410, 480, 118, 23));
        simulationProgressBar->setValue(1);
        calculationStatusLabel = new QLabel(parametersGroupBox);
        calculationStatusLabel->setObjectName("calculationStatusLabel");
        calculationStatusLabel->setGeometry(QRect(540, 480, 63, 20));
        widget = new QWidget(parametersGroupBox);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(370, 20, 133, 55));
        gridLayout_3 = new QGridLayout(widget);
        gridLayout_3->setObjectName("gridLayout_3");
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        label_9 = new QLabel(widget);
        label_9->setObjectName("label_9");

        gridLayout_3->addWidget(label_9, 0, 0, 1, 1);

        exchangeIntegralSpinBox = new QDoubleSpinBox(widget);
        exchangeIntegralSpinBox->setObjectName("exchangeIntegralSpinBox");
        exchangeIntegralSpinBox->setEnabled(true);
        exchangeIntegralSpinBox->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        exchangeIntegralSpinBox->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        exchangeIntegralSpinBox->setAcceptDrops(false);
        exchangeIntegralSpinBox->setFrame(false);
        exchangeIntegralSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        exchangeIntegralSpinBox->setAccelerated(true);
        exchangeIntegralSpinBox->setDecimals(6);
        exchangeIntegralSpinBox->setMinimum(-4.000000000000000);

        gridLayout_3->addWidget(exchangeIntegralSpinBox, 1, 0, 1, 1);

        widget1 = new QWidget(parametersGroupBox);
        widget1->setObjectName("widget1");
        widget1->setGeometry(QRect(510, 20, 89, 55));
        gridLayout_6 = new QGridLayout(widget1);
        gridLayout_6->setObjectName("gridLayout_6");
        gridLayout_6->setContentsMargins(0, 0, 0, 0);
        spinLengthSpinBox = new QDoubleSpinBox(widget1);
        spinLengthSpinBox->setObjectName("spinLengthSpinBox");
        spinLengthSpinBox->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        spinLengthSpinBox->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        spinLengthSpinBox->setAcceptDrops(true);
        spinLengthSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        spinLengthSpinBox->setAccelerated(true);
        spinLengthSpinBox->setCorrectionMode(QAbstractSpinBox::CorrectionMode::CorrectToPreviousValue);
        spinLengthSpinBox->setProperty("showGroupSeparator", QVariant(true));
        spinLengthSpinBox->setDecimals(6);
        spinLengthSpinBox->setMinimum(-10.000000000000000);
        spinLengthSpinBox->setMaximum(10.000000000000000);
        spinLengthSpinBox->setSingleStep(0.000000000000000);
        spinLengthSpinBox->setStepType(QAbstractSpinBox::StepType::AdaptiveDecimalStepType);

        gridLayout_6->addWidget(spinLengthSpinBox, 1, 0, 1, 1);

        label_8 = new QLabel(widget1);
        label_8->setObjectName("label_8");

        gridLayout_6->addWidget(label_8, 0, 0, 1, 1);

        widget2 = new QWidget(parametersGroupBox);
        widget2->setObjectName("widget2");
        widget2->setGeometry(QRect(0, 20, 169, 88));
        gridLayout_2 = new QGridLayout(widget2);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget2);
        label->setObjectName("label");

        gridLayout_2->addWidget(label, 0, 0, 1, 3);

        label_2 = new QLabel(widget2);
        label_2->setObjectName("label_2");

        gridLayout_2->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(widget2);
        label_3->setObjectName("label_3");

        gridLayout_2->addWidget(label_3, 1, 1, 1, 1);

        label_4 = new QLabel(widget2);
        label_4->setObjectName("label_4");

        gridLayout_2->addWidget(label_4, 1, 2, 1, 1);

        dimensionComboBox = new QComboBox(widget2);
        dimensionComboBox->addItem(QString());
        dimensionComboBox->addItem(QString());
        dimensionComboBox->addItem(QString());
        dimensionComboBox->setObjectName("dimensionComboBox");
        dimensionComboBox->setEditable(false);

        gridLayout_2->addWidget(dimensionComboBox, 1, 3, 1, 1);

        lxSpinBox = new QSpinBox(widget2);
        lxSpinBox->setObjectName("lxSpinBox");
        lxSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        gridLayout_2->addWidget(lxSpinBox, 2, 0, 1, 1);

        lySpinBox = new QSpinBox(widget2);
        lySpinBox->setObjectName("lySpinBox");
        lySpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        gridLayout_2->addWidget(lySpinBox, 2, 1, 1, 1);

        lzSpinBox = new QSpinBox(widget2);
        lzSpinBox->setObjectName("lzSpinBox");
        lzSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

        gridLayout_2->addWidget(lzSpinBox, 2, 2, 1, 1);

        widget3 = new QWidget(parametersGroupBox);
        widget3->setObjectName("widget3");
        widget3->setGeometry(QRect(170, 20, 196, 55));
        gridLayout_7 = new QGridLayout(widget3);
        gridLayout_7->setObjectName("gridLayout_7");
        gridLayout_7->setContentsMargins(0, 0, 0, 0);
        initializationComboBox = new QComboBox(widget3);
        initializationComboBox->addItem(QString());
        initializationComboBox->addItem(QString());
        initializationComboBox->setObjectName("initializationComboBox");

        gridLayout_7->addWidget(initializationComboBox, 2, 1, 1, 1);

        label_12 = new QLabel(widget3);
        label_12->setObjectName("label_12");

        gridLayout_7->addWidget(label_12, 0, 1, 1, 1);

        splitter = new QSplitter(parametersGroupBox);
        splitter->setObjectName("splitter");
        splitter->setGeometry(QRect(170, 80, 276, 26));
        splitter->setOrientation(Qt::Orientation::Horizontal);
        label_11 = new QLabel(splitter);
        label_11->setObjectName("label_11");
        splitter->addWidget(label_11);
        maxSweepsSpinBox = new QSpinBox(splitter);
        maxSweepsSpinBox->setObjectName("maxSweepsSpinBox");
        maxSweepsSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        splitter->addWidget(maxSweepsSpinBox);
        label_10 = new QLabel(splitter);
        label_10->setObjectName("label_10");
        splitter->addWidget(label_10);
        epsilonSpinBox = new QDoubleSpinBox(splitter);
        epsilonSpinBox->setObjectName("epsilonSpinBox");
        epsilonSpinBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
        splitter->addWidget(epsilonSpinBox);

        gridLayout->addWidget(parametersGroupBox, 0, 0, 1, 1);

        SystemResults->addTab(tab_2, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName("tab_4");
        formLayoutWidget = new QWidget(tab_4);
        formLayoutWidget->setObjectName("formLayoutWidget");
        formLayoutWidget->setGeometry(QRect(10, 10, 210, 205));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        initialEnergyLabel = new QLabel(formLayoutWidget);
        initialEnergyLabel->setObjectName("initialEnergyLabel");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, initialEnergyLabel);

        resultsLineEdit = new QLineEdit(formLayoutWidget);
        resultsLineEdit->setObjectName("resultsLineEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, resultsLineEdit);

        finalEnergyLabel = new QLabel(formLayoutWidget);
        finalEnergyLabel->setObjectName("finalEnergyLabel");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, finalEnergyLabel);

        finalEnergyLabel1 = new QLineEdit(formLayoutWidget);
        finalEnergyLabel1->setObjectName("finalEnergyLabel1");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, finalEnergyLabel1);

        analyticalEnergyLabel = new QLabel(formLayoutWidget);
        analyticalEnergyLabel->setObjectName("analyticalEnergyLabel");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, analyticalEnergyLabel);

        analyticalEnergyLineEdit = new QLineEdit(formLayoutWidget);
        analyticalEnergyLineEdit->setObjectName("analyticalEnergyLineEdit");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, analyticalEnergyLineEdit);

        energyDifferenceLabel = new QLabel(formLayoutWidget);
        energyDifferenceLabel->setObjectName("energyDifferenceLabel");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, energyDifferenceLabel);

        energyDifferenceLineEdit = new QLineEdit(formLayoutWidget);
        energyDifferenceLineEdit->setObjectName("energyDifferenceLineEdit");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, energyDifferenceLineEdit);

        completedIterationsLabel = new QLabel(formLayoutWidget);
        completedIterationsLabel->setObjectName("completedIterationsLabel");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, completedIterationsLabel);

        completedIterationsLineEdit = new QLineEdit(formLayoutWidget);
        completedIterationsLineEdit->setObjectName("completedIterationsLineEdit");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, completedIterationsLineEdit);

        stoppingReasonLabel = new QLabel(formLayoutWidget);
        stoppingReasonLabel->setObjectName("stoppingReasonLabel");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, stoppingReasonLabel);

        stoppingReasonLineEdit = new QLineEdit(formLayoutWidget);
        stoppingReasonLineEdit->setObjectName("stoppingReasonLineEdit");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, stoppingReasonLineEdit);

        SystemResults->addTab(tab_4, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        verticalLayout_2 = new QVBoxLayout(tab_3);
        verticalLayout_2->setObjectName("verticalLayout_2");
        graphicsView = new QGraphicsView(tab_3);
        graphicsView->setObjectName("graphicsView");

        verticalLayout_2->addWidget(graphicsView);

        SystemResults->addTab(tab_3, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName("tab_5");
        spinView = new QOpenGLWidget(tab_5);
        spinView->setObjectName("spinView");
        spinView->setGeometry(QRect(139, -1, 641, 551));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(spinView->sizePolicy().hasHeightForWidth());
        spinView->setSizePolicy(sizePolicy);
        layoutWidget = new QWidget(tab_5);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(0, 0, 142, 211));
        gridLayout_5 = new QGridLayout(layoutWidget);
        gridLayout_5->setObjectName("gridLayout_5");
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(layoutWidget);
        label_5->setObjectName("label_5");

        gridLayout_5->addWidget(label_5, 0, 0, 1, 1);

        iterationSpinBox = new QSpinBox(layoutWidget);
        iterationSpinBox->setObjectName("iterationSpinBox");

        gridLayout_5->addWidget(iterationSpinBox, 1, 0, 1, 1);

        finalFrameButton = new QPushButton(layoutWidget);
        finalFrameButton->setObjectName("finalFrameButton");

        gridLayout_5->addWidget(finalFrameButton, 2, 0, 1, 1);

        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName("label_6");

        gridLayout_5->addWidget(label_6, 3, 0, 1, 1);

        nodeIndexSpinBox = new QSpinBox(layoutWidget);
        nodeIndexSpinBox->setObjectName("nodeIndexSpinBox");

        gridLayout_5->addWidget(nodeIndexSpinBox, 4, 0, 1, 1);

        spinDirectionLabel = new QLabel(layoutWidget);
        spinDirectionLabel->setObjectName("spinDirectionLabel");

        gridLayout_5->addWidget(spinDirectionLabel, 5, 0, 1, 1);

        finalSpinDirectionLabel = new QLabel(layoutWidget);
        finalSpinDirectionLabel->setObjectName("finalSpinDirectionLabel");

        gridLayout_5->addWidget(finalSpinDirectionLabel, 6, 0, 1, 1);

        SystemResults->addTab(tab_5, QString());

        verticalLayout->addWidget(SystemResults);

        MainWindow_1->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow_1);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 815, 26));
        MainWindow_1->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow_1);
        statusbar->setObjectName("statusbar");
        MainWindow_1->setStatusBar(statusbar);

        retranslateUi(MainWindow_1);

        SystemResults->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow_1);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow_1)
    {
        MainWindow_1->setWindowTitle(QCoreApplication::translate("MainWindow_1", "MainWindow_1", nullptr));
        SystemResults->setTabText(SystemResults->indexOf(tab), QCoreApplication::translate("MainWindow_1", "Main Window", nullptr));
        parametersGroupBox->setTitle(QCoreApplication::translate("MainWindow_1", "                          \320\237\320\260\321\200\320\260\320\274\320\265\321\202\321\200\321\213 \321\201\320\270\321\201\321\202\320\265\320\274\321\213", nullptr));
        calculateButton->setText(QCoreApplication::translate("MainWindow_1", "Calculate", nullptr));
        calculationStatusLabel->setText(QCoreApplication::translate("MainWindow_1", "Ready", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow_1", " Exchange integral J", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow_1", "Spin length S", nullptr));
        label->setText(QCoreApplication::translate("MainWindow_1", "Systen dimension", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow_1", "Lx", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow_1", "Ly", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow_1", "Lz", nullptr));
        dimensionComboBox->setItemText(0, QCoreApplication::translate("MainWindow_1", "1D", nullptr));
        dimensionComboBox->setItemText(1, QCoreApplication::translate("MainWindow_1", "2D", nullptr));
        dimensionComboBox->setItemText(2, QCoreApplication::translate("MainWindow_1", "3D", nullptr));

        initializationComboBox->setItemText(0, QCoreApplication::translate("MainWindow_1", "Parallel", nullptr));
        initializationComboBox->setItemText(1, QCoreApplication::translate("MainWindow_1", "Random", nullptr));

        label_12->setText(QCoreApplication::translate("MainWindow_1", "Initial directions", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow_1", "Maximum iterations", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow_1", "Energy error", nullptr));
        SystemResults->setTabText(SystemResults->indexOf(tab_2), QCoreApplication::translate("MainWindow_1", "Set Parametrs", nullptr));
        initialEnergyLabel->setText(QCoreApplication::translate("MainWindow_1", "Initial energy", nullptr));
        finalEnergyLabel->setText(QCoreApplication::translate("MainWindow_1", "Final energy", nullptr));
        analyticalEnergyLabel->setText(QCoreApplication::translate("MainWindow_1", "Analytical energy", nullptr));
        energyDifferenceLabel->setText(QCoreApplication::translate("MainWindow_1", "Energy difference", nullptr));
        completedIterationsLabel->setText(QCoreApplication::translate("MainWindow_1", "Completed iterations", nullptr));
        stoppingReasonLabel->setText(QCoreApplication::translate("MainWindow_1", "Stopping reason", nullptr));
        SystemResults->setTabText(SystemResults->indexOf(tab_4), QCoreApplication::translate("MainWindow_1", "Results", nullptr));
        SystemResults->setTabText(SystemResults->indexOf(tab_3), QCoreApplication::translate("MainWindow_1", "Energy plot", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow_1", "choice of iteration", nullptr));
        finalFrameButton->setText(QCoreApplication::translate("MainWindow_1", "Final configuration", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow_1", "node selection", nullptr));
        spinDirectionLabel->setText(QCoreApplication::translate("MainWindow_1", "Direction:", nullptr));
        finalSpinDirectionLabel->setText(QCoreApplication::translate("MainWindow_1", "Final direction:", nullptr));
        SystemResults->setTabText(SystemResults->indexOf(tab_5), QCoreApplication::translate("MainWindow_1", "Spin Visualization", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow_1: public Ui_MainWindow_1 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_WINDOW_1_H
