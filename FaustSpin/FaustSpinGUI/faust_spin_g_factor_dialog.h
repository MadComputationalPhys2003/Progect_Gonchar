#pragma once

#include "FaustSpinSimulation.h"

#include <QDialog>

#include <cstddef>
#include <memory>
#include <optional>

namespace Ui { class FaustSpinGFactorDialog; }

class FaustSpinGFactorDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinGFactorDialog(QWidget *parent = nullptr);
    explicit FaustSpinGFactorDialog(const FaustSpinSim::SimulationConfig &config,
                                    QWidget *parent = nullptr);
    ~FaustSpinGFactorDialog() override;

    void accept() override;
    void applyTo(FaustSpinSim::SimulationConfig &config) const;

private:
    void configureValue();
    void updateControls();

    std::unique_ptr<Ui::FaustSpinGFactorDialog> ui;
    std::size_t ionCount_;
    FaustSpinSim::Interaction initial_;
    std::optional<FaustSpinSim::Interaction> selected_;
};
