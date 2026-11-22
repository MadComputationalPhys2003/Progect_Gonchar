#include "mainwindow.h"
#include "main_window_1.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    ui->FaustPushButtom->setEnabled(false);

    connect(ui->UrFaustPushButton, &QPushButton::clicked, this, [this] {
        if (!urFaustWindow) {
            urFaustWindow = new MainWindow_1(this);
        }

        urFaustWindow->showNormal();
        urFaustWindow->raise();
        urFaustWindow->activateWindow();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}