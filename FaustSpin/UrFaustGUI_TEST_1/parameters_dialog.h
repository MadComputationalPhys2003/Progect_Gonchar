#ifndef PARAMETERS_DIALOG_H
#define PARAMETERS_DIALOG_H
#include "UrFaustSimulation_1.h"
#include <QDialog>

namespace Ui {
class ParametersDialog;
}

class ParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog(QWidget *parent = nullptr);
    ~ParametersDialog();
    const UrFaustSim::SimulationResult &getSimulationResult() const;
signals:
    void simulationFinished();
    void systemResultsRequested();
    void energyPlotRequested();
    void spinVisualizationRequested();
private:
    Ui::ParametersDialog *ui;
    UrFaustSim::SimulationConfig readConfig() const;
    void startSimulation();

    UrFaustSim::SimulationResult simulationResult;
};

#endif // PARAMETERS_DIALOG_H
