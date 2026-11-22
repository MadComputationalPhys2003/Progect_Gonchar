#include "mainwindow.h"
#include "main_window_1.h"
#include "ui_mainwindow.h"
#include "faust_spin_parameters_dialog.h"
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    ui->FaustPushButtom->setEnabled(true);

    connect(ui->FaustPushButtom, &QPushButton::clicked, this, [this] {
        FaustSpinParametersDialog dialog(faustSpinConfig, this);
        if (dialog.exec() == QDialog::Accepted) {
            dialog.applyTo(faustSpinConfig);
        }
    });

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