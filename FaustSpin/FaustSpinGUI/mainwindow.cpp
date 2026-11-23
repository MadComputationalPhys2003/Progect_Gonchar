#include "mainwindow.h"
#include "main_window_1.h"
#include "ui_mainwindow.h"
#include "faust_spin_parameters_dialog.h"
#include "Write_To_File.h"
#include "energy_plot_dialog.h"
#include <QPointer>
#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QThread>

#include <exception>
#include <memory>
#include <utility>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    ui->FaustPushButtom->setEnabled(true);

    auto *fileMenu = menuBar()->addMenu(tr("&File"));
    saveFaustSpinTxtAction_ = fileMenu->addAction(tr("Save TXT..."));
    saveFaustSpinTxtAction_->setEnabled(false);
    connect(saveFaustSpinTxtAction_, &QAction::triggered,
            this, &MainWindow::saveFaustSpinTxt);


    connect(ui->FaustPushButtom, &QPushButton::clicked, this, [this] {
        FaustSpinParametersDialog dialog(faustSpinConfig, this);

        auto *calculate = dialog.findChild<QPushButton *>("CalculatepushButton");
        auto *energyPlot = dialog.findChild<QPushButton *>("pushButtonEnergyplot");
        calculate->setAutoDefault(false);
        energyPlot->setAutoDefault(false);
        energyPlot->setEnabled(faustSpinResult_.has_value());

        connect(calculate, &QPushButton::clicked, &dialog, [this, &dialog] {
            if (dialog.validateInputs()) {
                startFaustSpin(dialog);
            }
        });

        connect(energyPlot, &QPushButton::clicked, &dialog, [this, &dialog] {
            UrFaustSim::SimulationResult plotData;
            plotData.frames.reserve(faustSpinResult_->frames.size());
            for (const auto &frame : faustSpinResult_->frames) {
                plotData.frames.push_back({frame.sweep, frame.energy, {}});
            }

            EnergyPlotDialog plot(&dialog);
            plot.setWindowTitle(tr("FaustSpin Energy Plot (meV)"));
            plot.setResult(plotData);
            plot.exec();
        });

        if (dialog.exec() == QDialog::Accepted) {
            dialog.applyTo(faustSpinConfig);
        }
    });
}

MainWindow::~MainWindow()
{
    if (faustSpinWorker_) {
        faustSpinWorker_->wait();
    }
    delete ui;
}

void MainWindow::startFaustSpin(FaustSpinParametersDialog &dialog)
{
    if (faustSpinWorker_) {
        return;
    }

    auto config = faustSpinConfig;
    dialog.applyTo(config);

    struct Outcome {
        std::optional<FaustSpinSim::SimulationResult> result;
        QString error;
    };

    const auto outcome = std::make_shared<Outcome>();
    auto *worker = QThread::create([config = std::move(config), outcome]() mutable {
        try {
            FaustSpinSim::Simulation simulation(std::move(config));
            outcome->result.emplace(simulation.run());
        } catch (const std::exception &error) {
            outcome->error = QString::fromUtf8(error.what());
        } catch (...) {
            outcome->error = QStringLiteral("Unexpected simulation error.");
        }
    });

    worker->setParent(this);
    faustSpinWorker_ = worker;

    QPointer<QPushButton> calculate(dialog.findChild<QPushButton *>("CalculatepushButton"));
    QPointer<QPushButton> energyPlot(dialog.findChild<QPushButton *>("pushButtonEnergyplot"));
    calculate->setText(tr("Calculating..."));
    calculate->setEnabled(false);
    energyPlot->setEnabled(false);
    ui->FaustPushButtom->setEnabled(false);

    connect(worker, &QThread::finished, this,
            [this, worker, outcome, calculate, energyPlot, parent = QPointer<QWidget>(&dialog)] {
                faustSpinWorker_ = nullptr;
                worker->deleteLater();
                ui->FaustPushButtom->setEnabled(true);

                const bool succeeded = outcome->result.has_value();
                if (succeeded) {
                    faustSpinResult_ = std::move(outcome->result);
                }
                if (calculate) {
                    calculate->setText(tr("Calculate"));
                    calculate->setEnabled(true);
                }
                if (energyPlot) {
                    energyPlot->setEnabled(faustSpinResult_.has_value());
                }

                QWidget *messageParent = parent ? parent.data() : this;
                if (!succeeded) {
                    QMessageBox::critical(messageParent, tr("FaustSpin error"), outcome->error);
                    return;
                }

                const auto &result = *faustSpinResult_;
                QMessageBox::information(
                    messageParent, tr("FaustSpin result"),
                    tr("Spins: %1\nInitial energy: %2 meV\nFinal energy: %3 meV"
                       "\nSweeps: %4\nConverged: %5")
                        .arg(static_cast<qulonglong>(result.spinLengths.size()))
                        .arg(result.frames.front().energy, 0, 'g', 12)
                        .arg(result.frames.back().energy, 0, 'g', 12)
                        .arg(static_cast<qulonglong>(result.completedSweeps))
                        .arg(result.converged ? tr("Yes") : tr("No")));
            }, Qt::QueuedConnection);

    worker->start();
}

void MainWindow::saveFaustSpinTxt()
{
    if (!faustSpinResult_ || faustSpinWorker_) {
        return;
    }

    const QString parentDirectory = QFileDialog::getExistingDirectory(
        this, tr("Choose folder for FaustSpin results"));
    if (parentDirectory.isEmpty()) {
        return;
    }

    QTemporaryDir directory(
        QDir(parentDirectory).filePath(QStringLiteral("FaustSpin_XXXXXX")));
    if (!directory.isValid()) {
        QMessageBox::critical(this, tr("TXT export error"), directory.errorString());
        return;
    }

    try {
        WriteFileFaustSpinSimulation::write_all_txt(
            *faustSpinResult_, QFileInfo(directory.path()).filesystemFilePath());
    } catch (const std::exception &error) {
        QMessageBox::critical(this, tr("TXT export error"), QString::fromUtf8(error.what()));
        return;
    }

    directory.setAutoRemove(false);
    QMessageBox::information(this, tr("Saved"),
                             tr("Files saved to:\n%1").arg(directory.path()));
}
