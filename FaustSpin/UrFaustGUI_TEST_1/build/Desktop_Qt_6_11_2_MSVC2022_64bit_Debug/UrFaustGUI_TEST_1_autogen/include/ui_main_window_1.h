/********************************************************************************
** Form generated from reading UI file 'main_window_1.ui'
**
** Created by: Qt User Interface Compiler version 6.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_WINDOW_1_H
#define UI_MAIN_WINDOW_1_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow_1
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QSpinBox *lxSpinBox;
    QLabel *label_2;
    QLabel *CellCountLabel;
    QLabel *label;
    QSpinBox *lzSpinBox;
    QPushButton *calculateNButton;
    QSpinBox *lySpinBox;
    QLabel *label_3;
    QLabel *nResultLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow_1)
    {
        if (MainWindow_1->objectName().isEmpty())
            MainWindow_1->setObjectName("MainWindow_1");
        MainWindow_1->resize(800, 600);
        centralwidget = new QWidget(MainWindow_1);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        lxSpinBox = new QSpinBox(centralwidget);
        lxSpinBox->setObjectName("lxSpinBox");
        lxSpinBox->setMaximum(1000);
        lxSpinBox->setValue(10);

        gridLayout->addWidget(lxSpinBox, 0, 1, 1, 1);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        CellCountLabel = new QLabel(centralwidget);
        CellCountLabel->setObjectName("CellCountLabel");

        gridLayout->addWidget(CellCountLabel, 5, 0, 1, 2);

        label = new QLabel(centralwidget);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lzSpinBox = new QSpinBox(centralwidget);
        lzSpinBox->setObjectName("lzSpinBox");
        lzSpinBox->setMaximum(1000);
        lzSpinBox->setValue(10);

        gridLayout->addWidget(lzSpinBox, 2, 1, 1, 1);

        calculateNButton = new QPushButton(centralwidget);
        calculateNButton->setObjectName("calculateNButton");

        gridLayout->addWidget(calculateNButton, 3, 0, 1, 2);

        lySpinBox = new QSpinBox(centralwidget);
        lySpinBox->setObjectName("lySpinBox");
        lySpinBox->setMaximum(1000);
        lySpinBox->setValue(10);

        gridLayout->addWidget(lySpinBox, 1, 1, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        nResultLabel = new QLabel(centralwidget);
        nResultLabel->setObjectName("nResultLabel");

        gridLayout->addWidget(nResultLabel, 4, 0, 1, 1);


        verticalLayout->addLayout(gridLayout);

        MainWindow_1->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow_1);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 26));
        MainWindow_1->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow_1);
        statusbar->setObjectName("statusbar");
        MainWindow_1->setStatusBar(statusbar);

        retranslateUi(MainWindow_1);

        QMetaObject::connectSlotsByName(MainWindow_1);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow_1)
    {
        MainWindow_1->setWindowTitle(QCoreApplication::translate("MainWindow_1", "MainWindow_1", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow_1", "Ly", nullptr));
        CellCountLabel->setText(QCoreApplication::translate("MainWindow_1", "N=", nullptr));
        label->setText(QCoreApplication::translate("MainWindow_1", "Lx", nullptr));
        calculateNButton->setText(QCoreApplication::translate("MainWindow_1", "N==", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow_1", "Lz", nullptr));
        nResultLabel->setText(QCoreApplication::translate("MainWindow_1", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow_1: public Ui_MainWindow_1 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_WINDOW_1_H
