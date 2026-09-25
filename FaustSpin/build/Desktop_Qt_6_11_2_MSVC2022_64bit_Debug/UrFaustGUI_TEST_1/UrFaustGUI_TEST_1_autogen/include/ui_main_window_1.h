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
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow_1
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
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
    } // retranslateUi

};

namespace Ui {
    class MainWindow_1: public Ui_MainWindow_1 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_WINDOW_1_H
