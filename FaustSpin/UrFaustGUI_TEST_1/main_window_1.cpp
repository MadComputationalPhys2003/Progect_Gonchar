#include "main_window_1.h"
#include "ui_main_window_1.h"
#include<cstdint>
MainWindow_1::MainWindow_1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow_1)
{
    ui->setupUi(this);
    connect(ui->calculateNButton, &QPushButton::clicked,
            this, [this]()
            {
                const auto lx =
                    static_cast<std::uint64_t>(ui->lxSpinBox->value());

                const auto ly =
                    static_cast<std::uint64_t>(ui->lySpinBox->value());

                const auto lz =
                    static_cast<std::uint64_t>(ui->lzSpinBox->value());

                const std::uint64_t n = lx * ly * lz;

                ui->nResultLabel->setText(
                    QStringLiteral("N = %1").arg(static_cast<qulonglong>(n))
                    );
    });
}

MainWindow_1::~MainWindow_1()
{
    delete ui;
}
