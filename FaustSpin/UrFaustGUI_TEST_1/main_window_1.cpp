#include "main_window_1.h"
#include "ui_main_window_1.h"
#include "parameters_dialog.h"
#include "system_results_dialog.h"
#include "energy_plot_dialog.h"
#include "spin_visualization_dialog.h"
#include <QAction>
#include <QMessageBox>
#include <QPixmap>
MainWindow_1::MainWindow_1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow_1)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("UrFaust, subprogram FaustSpin"));

    const auto showCat = [](QLabel *label,
                            const QString &resourcePath,
                            int width,
                            int height)
    {
        const QPixmap picture(resourcePath);

        label->setAlignment(Qt::AlignCenter);
        label->setScaledContents(false);
        label->setMinimumSize(width, height);

        label->setPixmap(
            picture.scaled(
                width, height,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    };

    showCat(ui->catLabel1, QStringLiteral(":/images/cat_1"), 160, 160);
    showCat(ui->catLabel2, QStringLiteral(":/images/cat_2"), 160, 160);
    showCat(ui->catLabel5, QStringLiteral(":/images/cat_3"), 160, 160);
    showCat(ui->catLabel4, QStringLiteral(":/images/cat_4"), 160, 160);
    showCat(ui->catLabel3, QStringLiteral(":/images/main_cat"), 320, 240);
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

                systemResultsDialog->show();
                systemResultsDialog->raise();
                systemResultsDialog->activateWindow();
            });
    connect(ui->actionSetParameters, &QAction::triggered,
            this, [this]()
            {
                if (!parametersDialog) {
                    parametersDialog = new ParametersDialog(this);
                }

                parametersDialog->show();
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

                energyPlotDialog->show();
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

                spinVisualizationDialog->show();
                spinVisualizationDialog->raise();
                spinVisualizationDialog->activateWindow();
            });
}

MainWindow_1::~MainWindow_1()
{
    delete ui;
}

