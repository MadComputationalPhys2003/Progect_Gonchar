#ifndef ENERGY_PLOT_DIALOG_H
#define ENERGY_PLOT_DIALOG_H

#include <QDialog>
#include "UrFaustSimulation_1.h"
#include <QGraphicsScene>
namespace Ui {
class EnergyPlotDialog;
}

class EnergyPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnergyPlotDialog(QWidget *parent = nullptr);
    ~EnergyPlotDialog();
    void setResult(const UrFaustSim::SimulationResult &result);
private:
    Ui::EnergyPlotDialog *ui;
    QGraphicsScene *scene = nullptr;
};

#endif // ENERGY_PLOT_DIALOG_H
