#pragma once

#include "FaustSpinSimulation.h"

#include <QDialog>

#include <memory>
#include <optional>

namespace Ui { class FaustSpinSingleIonAnisotropyDialog; }

class FaustSpinSingleIonAnisotropyDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinSingleIonAnisotropyDialog(FaustSpinSim::SimulationConfig &config,
                                                QWidget *parent = nullptr);
    ~FaustSpinSingleIonAnisotropyDialog() override;

    void accept() override;

private:
    void configureAnisotropy();
    void updateControls();

    std::unique_ptr<Ui::FaustSpinSingleIonAnisotropyDialog> ui;
    FaustSpinSim::SimulationConfig &config_;
    std::optional<FaustSpinSim::Interaction> selected_;
};