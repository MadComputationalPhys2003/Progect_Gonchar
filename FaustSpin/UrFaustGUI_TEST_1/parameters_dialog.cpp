#include "parameters_dialog.h"
#include "ui_parameters_dialog.h"
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <QString>
#include "Write_To_File.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QTemporaryDir>
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
    connect(ui->saveTxtButton, &QPushButton::clicked,
            this, &ParametersDialog::saveTxtResults);

    connect(ui->saveTexButton, &QPushButton::clicked,
            this, &ParametersDialog::saveTexResult);
    setWindowFlags(
        (windowFlags() & ~Qt::WindowContextHelpButtonHint)
        | Qt::CustomizeWindowHint
        | Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowCloseButtonHint
        );
    connect(ui->calculateButton, &QPushButton::clicked,
            this, &ParametersDialog::startSimulation);
    connect(ui->systemResultsButton, &QPushButton::clicked,
            this, &ParametersDialog::systemResultsRequested);

    connect(ui->energyPlotButton, &QPushButton::clicked,
            this, &ParametersDialog::energyPlotRequested);

    connect(ui->spinVisualizationButton, &QPushButton::clicked,
            this, &ParametersDialog::spinVisualizationRequested);
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

    // Индивидуальные длины заполним в рабочем потоке.


    config.epsilon =
        ui->EnergyErrorDoubleSpinBox->value();

    config.maxSweeps =
        static_cast<std::size_t>(ui->spinBox->value());

    config.initialization =
        static_cast<InitIons::Ions::SpinDirectionInit>(
            ui->InitialDirectionComboBox->currentIndex());

    return config;
}
void ParametersDialog::startSimulation()
{
    auto config = readConfig();

    const double minimumLength =
        ui->SpinLengthMinDoubleSpinBox->value();

    const double maximumLength =
        ui->SpinLengthMaxDoubleSpinBox->value();

    if (!std::isfinite(minimumLength) ||
        !std::isfinite(maximumLength) ||
        minimumLength <= 0.0 ||
        minimumLength >= maximumLength) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid parameters"),
            QStringLiteral("Spin lengths must satisfy: 0 < min < max.")
            );
        return;
    }
    const double minimumExchange =
        ui->ExchangeIntegralMinDoubleSpinBox->value();

    const double maximumExchange =
        ui->ExchangeIntegralMaxDoubleSpinBox->value();

    if (!std::isfinite(minimumExchange) ||
        !std::isfinite(maximumExchange) ||
        minimumExchange >= maximumExchange) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid parameters"),
            QStringLiteral(
                "Exchange matrix elements must satisfy: min < max."
                )
            );
        return;
    }
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

    auto* worker = QThread::create(
        [config, outcome,
         minimumLength, maximumLength,
         minimumExchange, maximumExchange]() mutable
        {
            try {
                const auto checkedProduct =
                    [](std::size_t left, std::uint64_t right)
                {
                    if (left == 0 || right == 0 ||
                        right >
                            std::numeric_limits<std::size_t>::max() / left) {
                        throw std::invalid_argument(
                            "Invalid lattice size"
                            );
                    }

                    return left * static_cast<std::size_t>(right);
                };

                std::size_t count = checkedProduct(1, config.lx);
                count = checkedProduct(count, config.ly);
                count = checkedProduct(count, config.lz);

                std::random_device seed;
                std::mt19937_64 generator(seed());

                std::uniform_real_distribution<double> distribution(
                    minimumLength,
                    maximumLength
                    );

                std::vector<double> lengths(count);

                for (double& length : lengths) {
                    length = distribution(generator);
                }

                config.spinLengthMode =
                    InitIons::Ions::SpinLengthInit::different;

                config.spinLength = std::move(lengths);
                std::size_t neighborCount = 0;

                switch (config.dimension) {
                case UrFaust::Dimension::one:
                    neighborCount = 2;
                    break;

                case UrFaust::Dimension::two:
                    neighborCount = 4;
                    break;

                case UrFaust::Dimension::three:
                    neighborCount = 6;
                    break;

                default:
                    throw std::invalid_argument(
                        "Unknown lattice dimension"
                        );
                }

                const std::size_t matrixCount =
                    checkedProduct(count, neighborCount);

                std::vector<FM::Matrix> exchangeMatrices(
                    matrixCount,
                    FM::Matrix(3, 3)
                    );

                std::uniform_real_distribution<double> exchangeDistribution(
                    minimumExchange,
                    maximumExchange
                    );

                for (FM::Matrix& matrix : exchangeMatrices) {
                    for (std::size_t element = 0; element < 9; ++element) {
                        matrix[element] = exchangeDistribution(generator);
                    }
                }

                config.exchangeMode =
                    ExchangeIntegral::ExchangeIntegralInit::matrix;

                config.exchangeIntegral = std::move(exchangeMatrices);
                UrFaustSim::Simulation simulation(std::move(config));
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
                emit simulationFinished();
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
                qDebug() << "Spin lengths count:"
                         << simulationResult.spinLengths.size();

                qDebug() << "First spin length:"
                         << simulationResult.spinLengths.front();

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
void ParametersDialog::saveTxtResults()
{
    if (simulationResult.frames.empty()) {
        QMessageBox::information(
            this, QStringLiteral("No results"),
            QStringLiteral("Run a simulation first."));
        return;
    }

    const QString parentDirectory =
        QFileDialog::getExistingDirectory(
            this, QStringLiteral("Choose output directory"));

    if (parentDirectory.isEmpty()) {
        return;
    }

    QTemporaryDir directory(QDir(parentDirectory).filePath(
        QStringLiteral("UrFaust_") +
        QDateTime::currentDateTime().toString(
            QStringLiteral("yyyyMMdd_HHmmss")) +
        QStringLiteral("_XXXXXX")));

    if (!directory.isValid()) {
        QMessageBox::critical(
            this, QStringLiteral("Export error"),
            directory.errorString());
        return;
    }

    try {
        WriteFileUrFaust::write_all_txt(
            simulationResult,
            QFileInfo(directory.path()).filesystemFilePath());

        directory.setAutoRemove(false);
        lastExportDirectory = directory.path();

        QMessageBox::information(
            this, QStringLiteral("Saved"),
            QStringLiteral("Files saved to:\n%1")
                .arg(lastExportDirectory));
    }
    catch (const std::exception& error) {
        QMessageBox::critical(
            this, QStringLiteral("Export error"),
            QString::fromUtf8(error.what()));
    }
}

void ParametersDialog::saveTexResult()
{
    if (simulationResult.frames.empty()) {
        QMessageBox::information(
            this, QStringLiteral("No results"),
            QStringLiteral("Run a simulation first."));
        return;
    }

    const QString templateFile = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Choose UrFaustResult.tex template"),
        QString(),
        QStringLiteral("LaTeX files (*.tex)"));

    if (templateFile.isEmpty()) {
        return;
    }

    QFileDialog outputDialog(this);
    outputDialog.setWindowTitle(QStringLiteral("Save LaTeX report"));
    outputDialog.setAcceptMode(QFileDialog::AcceptSave);
    outputDialog.setFileMode(QFileDialog::AnyFile);
    outputDialog.setNameFilter(QStringLiteral("LaTeX files (*.tex)"));
    outputDialog.setDefaultSuffix(QStringLiteral("tex"));

    if (!lastExportDirectory.isEmpty()) {
        outputDialog.setDirectory(lastExportDirectory);
    }

    outputDialog.selectFile(QStringLiteral("UrFaustResult.tex"));

    if (outputDialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString outputFile =
        outputDialog.selectedFiles().front();

    try {
        WriteFileUrFaust::write_result_tex(
            simulationResult,
            QFileInfo(templateFile).filesystemFilePath(),
            QFileInfo(outputFile).filesystemFilePath(),
            simulationResult.config.texRowsPerPage);

        QMessageBox::information(
            this, QStringLiteral("Saved"),
            QStringLiteral("Report saved to:\n%1").arg(outputFile));
    }
    catch (const std::exception& error) {
        QMessageBox::critical(
            this, QStringLiteral("Export error"),
            QString::fromUtf8(error.what()));
    }
}