#pragma once

#include "FaustSpinSimulation.h"

#include <QDialog>
#include <array>

QT_BEGIN_NAMESPACE
namespace Ui { class FaustSpinParametersDialog; }
class QSpinBox;
QT_END_NAMESPACE

class FaustSpinParametersDialog : public QDialog
{
public:
    explicit FaustSpinParametersDialog(
        const FaustSpinSim::SimulationConfig &config,
        QWidget *parent = nullptr);
    ~FaustSpinParametersDialog() override;

    void accept() override;
    void applyTo(FaustSpinSim::SimulationConfig &config) const;

private:
    void updateDimensionControls(int index);
    void readLattice(FaustSpinSim::SimulationConfig &config) const;
    bool exchangeFitsLattice(const FaustSpinSim::SimulationConfig &config) const;
    bool anisotropyFitsLattice(const FaustSpinSim::SimulationConfig &config) const;

    Ui::FaustSpinParametersDialog *ui;
    FaustSpinSim::SimulationConfig exchangeConfig_;
    FaustSpinSim::SimulationConfig anisotropyConfig_;
    std::array<QSpinBox *, 3> sizeEditors{};
};