#include "parameters_dialog.h"
#include "ui_parameters_dialog.h"
#include "parallel_direction_dialog.h"
#include "random_direction_dialog.h"
#include "manual_spin_length_dialog.h"
#include "random_spin_length_dialog.h"
#include "manual_exchange_integral_dialog.h"
#include "random_exchange_integral_dialog.h"
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <QString>
#include <QComboBox>
#include "Write_To_File.h"
#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QLocale>
#include <QTextStream>
#include <QByteArray>
#include <QFile>
#include <QFileDevice>
#include <QFileDialog>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <utility>
#include "manual_direction_dialog.h"
#include <exception>
#include <limits>
#include <cmath>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
ParametersDialog::ParametersDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ParametersDialog)
{
    ui->setupUi(this);
    connect(
        ui->exchangeSettingsButton,
        &QPushButton::clicked,
        this,
        &ParametersDialog::configureExchangeIntegral
        );

    connect(
        ui->exchangeInputComboBox,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            updateExchangeControls();
        }
        );

    updateExchangeControls();
    connect(
        ui->spinLengthInputComboBox,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            updateSpinLengthControls();
        }
        );

    updateSpinLengthControls();
    connect(
        ui->spinLengthSettingsButton,
        &QPushButton::clicked,
        this,
        &ParametersDialog::configureSpinLength
        );
    connect(ui->initialDirectionSettingsButton,
            &QPushButton::clicked,
            this, &ParametersDialog::configureInitialDirections);
    connect(ui->InitialDirectionComboBox,
            &QComboBox::currentIndexChanged,
            this, [this](int) {
                updateInitialDirectionControls();
            });

    updateInitialDirectionControls();
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


    if (ui->spinLengthModeComboBox->currentIndex() == 0) {
        config.spinLengthMode =
            InitIons::Ions::SpinLengthInit::constant;

        config.spinLength = constantSpinLength;
    }

    config.epsilon =
        ui->EnergyErrorDoubleSpinBox->value();

    config.maxSweeps =
        static_cast<std::size_t>(ui->spinBox->value());

    const int directionIndex =
        ui->InitialDirectionComboBox->currentIndex();

    config.initialization = directionIndex == 3
                                ? InitIons::Ions::SpinDirectionInit::file
                                : static_cast<InitIons::Ions::SpinDirectionInit>(
                                      directionIndex
                                      );
    config.parallelDirection = parallelDirection;
    config.directionSeed = randomDirectionSeed;
    config.directionFile =
        QFile::encodeName(initialDirectionFileName).toStdString();
    if (ui->exchangeModeComboBox->currentIndex() == 0) {
        config.exchangeMode =
            ExchangeIntegral::ExchangeIntegralInit::constant;

        config.exchangeIntegral = constantExchangeIntegral;
    }
    return config;
}
void ParametersDialog::startSimulation()
{
    const int exchangeModeIndex =
        ui->exchangeModeComboBox->currentIndex();

    const int exchangeInputIndex =
        ui->exchangeInputComboBox->currentIndex();

    const bool constantManualExchange =
        exchangeModeIndex == 0 && exchangeInputIndex == 0;

    const bool manualMatrixExchange =
        exchangeModeIndex == 1 && exchangeInputIndex == 0;

    const bool randomExchangeMatrices =
        exchangeModeIndex == 1 && exchangeInputIndex == 1;

    if (exchangeInputIndex != 2 &&
        !constantManualExchange &&
        !manualMatrixExchange &&
        !randomExchangeMatrices) {
        QMessageBox::warning(
            this,
            QStringLiteral("Exchange integral"),
            QStringLiteral(
                "Select Constant + Manual, Matrix + Manual, "
                "Matrix + Random, or File input."
                )
            );

        return;
    }
    const int lengthModeIndex =
        ui->spinLengthModeComboBox->currentIndex();

    const int lengthInputIndex =
        ui->spinLengthInputComboBox->currentIndex();

    const bool constantManual =
        lengthModeIndex == 0 && lengthInputIndex == 0;

    const bool differentManual =
        lengthModeIndex == 1 && lengthInputIndex == 0;

    const bool differentLengths =
        lengthModeIndex == 1 && lengthInputIndex == 1;

    const bool lengthFromFile =
        lengthInputIndex == 2;

    if (!constantManual &&
        !differentManual &&
        !differentLengths &&
        !lengthFromFile) {
        QMessageBox::warning(
            this,
            QStringLiteral("Spin length"),
            QStringLiteral(
                "Select Constant + Manual, Different + Manual, "
                "Different + Random, or File input."
                )
            );

        return;
    }
     auto config = readConfig();
    if (exchangeInputIndex == 2) {
        if (!exchangeIntegralFileLoaded) {
            QMessageBox::warning(
                this,
                QStringLiteral("Exchange integrals file"),
                QStringLiteral(
                    "Choose an exchange integrals file first."
                    )
                );

            return;
        }

        if (exchangeModeIndex == 0) {
            if (!std::holds_alternative<double>(
                    exchangeIntegralFromFile)) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Exchange integrals file"),
                    QStringLiteral(
                        "Choose a file containing one number "
                        "for Constant mode."
                        )
                    );

                return;
            }

            config.exchangeMode =
                ExchangeIntegral::ExchangeIntegralInit::constant;
        }
        else if (exchangeModeIndex == 1) {
            const std::array<std::uint64_t, 4> currentLatticeKey{
                static_cast<std::uint64_t>(config.dimension),
                config.lx,
                config.ly,
                config.lz
            };

            const std::vector<FM::Matrix>* matrices =
                std::get_if<std::vector<FM::Matrix>>(
                    &exchangeIntegralFromFile
                    );

            if (matrices == nullptr ||
                matrices->empty() ||
                currentLatticeKey != exchangeFileLatticeKey) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Exchange integrals file"),
                    QStringLiteral(
                        "Choose a matrix file for the current "
                        "lattice dimension and sizes."
                        )
                    );

                return;
            }

            config.exchangeMode =
                ExchangeIntegral::ExchangeIntegralInit::matrix;
        }
        else {
            QMessageBox::warning(
                this,
                QStringLiteral("Exchange integral"),
                QStringLiteral("Unknown exchange mode.")
                );

            return;
        }

        config.exchangeIntegral = exchangeIntegralFromFile;
    }

    if (manualMatrixExchange) {
        const std::array<std::uint64_t, 4> currentLatticeKey{
            static_cast<std::uint64_t>(config.dimension),
            config.lx,
            config.ly,
            config.lz
        };

        if (manualExchangeMatrices.empty() ||
            currentLatticeKey != manualExchangeLatticeKey) {
            QMessageBox::warning(
                this,
                QStringLiteral("Exchange matrices"),
                QStringLiteral(
                    "Open Set Manual and confirm exchange matrices "
                    "for the current lattice."
                    )
                );

            return;
        }

        config.exchangeMode =
            ExchangeIntegral::ExchangeIntegralInit::matrix;

        config.exchangeIntegral = manualExchangeMatrices;
    }
    if (lengthFromFile) {
        if (!spinLengthFileLoaded) {
            QMessageBox::warning(
                this,
                QStringLiteral("Spin lengths file"),
                QStringLiteral("Choose a spin lengths file first.")
                );

            return;
        }

        if (lengthModeIndex == 0) {
            const double* value =
                std::get_if<double>(&spinLengthsFromFile);

            if (value == nullptr) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Spin lengths file"),
                    QStringLiteral(
                        "Choose a file containing one value "
                        "for Constant mode."
                        )
                    );

                return;
            }

            config.spinLengthMode =
                InitIons::Ions::SpinLengthInit::constant;

            config.spinLength = *value;
        }
        else if (lengthModeIndex == 1) {
            std::size_t count = 1;

            const std::uint64_t latticeSizes[] = {
                config.lx,
                config.ly,
                config.lz
            };

            for (const std::uint64_t size : latticeSizes) {
                if (size == 0 ||
                    size >
                        std::numeric_limits<std::size_t>::max() / count) {
                    QMessageBox::warning(
                        this,
                        QStringLiteral("Invalid parameters"),
                        QStringLiteral("Invalid lattice size.")
                        );

                    return;
                }

                count *= static_cast<std::size_t>(size);
            }

            const std::vector<double>* lengths =
                std::get_if<std::vector<double>>(&spinLengthsFromFile);

            if (lengths == nullptr || lengths->size() != count) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Spin lengths file"),
                    QStringLiteral(
                        "Choose a file containing one spin length "
                        "per ion for the current lattice."
                        )
                    );

                return;
            }

            config.spinLengthMode =
                InitIons::Ions::SpinLengthInit::different;

            config.spinLength = *lengths;
        }
        else {
            QMessageBox::warning(
                this,
                QStringLiteral("Spin length"),
                QStringLiteral("Unknown spin length mode.")
                );

            return;
        }
    }
    if (differentManual) {
        std::size_t count = 1;

        const std::uint64_t latticeSizes[] = {
            config.lx,
            config.ly,
            config.lz
        };

        for (const std::uint64_t size : latticeSizes) {
            if (size == 0 ||
                size >
                    std::numeric_limits<std::size_t>::max() / count) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Invalid parameters"),
                    QStringLiteral("Invalid lattice size.")
                    );
                return;
            }

            count *= static_cast<std::size_t>(size);
        }

        if (manualSpinLengths.size() != count) {
            QMessageBox::warning(
                this,
                QStringLiteral("Manual spin lengths"),
                QStringLiteral(
                    "Open Set Manual and confirm spin lengths "
                    "for the current lattice size."
                    )
                );
            return;
        }

        config.spinLengthMode =
            InitIons::Ions::SpinLengthInit::different;

        config.spinLength = manualSpinLengths;
    }
    const bool manualMode =
        ui->InitialDirectionComboBox->currentIndex() == 3;

    std::vector<Vector3D_Decart::Vector3D_Dec> directionsForRun;

    if (manualMode) {
        std::size_t count = 1;

        const std::uint64_t lengths[] = {
            config.lx,
            config.ly,
            config.lz
        };

        for (const std::uint64_t length : lengths) {
            if (length == 0 ||
                length >
                    std::numeric_limits<std::size_t>::max() / count) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Invalid parameters"),
                    QStringLiteral("Invalid lattice size.")
                    );
                return;
            }

            count *= static_cast<std::size_t>(length);
        }

        if (manualDirections.size() != count) {
            QMessageBox::warning(
                this,
                QStringLiteral("Manual directions"),
                QStringLiteral(
                    "Open Set Manual and confirm directions "
                    "for the current lattice size."
                    )
                );
            return;
        }

        directionsForRun = manualDirections;
    }

    const double minimumLength = randomSpinLengthMinimum;
    const double maximumLength = randomSpinLengthMaximum;
    if (differentLengths &&
        (!std::isfinite(minimumLength) ||
         !std::isfinite(maximumLength) ||
         minimumLength <= 0.0 ||
         minimumLength >= maximumLength)) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid parameters"),
            QStringLiteral(
                "Spin lengths must satisfy: 0 < min < max."
                )
            );
        return;
    }

    const double minimumExchange = randomExchangeMinimum;
    const double maximumExchange = randomExchangeMaximum;
    if (randomExchangeMatrices &&
        (!std::isfinite(minimumExchange) ||
         !std::isfinite(maximumExchange) ||
         minimumExchange >= maximumExchange)) {
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
         manualMode,
         runDirections = std::move(directionsForRun),
         minimumLength, maximumLength,
         minimumExchange, maximumExchange,
         differentLengths,
         randomExchangeMatrices]() mutable
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

                if (differentLengths) {
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
                }
                if (randomExchangeMatrices) {
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

                    std::uniform_real_distribution<double>
                        exchangeDistribution(
                            minimumExchange,
                            maximumExchange
                            );

                    for (FM::Matrix& matrix : exchangeMatrices) {
                        for (std::size_t element = 0; element < 9; ++element) {
                            matrix[element] =
                                exchangeDistribution(generator);
                        }
                    }

                    config.exchangeMode =
                        ExchangeIntegral::ExchangeIntegralInit::matrix;

                    config.exchangeIntegral =
                        std::move(exchangeMatrices);
                }
                std::unique_ptr<QTemporaryDir> manualInputDirectory;

                if (manualMode) {
                    manualInputDirectory =
                        std::make_unique<QTemporaryDir>();

                    if (!manualInputDirectory->isValid()) {
                        throw std::runtime_error(
                            "Cannot create temporary directions directory: " +
                            manualInputDirectory->errorString().toStdString()
                            );
                    }

                    const QString filename =
                        manualInputDirectory->filePath(
                            QStringLiteral("directions.txt")
                            );

                    const QByteArray encodedPath = QFile::encodeName(filename);

                    if (QFile::decodeName(encodedPath).normalized(
                            QString::NormalizationForm_C
                            ) != filename.normalized(
                            QString::NormalizationForm_C
                            )) {
                        throw std::runtime_error(
                            "The temporary directory path cannot be represented "
                            "for the current spin directions reader."
                            );
                    }

                    QFile directionFile(filename);

                    if (!directionFile.open(
                            QIODevice::WriteOnly | QIODevice::Text
                            )) {
                        throw std::runtime_error(
                            "Cannot create temporary directions file: " +
                            directionFile.errorString().toStdString()
                            );
                    }

                    {
                        QTextStream output(&directionFile);

                        output.setLocale(QLocale::c());
                        output.setGenerateByteOrderMark(false);
                        output.setRealNumberNotation(QTextStream::SmartNotation);

                        output.setRealNumberPrecision(
                            std::numeric_limits<double>::max_digits10
                            );

                        for (const auto& direction : runDirections) {
                            output << direction[0] << ' '
                                   << direction[1] << ' '
                                   << direction[2] << '\n';
                        }

                        output.flush();

                        if (output.status() != QTextStream::Ok) {
                            throw std::runtime_error(
                                "Cannot write temporary spin directions."
                                );
                        }
                    }

                    if (!directionFile.flush()) {
                        throw std::runtime_error(
                            "Cannot flush temporary directions file: " +
                            directionFile.errorString().toStdString()
                            );
                    }

                    directionFile.close();

                    config.directionFile = encodedPath.toStdString();
                }
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

    const UrFaustSim::SimulationResult&
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
void ParametersDialog::updateInitialDirectionControls()
{
    ui->initialDirectionFileLineEdit->setVisible(
        ui->InitialDirectionComboBox->currentIndex() == 2);
    switch (ui->InitialDirectionComboBox->currentIndex()) {
    case 0:
        ui->initialDirectionSettingsButton->setText(
            QStringLiteral("Set Parallel…"));
        break;

    case 1:
        ui->initialDirectionSettingsButton->setText(
            QStringLiteral("Set Random…"));
        break;

    case 2:
        ui->initialDirectionSettingsButton->setText(
            QStringLiteral("Choose File…"));
        break;

    case 3:
        ui->initialDirectionSettingsButton->setText(
            QStringLiteral("Set Manual…"));
        break;

    default:
        break;
    }
}
void ParametersDialog::configureInitialDirections()
{
    if (ui->InitialDirectionComboBox->currentIndex() == 2) {
        const QString filename = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("Choose spin directions file"),
            initialDirectionFileName,
            QStringLiteral("Text files (*.txt);;All files (*)"));

        if (filename.isEmpty()) {
            return;
        }

        const QByteArray encoded = QFile::encodeName(filename);

        if (QFile::decodeName(encoded).normalized(QString::NormalizationForm_C) !=
            filename.normalized(QString::NormalizationForm_C)) {
            QMessageBox::warning(
                this,
                QStringLiteral("Unsupported path"),
                QStringLiteral(
                    "The selected path cannot be represented "
                    "for the current file reader. "
                    "Choose a path using Latin letters."));
            return;
        }

        initialDirectionFileName = filename;
        ui->initialDirectionFileLineEdit->setText(filename);

        qDebug() << "Directions file:" << filename;
        return;
    }
    if (ui->InitialDirectionComboBox->currentIndex() == 1) {
        auto* dialog = new RandomDirectionDialog(this);

        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setSeed(randomDirectionSeed);

        connect(dialog, &QDialog::accepted,
                this, [this, dialog]()
                {
                    randomDirectionSeed = dialog->getSeed();

                    if (randomDirectionSeed.has_value()) {
                        qDebug() << "Direction seed:"
                                 << *randomDirectionSeed;
                    }
                    else {
                        qDebug() << "Direction seed: automatic";
                    }
                });

        dialog->open();
        return;
    }
    if (ui->InitialDirectionComboBox->currentIndex() == 3) {
        const int dimension =
            ui->SystemDimentionComboBox->currentIndex();

        if (dimension < 0 || dimension > 2) {
            QMessageBox::warning(
                this,
                QStringLiteral("Invalid parameters"),
                QStringLiteral("Choose the system dimension.")
                );
            return;
        }

        const int lengths[] = {
            ui->LxSpinBox->value(),
            dimension >= 1 ? ui->LySpinBox->value() : 1,
            dimension >= 2 ? ui->LzSpinBox->value() : 1
        };

        std::size_t count = 1;

        for (const int length : lengths) {
            if (length <= 0 ||
                static_cast<std::size_t>(length) >
                    std::numeric_limits<std::size_t>::max() / count) {
                QMessageBox::warning(
                    this,
                    QStringLiteral("Invalid parameters"),
                    QStringLiteral("Invalid lattice size.")
                    );
                return;
            }

            count *= static_cast<std::size_t>(length);
        }

        ManualDirectionDialog* dialog = nullptr;

        try {
            dialog = new ManualDirectionDialog(this);
            dialog->setDirections(count, manualDirections);
        }
        catch (const std::exception& error) {
            delete dialog;

            QMessageBox::warning(
                this,
                QStringLiteral("Cannot open directions table"),
                QString::fromUtf8(error.what())
                );
            return;
        }

        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(
            dialog,
            &QDialog::accepted,
            this,
            [this, dialog]()
            {
                manualDirections = dialog->getDirections();

                qDebug() << "Manual directions:"
                         << static_cast<qulonglong>(
                                manualDirections.size()
                                );
            }
            );

        dialog->open();
        return;
    }
    // На этом шаге подключаем окно для Parallel.
    if (ui->InitialDirectionComboBox->currentIndex() != 0) {
        return;
    }

    auto* dialog = new ParallelDirectionDialog(this);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setDirection(parallelDirection);

    connect(dialog, &QDialog::accepted,
            this, [this, dialog]()
            {
                parallelDirection = dialog->getDirection();

                qDebug() << "Selected parallel direction:"
                         << parallelDirection[0]
                         << parallelDirection[1]
                         << parallelDirection[2];
            });

    dialog->open();
}
void ParametersDialog::configureSpinLength()
{
    if (ui->spinLengthInputComboBox->currentIndex() == 2) {
        chooseSpinLengthFile();
        return;
    }
    if (ui->spinLengthInputComboBox->currentIndex() == 1) {
        if (ui->spinLengthModeComboBox->currentIndex() != 1) {
            QMessageBox::information(
                this,
                QStringLiteral("Spin length"),
                QStringLiteral(
                    "Select Different mode for random spin lengths."
                    )
                );
           \

            return;
        }

        auto* dialog = new RandomSpinLengthDialog(this);

        dialog->setBounds(
            randomSpinLengthMinimum,
            randomSpinLengthMaximum
            );

        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(
            dialog,
            &QDialog::accepted,
            this,
            [this, dialog]() {
                randomSpinLengthMinimum =
                    dialog->getMinimumLength();

                randomSpinLengthMaximum =
                    dialog->getMaximumLength();

                qDebug() << "Random spin length range:"
                         << randomSpinLengthMinimum
                         << randomSpinLengthMaximum;
            }
            );

        dialog->open();
        return;
    }
    if (ui->spinLengthInputComboBox->currentIndex() != 0) {
        QMessageBox::information(
            this,
            QStringLiteral("Spin length"),
            QStringLiteral("Select Manual input.")
            );
        return;
    }

    const int lengthMode =
        ui->spinLengthModeComboBox->currentIndex();

    ManualSpinLengthDialog* dialog = nullptr;

    try {
        dialog = new ManualSpinLengthDialog(this);

        if (lengthMode == 0) {
            dialog->setSpinLength(constantSpinLength);
        }
        else if (lengthMode == 1) {
            const auto config = readConfig();

            std::size_t count = 1;

            const std::uint64_t latticeSizes[] = {
                config.lx,
                config.ly,
                config.lz
            };

            for (const std::uint64_t size : latticeSizes) {
                if (size == 0 ||
                    size >
                        std::numeric_limits<std::size_t>::max()
                            / count) {
                    throw std::invalid_argument(
                        "Invalid lattice size"
                        );
                }

                count *= static_cast<std::size_t>(size);
            }

            dialog->setSpinLengths(
                count,
                manualSpinLengths,
                constantSpinLength
                );
        }
        else {
            throw std::invalid_argument(
                "Unknown spin length mode"
                );
        }
    }
    catch (const std::exception& error) {
        delete dialog;

        QMessageBox::warning(
            this,
            QStringLiteral("Cannot open spin length settings"),
            QString::fromUtf8(error.what())
            );
        return;
    }

    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(
        dialog,
        &QDialog::accepted,
        this,
        [this, dialog, lengthMode]()
        {
            if (lengthMode == 0) {
                constantSpinLength = dialog->getSpinLength();

                qDebug() << "Constant spin length:"
                         << constantSpinLength;
            }
            else {
                manualSpinLengths = dialog->getSpinLengths();

                qDebug() << "Manual spin lengths count:"
                         << static_cast<qulonglong>(
                                manualSpinLengths.size()
                                );
            }
        }
        );

    dialog->open();
}
void ParametersDialog::updateSpinLengthControls()
{
    ui->spinLengthFileLineEdit->setVisible(
        ui->spinLengthInputComboBox->currentIndex() == 2
        );

    ui->spinLengthFileLineEdit->setText(
        spinLengthFileName
        );

    ui->spinLengthFileLineEdit->setToolTip(
        spinLengthFileName
        );
    switch (ui->spinLengthInputComboBox->currentIndex()) {
    case 0:
        ui->spinLengthSettingsButton->setText(
            QStringLiteral("Set Manual...")
            );
        break;

    case 1:
        ui->spinLengthSettingsButton->setText(
            QStringLiteral("Set Random...")
            );
        break;

    case 2:
        ui->spinLengthSettingsButton->setText(
            QStringLiteral("Choose File...")
            );
        break;
    }
}
void ParametersDialog::chooseSpinLengthFile()
{
    const QString filename = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Choose spin lengths file"),
        spinLengthFileName,
        QStringLiteral("Text files (*.txt);;All files (*)")
        );

    if (filename.isEmpty()) {
        return;
    }

    try {
        const int lengthMode =
            ui->spinLengthModeComboBox->currentIndex();

        std::size_t count = 1;

        if (lengthMode == 1) {
            const auto config = readConfig();

            const std::uint64_t latticeSizes[] = {
                config.lx,
                config.ly,
                config.lz
            };

            for (const std::uint64_t size : latticeSizes) {
                if (size == 0 ||
                    size >
                        std::numeric_limits<std::size_t>::max() / count) {
                    throw std::invalid_argument(
                        "Invalid lattice size"
                        );
                }

                count *= static_cast<std::size_t>(size);
            }
        }
        else if (lengthMode != 0) {
            throw std::invalid_argument(
                "Unknown spin length mode"
                );
        }

        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error(
                "Cannot open spin lengths file"
                );
        }

        QByteArray contents = file.readAll();

        if (file.error() != QFileDevice::NoError) {
            throw std::runtime_error(
                "Cannot read spin lengths file"
                );
        }

        // Допускаем UTF-8 BOM в начале файла.
        if (contents.startsWith("\xEF\xBB\xBF")) {
            contents.remove(0, 3);
        }

        std::istringstream input(contents.toStdString());
        input.imbue(std::locale::classic());

        std::vector<double> lengths;
        lengths.reserve(count);

        for (std::size_t i = 0; i < count; ++i) {
            double value = 0.0;

            if (!(input >> value)) {
                throw std::runtime_error(
                    "Cannot read spin length at index " +
                    std::to_string(i)
                    );
            }

            if (!std::isfinite(value) || value <= 0.0) {
                throw std::invalid_argument(
                    "Spin length must be finite and positive; index " +
                    std::to_string(i)
                    );
            }

            lengths.push_back(value);
        }

        input >> std::ws;

        if (input.bad() || !input.eof()) {
            throw std::runtime_error(
                "Spin lengths file contains extra data"
                );
        }

        // Сохраняем значения только после проверки всего файла.
        if (lengthMode == 0) {
            spinLengthsFromFile = lengths.front();
        }
        else {
            spinLengthsFromFile = std::move(lengths);
        }

        spinLengthFileName = filename;
        spinLengthFileLoaded = true;
        updateSpinLengthControls();
        QMessageBox::information(
            this,
            QStringLiteral("Spin lengths loaded"),
            QStringLiteral("Loaded values: %1\nFile: %2")
                .arg(static_cast<qulonglong>(count))
                .arg(spinLengthFileName)
            );
    }
    catch (const std::exception& error) {
        QMessageBox::warning(
            this,
            QStringLiteral("Spin lengths file"),
            QString::fromUtf8(error.what())
            );
    }
}
void ParametersDialog::updateExchangeControls()
{
    ui->exchangeIntegralFileLineEdit->setVisible(
        ui->exchangeInputComboBox->currentIndex() == 2
        );

    ui->exchangeIntegralFileLineEdit->setText(
        exchangeIntegralFileName
        );

    ui->exchangeIntegralFileLineEdit->setToolTip(
        exchangeIntegralFileName
        );
    switch (ui->exchangeInputComboBox->currentIndex()) {
    case 0:
        ui->exchangeSettingsButton->setText(
            QStringLiteral("Set Manual...")
            );
        break;

    case 1:
        ui->exchangeSettingsButton->setText(
            QStringLiteral("Set Random...")
            );
        break;

    case 2:
        ui->exchangeSettingsButton->setText(
            QStringLiteral("Choose File...")
            );
        break;
    }
}

void ParametersDialog::configureExchangeIntegral()
{
    if (ui->exchangeInputComboBox->currentIndex() == 2) {
        chooseExchangeIntegralFile();
        return;
    }
    if (ui->exchangeInputComboBox->currentIndex() == 1) {
        if (ui->exchangeModeComboBox->currentIndex() != 1) {
            QMessageBox::information(
                this,
                QStringLiteral("Exchange integral"),
                QStringLiteral(
                    "Select Matrix mode for random exchange matrices."
                    )
                );

            return;
        }

        auto* dialog = new RandomExchangeIntegralDialog(this);

        dialog->setBounds(
            randomExchangeMinimum,
            randomExchangeMaximum
            );

        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(
            dialog,
            &QDialog::accepted,
            this,
            [this, dialog]() {
                randomExchangeMinimum =
                    dialog->getMinimumExchange();

                randomExchangeMaximum =
                    dialog->getMaximumExchange();

                qDebug() << "Random exchange range:"
                         << randomExchangeMinimum
                         << randomExchangeMaximum;
            }
            );

        dialog->open();
        return;
    }
    if (ui->exchangeInputComboBox->currentIndex() != 0) {
        QMessageBox::information(
            this,
            QStringLiteral("Exchange integral"),
            QStringLiteral("Select Manual input.")
            );

        return;
    }

    const int exchangeMode =
        ui->exchangeModeComboBox->currentIndex();

    ManualExchangeIntegralDialog* dialog = nullptr;

    std::array<std::uint64_t, 4> latticeKey{};

    try {
        dialog = new ManualExchangeIntegralDialog(this);

        if (exchangeMode == 0) {
            dialog->setExchangeIntegral(
                constantExchangeIntegral
                );
        }
        else if (exchangeMode == 1) {
            const auto config = readConfig();

            const int dimensionIndex =
                static_cast<int>(config.dimension);

            if (dimensionIndex < 0 || dimensionIndex > 2) {
                throw std::invalid_argument(
                    "Unknown lattice dimension"
                    );
            }

            const std::size_t neighborCount =
                2 * (
                    static_cast<std::size_t>(dimensionIndex) + 1
                    );

            std::size_t ionCount = 1;

            const std::uint64_t latticeSizes[] = {
                config.lx,
                config.ly,
                config.lz
            };

            for (const std::uint64_t size : latticeSizes) {
                if (size == 0 ||
                    size >
                        std::numeric_limits<std::size_t>::max()
                            / ionCount) {
                    throw std::invalid_argument(
                        "Invalid lattice size"
                        );
                }

                ionCount *= static_cast<std::size_t>(size);
            }

            latticeKey = {
                static_cast<std::uint64_t>(dimensionIndex),
                config.lx,
                config.ly,
                config.lz
            };

            const std::vector<FM::Matrix> noPrevious;

            const std::vector<FM::Matrix>& previous =
                latticeKey == manualExchangeLatticeKey
                    ? manualExchangeMatrices
                    : noPrevious;

            dialog->setMatrices(
                ionCount,
                neighborCount,
                previous,
                constantExchangeIntegral
                );
        }
        else {
            throw std::invalid_argument(
                "Unknown exchange mode"
                );
        }
    }
    catch (const std::exception& error) {
        delete dialog;

        QMessageBox::warning(
            this,
            QStringLiteral("Exchange integral"),
            QString::fromUtf8(error.what())
            );

        return;
    }

    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(
        dialog,
        &QDialog::accepted,
        this,
        [this, dialog, exchangeMode, latticeKey]() {
            if (exchangeMode == 0) {
                constantExchangeIntegral =
                    dialog->getExchangeIntegral();

                qDebug() << "Constant exchange integral:"
                         << constantExchangeIntegral;
            }
            else {
                manualExchangeMatrices =
                    dialog->getMatrices();

                manualExchangeLatticeKey = latticeKey;

                qDebug() << "Manual exchange matrices:"
                         << static_cast<qulonglong>(
                                manualExchangeMatrices.size()
                                );
            }
        }
        );

    dialog->open();
}
void ParametersDialog::chooseExchangeIntegralFile()
{
    const QString filename = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Choose exchange integrals file"),
        exchangeIntegralFileName,
        QStringLiteral("Text files (*.txt);;All files (*)")
        );

    if (filename.isEmpty()) {
        return;
    }

    try {
        const int mode =
            ui->exchangeModeComboBox->currentIndex();

        if (mode != 0 && mode != 1) {
            throw std::invalid_argument(
                "Unknown exchange mode"
                );
        }

        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error(
                "Cannot open exchange integrals file"
                );
        }

        QByteArray contents = file.readAll();

        if (file.error() != QFileDevice::NoError) {
            throw std::runtime_error(
                "Cannot read exchange integrals file"
                );
        }

        file.close();

        if (contents.startsWith("\xEF\xBB\xBF")) {
            contents.remove(0, 3);
        }

        std::istringstream input(contents.toStdString());
        input.imbue(std::locale::classic());

        std::variant<double, std::vector<FM::Matrix>> values;

        std::array<std::uint64_t, 4> latticeKey{};

        if (mode == 0) {
            double value = 0.0;

            if (!(input >> value) || !std::isfinite(value)) {
                throw std::runtime_error(
                    "Expected one finite exchange integral"
                    );
            }

            values = value;
        }
        else {
            const auto config = readConfig();

            const int dimensionIndex =
                static_cast<int>(config.dimension);

            if (dimensionIndex < 0 || dimensionIndex > 2) {
                throw std::invalid_argument(
                    "Unknown lattice dimension"
                    );
            }

            std::size_t matrixCount =
                2 * (
                    static_cast<std::size_t>(dimensionIndex) + 1
                    );

            const std::uint64_t latticeSizes[] = {
                config.lx,
                config.ly,
                config.lz
            };

            for (const std::uint64_t size : latticeSizes) {
                if (size == 0 ||
                    size >
                        std::numeric_limits<std::size_t>::max()
                            / matrixCount) {
                    throw std::invalid_argument(
                        "Invalid lattice size"
                        );
                }

                matrixCount *= static_cast<std::size_t>(size);
            }

            std::vector<FM::Matrix> matrices;
            matrices.reserve(matrixCount);

            for (std::size_t index = 0;
                 index < matrixCount;
                 ++index) {
                FM::Matrix matrix(3, 3);

                for (std::size_t element = 0;
                     element < 9;
                     ++element) {
                    double value = 0.0;

                    if (!(input >> value) ||
                        !std::isfinite(value)) {
                        throw std::runtime_error(
                            "Invalid element " +
                            std::to_string(element) +
                            " of matrix " +
                            std::to_string(index)
                            );
                    }

                    matrix[element] = value;
                }

                matrices.push_back(std::move(matrix));
            }

            values = std::move(matrices);

            latticeKey = {
                static_cast<std::uint64_t>(dimensionIndex),
                config.lx,
                config.ly,
                config.lz
            };
        }

        input >> std::ws;

        if (input.bad() || !input.eof()) {
            throw std::runtime_error(
                "Exchange integrals file contains extra data"
                );
        }

        // Применяем данные после проверки всего файла.
        exchangeIntegralFromFile = std::move(values);

        exchangeFileLatticeKey = latticeKey;
        exchangeIntegralFileName = filename;
        exchangeIntegralFileLoaded = true;

        updateExchangeControls();
        QMessageBox::information(
            this,
            QStringLiteral("Exchange integrals loaded"),
            QStringLiteral("Loaded exchange integrals from:\n%1")
                .arg(exchangeIntegralFileName)
            );
    }
    catch (const std::exception& error) {
        QMessageBox::warning(
            this,
            QStringLiteral("Exchange integrals file"),
            QString::fromUtf8(error.what())
            );
    }
}