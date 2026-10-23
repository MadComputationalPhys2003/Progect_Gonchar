#include "main_window_1.h"
#include "ui_main_window_1.h"
#include "parameters_dialog.h"
#include "system_results_dialog.h"
#include "energy_plot_dialog.h"
#include "spin_visualization_dialog.h"
#include "creator_dialog.h"
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QDialog>
#include <QGridLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWidget>
MainWindow_1::MainWindow_1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow_1)
{
    ui->setupUi(this);
    connect(ui->setParametersButton, &QPushButton::clicked,
            ui->actionSetParameters, &QAction::trigger);
    setWindowTitle(QStringLiteral("UrFaust, subprogram FaustSpin"));
    CreatorDialog *creatorDialog = new CreatorDialog(this);

    QShortcut *creatorShortcut = new QShortcut(
        QKeySequence(QStringLiteral("Ctrl+Shift+I")),
        this
        );

    creatorShortcut->setContext(Qt::ApplicationShortcut);
    creatorShortcut->setAutoRepeat(false);

    connect(
        creatorShortcut,
        &QShortcut::activated,
        creatorDialog,
        [creatorDialog]()
        {
            creatorDialog->showNormal();
            creatorDialog->raise();
            creatorDialog->activateWindow();
        }
        );

    connect(ui->actionSysten_results, &QAction::triggered,
            this, [this]()
            {
                if (!parametersDialog ||
                    parametersDialog->getSimulationResult().frames.empty()) {
                    QMessageBox::information(
                        this,
                        QStringLiteral("No results"),
                        QStringLiteral("Run a simulation first.")
                        );
                    return;
                }

                if (!systemResultsDialog) {
                    systemResultsDialog =
                        new SystemResultsDialog(this);
                }

                systemResultsDialog->setResult(
                    parametersDialog->getSimulationResult()
                    );

                systemResultsDialog->showNormal();
                systemResultsDialog->raise();
                systemResultsDialog->activateWindow();
            });
    connect(ui->actionSetParameters, &QAction::triggered,
            this, [this]()
            {
                if (!parametersDialog) {
                    parametersDialog = new ParametersDialog(this);
                    connect(parametersDialog,
                            &ParametersDialog::systemResultsRequested,
                            ui->actionSysten_results,
                            &QAction::trigger);

                    connect(parametersDialog,
                            &ParametersDialog::energyPlotRequested,
                            ui->actionEnergy_plot,
                            &QAction::trigger);

                    connect(parametersDialog,
                            &ParametersDialog::spinVisualizationRequested,
                            ui->actionSpin_Visualisation,
                            &QAction::trigger);
                    connect(
                        parametersDialog,
                        &ParametersDialog::simulationFinished,
                        this,
                        [this]()
                        {
                            const auto &result =
                                parametersDialog->getSimulationResult();

                            if (systemResultsDialog) {
                                systemResultsDialog->setResult(result);
                            }

                            if (energyPlotDialog) {
                                energyPlotDialog->setResult(result);
                            }

                            if (spinVisualizationDialog) {
                                spinVisualizationDialog->setResult(result);
                            }
                        }
                        );
                }

                parametersDialog->showNormal();
                parametersDialog->raise();
                parametersDialog->activateWindow();
            });
    connect(ui->actionEnergy_plot, &QAction::triggered,
            this, [this]()
            {
        if (!parametersDialog ||
            parametersDialog->getSimulationResult().frames.empty()) {
            QMessageBox::information(
                this,
                QStringLiteral("No results"),
                QStringLiteral("Run a simulation first.")
                );
            return;
        }
                if (!energyPlotDialog) {
                    energyPlotDialog =
                        new EnergyPlotDialog(this);
                }

                energyPlotDialog->showNormal();
                energyPlotDialog->setResult(
                    parametersDialog->getSimulationResult()
                    );
                energyPlotDialog->raise();
                energyPlotDialog->activateWindow();
            });
    connect(ui->actionSpin_Visualisation, &QAction::triggered,
            this, [this]()
            {
                if (!parametersDialog ||
                    parametersDialog->getSimulationResult()
                        .frames.empty()) {
                    QMessageBox::information(
                        this,
                        QStringLiteral("No results"),
                        QStringLiteral("Run a simulation first.")
                        );
                    return;
                }

                if (!spinVisualizationDialog) {
                    spinVisualizationDialog =
                        new SpinVisualizationDialog(this);
                }

                spinVisualizationDialog->setResult(
                    parametersDialog->getSimulationResult()
                    );

                spinVisualizationDialog->showNormal();
                spinVisualizationDialog->raise();
                spinVisualizationDialog->activateWindow();
            });
}

MainWindow_1::~MainWindow_1()
{
    delete ui;
}

