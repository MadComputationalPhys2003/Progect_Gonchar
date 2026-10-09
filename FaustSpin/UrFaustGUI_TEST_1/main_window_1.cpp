#include "main_window_1.h"
#include "ui_main_window_1.h"
#include "parameters_dialog.h"
#include "system_results_dialog.h"
#include "energy_plot_dialog.h"
#include "spin_visualization_dialog.h"
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
    QDialog *catsDialog = new QDialog(this);
    catsDialog->setWindowTitle(QStringLiteral("Cats"));
    catsDialog->resize(560, 600);

    QVBoxLayout *catsLayout = new QVBoxLayout(catsDialog);

    QScrollArea *catsScroll = new QScrollArea(catsDialog);
    catsScroll->setWidgetResizable(true);
    catsLayout->addWidget(catsScroll);

    QWidget *catsContent = new QWidget;
    QGridLayout *catsGrid = new QGridLayout(catsContent);
    catsGrid->setSpacing(12);

    const auto addCat =
        [catsContent, catsGrid](
            const QString &resourcePath,
            int row,
            int column,
            int width,
            int height,
            int columnSpan = 1)
    {
        QLabel *label = new QLabel(catsContent);
        label->setAlignment(Qt::AlignCenter);
        label->setScaledContents(false);
        label->setMinimumSize(width, height);

        const QPixmap picture(resourcePath);

        label->setPixmap(
            picture.scaled(
                width,
                height,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));

        catsGrid->addWidget(
            label,
            row,
            column,
            1,
            columnSpan);
    };

    addCat(QStringLiteral(":/images/cat_1"), 0, 0, 200, 200);
    addCat(QStringLiteral(":/images/cat_2"), 0, 1, 200, 200);
    addCat(QStringLiteral(":/images/cat_3"), 1, 0, 200, 200);
    addCat(QStringLiteral(":/images/cat_4"), 1, 1, 200, 200);

    addCat(
        QStringLiteral(":/images/main_cat"),
        2, 0, 360, 240, 2);

    catsScroll->setWidget(catsContent);

    QShortcut *catsShortcut = new QShortcut(
        QKeySequence(QStringLiteral("Ctrl+Shift+I")),
        this);

    catsShortcut->setContext(Qt::ApplicationShortcut);
    catsShortcut->setAutoRepeat(false);

    connect(
        catsShortcut,
        &QShortcut::activated,
        catsDialog,
        [catsDialog]()
        {
            catsDialog->showNormal();
            catsDialog->raise();
            catsDialog->activateWindow();
        });

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

