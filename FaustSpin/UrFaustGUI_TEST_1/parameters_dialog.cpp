#include "parameters_dialog.h"
#include "ui_parameters_dialog.h"
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <QString>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <utility>

ParametersDialog::ParametersDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ParametersDialog)
{
    ui->setupUi(this);
    connect(ui->calculateButton, &QPushButton::clicked,
            this, &ParametersDialog::startSimulation);
}
UrFaustSim::SimulationConfig ParametersDialog::readConfig() const
{
    UrFaustSim::SimulationConfig config;

    const int dimensionIndex =
        ui->SystemDimentionComboBox->currentIndex();

    config.dimension =
        static_cast<UrFaust::Dimension>(dimensionIndex);

    config.lx =
        static_cast<std::uint64_t>(ui->LxSpinBox->value());

    config.ly = dimensionIndex >= 1
                    ? static_cast<std::uint64_t>(ui->LySpinBox->value())
                    : 1;

    config.lz = dimensionIndex >= 2
                    ? static_cast<std::uint64_t>(ui->LzSpinBox->value())
                    : 1;

    config.spinLength =
        ui->SpinLengthDoubleSpinBox->value();

    config.exchangeIntegral =
        ui->ExchangeIntegralDoubleSpinBox->value();

    config.epsilon =
        ui->EnergyErrorDoubleSpinBox->value();

    config.maxSweeps =
        static_cast<std::size_t>(ui->spinBox->value());

    config.initialization =
        static_cast<UrFaust::Ions::SpinInit>(
            ui->InitialDirectionComboBox->currentIndex());

    return config;
}
void ParametersDialog::startSimulation()
{
    const auto config = readConfig();

    if (config.maxSweeps == 0) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid parameters"),
            QStringLiteral(
                "Maximum iterations must be greater than zero.")
            );
        return;
    }

    struct SimulationOutcome {
        UrFaustSim::SimulationResult result;
        QString error;
    };

    const auto outcome =
        std::make_shared<SimulationOutcome>();

    auto* worker = QThread::create([config, outcome]()
                                   {
                                       try {
                                           UrFaustSim::Simulation simulation(config);
                                           outcome->result = simulation.run();
                                       }
                                       catch (const std::exception& error) {
                                           outcome->error = QString::fromUtf8(error.what());
                                       }
                                       catch (...) {
                                           outcome->error =
                                               QStringLiteral("Unknown simulation error.");
                                       }
                                   });

    worker->setParent(this);

    connect(worker, &QThread::finished,
            worker, &QObject::deleteLater);

    connect(worker, &QThread::finished,
            this, [this, outcome]()
            {
                ui->calculateButton->setEnabled(true);

                if (!outcome->error.isEmpty()) {
                    ui->calculationStatusLabel->setText(
                        QStringLiteral("Failed"));

                    ui->simulationProgressBar->setRange(0, 100);
                    ui->simulationProgressBar->setValue(0);

                    QMessageBox::critical(
                        this,
                        QStringLiteral("Simulation error"),
                        outcome->error
                        );
                    return;
                }

                simulationResult = std::move(outcome->result);

                ui->simulationProgressBar->setRange(0, 100);
                ui->simulationProgressBar->setValue(100);

                ui->calculationStatusLabel->setText(
                    QStringLiteral("Finished"));

                qDebug() << "Initial energy:"
                         << simulationResult.frames.front().energy;

                qDebug() << "Final energy:"
                         << simulationResult.frames.back().energy;

                qDebug() << "Completed iterations:"
                         << simulationResult.completedSweeps;

                qDebug() << "Saved frames:"
                         << simulationResult.frames.size();

                qDebug() << "Energy tolerance reached:"
                         << simulationResult.converged;

                QMessageBox::information(
                    this,
                    QStringLiteral("Simulation finished"),
                    QStringLiteral("Simulation completed.")
                    );
            }, Qt::QueuedConnection);

    ui->calculateButton->setEnabled(false);

    ui->calculationStatusLabel->setText(
        QStringLiteral("Running..."));

    ui->simulationProgressBar->setRange(0, 0);
    ui->simulationProgressBar->setValue(0);

    worker->start();
}
ParametersDialog::~ParametersDialog()
{
    delete ui;
}
const UrFaustSim::SimulationResult &
ParametersDialog::getSimulationResult() const
{
    return simulationResult;
}