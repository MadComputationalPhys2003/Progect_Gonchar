#pragma once

#include <QDialog>
#include <array>
#include "FaustSpinSimulation.h"
QT_BEGIN_NAMESPACE
namespace Ui { class FaustSpinParametersDialog; }
class QSpinBox;
QT_END_NAMESPACE

namespace FaustSpinSim { struct SimulationConfig; }

class FaustSpinParametersDialog : public QDialog
{
public:
    explicit FaustSpinParametersDialog(
        const FaustSpinSim::SimulationConfig &config,
        QWidget *parent = nullptr);
    ~FaustSpinParametersDialog() override;
    void accept();
    void applyTo(FaustSpinSim::SimulationConfig &config) const;

private:
    void updateDimensionControls(int index);
    void readLattice(FaustSpinSim::SimulationConfig &config) const;
    bool exchangeFitsLattice(const FaustSpinSim::SimulationConfig &config) const;

    // Это поле поставь после указателя ui.
    FaustSpinSim::SimulationConfig exchangeConfig_;
    Ui::FaustSpinParametersDialog *ui;
    std::array<QSpinBox *, 3> sizeEditors{};
};