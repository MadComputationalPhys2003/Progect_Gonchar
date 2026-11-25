#ifndef FAUST_SPIN_VISUALIZATION_H
#define FAUST_SPIN_VISUALIZATION_H

#include <QDialog>

namespace FaustSpinSim {
struct SimulationResult;
}

namespace Ui {
class FaustSpinVisualization;
}

class FaustSpinVisualization : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinVisualization(QWidget *parent = nullptr);
    ~FaustSpinVisualization() override;

    // The result must remain alive and stable while this dialog is open.
    void setResult(const FaustSpinSim::SimulationResult &result);

private:
    void updateFrame(int frameIndex);
    void openSaveFrameDialog();

    Ui::FaustSpinVisualization *ui;
    const FaustSpinSim::SimulationResult *result_ = nullptr;
};

#endif // FAUST_SPIN_VISUALIZATION_H
