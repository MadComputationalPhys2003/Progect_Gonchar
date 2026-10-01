#include "main_window_1.h"
#include "ui_main_window_1.h"
#include <QDebug>
#include <QPushButton>
#include<cstdint>
MainWindow_1::MainWindow_1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow_1)
{
    ui->setupUi(this);
    connect(ui->calculateButton, &QPushButton::clicked,
            this, [this]()
            {
                const auto config = readConfig();

                qDebug() << "Dimension:"
                         << static_cast<int>(config.dimension) + 1;

                qDebug() << "Lx:" << config.lx
                         << "Ly:" << config.ly
                         << "Lz:" << config.lz;

                qDebug() << "Spin length:" << config.spinLength;
                qDebug() << "Exchange integral:" << config.exchangeIntegral;
                qDebug() << "Epsilon:" << config.epsilon;
                qDebug() << "Maximum iterations:" << config.maxSweeps;

                qDebug() << "Initialization:"
                         << (config.initialization ==
                                     UrFaust::Ions::SpinInit::parallel
                                 ? "Parallel"
                                 : "Random");
            });

}
UrFaustSim::SimulationConfig MainWindow_1::readConfig() const
{
    UrFaustSim::SimulationConfig config;

    const int dimensionIndex =
        ui->dimensionComboBox->currentIndex();

    config.dimension =
        static_cast<UrFaust::Dimension>(dimensionIndex);

    config.lx =
        static_cast<std::uint64_t>(ui->lxSpinBox->value());

    config.ly = dimensionIndex >= 1
                    ? static_cast<std::uint64_t>(ui->lySpinBox->value())
                    : 1;

    config.lz = dimensionIndex >= 2
                    ? static_cast<std::uint64_t>(ui->lzSpinBox->value())
                    : 1;

    config.spinLength = ui->spinLengthSpinBox->value();
    config.exchangeIntegral = ui->exchangeIntegralSpinBox->value();
    ui->exchangeIntegralSpinBox->setRange(-1000.0, 1000.0);
    ui->exchangeIntegralSpinBox->setValue(-1.0);

    qDebug() << "J range:"
             << ui->exchangeIntegralSpinBox->minimum()
             << ui->exchangeIntegralSpinBox->maximum();

    qDebug() << "J value:"
             << ui->exchangeIntegralSpinBox->value();
    config.epsilon = ui->epsilonSpinBox->value();

    config.maxSweeps =
        static_cast<std::size_t>(ui->maxSweepsSpinBox->value());

    config.initialization =
        static_cast<UrFaust::Ions::SpinInit>(
            ui->initializationComboBox->currentIndex()
            );

    return config;
}
MainWindow_1::~MainWindow_1()
{
    delete ui;
}
