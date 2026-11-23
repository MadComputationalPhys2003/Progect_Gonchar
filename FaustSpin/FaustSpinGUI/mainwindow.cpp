#include "mainwindow.h"
#include "main_window_1.h"
#include "ui_mainwindow.h"
#include "faust_spin_parameters_dialog.h"
#include <QMessageBox>
#include <QPushButton>
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

    connect(ui->FaustPushButtom, &QPushButton::clicked, this, [this] {
        FaustSpinParametersDialog dialog(faustSpinConfig, this);
        if (dialog.exec() == QDialog::Accepted) {
            dialog.applyTo(faustSpinConfig);
            startFaustSpin();
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
    if (faustSpinWorker_) {
        faustSpinWorker_->wait();
    }
    delete ui;
}

void MainWindow::startFaustSpin()
{
    struct Outcome {
        std::optional<FaustSpinSim::SimulationResult> result;
        QString error;
    };

    const auto outcome = std::make_shared<Outcome>();
    auto *worker = QThread::create([config = faustSpinConfig, outcome]() mutable {
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

    auto *button = ui->FaustPushButtom;
    const QString originalText = button->text();
    button->setEnabled(false);
    button->setText(tr("Calculating..."));

    connect(worker, &QThread::finished, this,
            [this, worker, outcome, originalText] {
                faustSpinWorker_ = nullptr;
                worker->deleteLater();
                ui->FaustPushButtom->setText(originalText);
                ui->FaustPushButtom->setEnabled(true);

                if (!outcome->result) {
                    QMessageBox::critical(this, tr("FaustSpin error"), outcome->error);
                    return;
                }

                faustSpinResult_ = std::move(outcome->result);
                const auto &result = *faustSpinResult_;

                QMessageBox::information(
                    this, tr("FaustSpin result"),
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
