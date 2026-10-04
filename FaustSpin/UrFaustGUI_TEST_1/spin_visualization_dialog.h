#ifndef SPIN_VISUALIZATION_DIALOG_H
#define SPIN_VISUALIZATION_DIALOG_H

#include <QDialog>
#include "UrFaustSimulation_1.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class SpinVisualizationDialog;
}
QT_END_NAMESPACE

class SpinVisualizationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpinVisualizationDialog(QWidget *parent = nullptr);
    ~SpinVisualizationDialog() override;

    void setResult(const UrFaustSim::SimulationResult &result);

private:
    Ui::SpinVisualizationDialog *ui;

    const UrFaustSim::SimulationResult *simulationResult = nullptr;

    void updateFrame(int frameIndex);
};

#endif // SPIN_VISUALIZATION_DIALOG_H