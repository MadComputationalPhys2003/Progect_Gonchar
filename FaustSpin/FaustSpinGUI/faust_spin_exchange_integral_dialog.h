#pragma once

#include "FaustSpinSimulation.h"

#include <QDialog>
#include <optional>

namespace Ui { class FaustSpinExchangeIntegralDialog; }

class FaustSpinExchangeIntegralDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinExchangeIntegralDialog(FaustSpinSim::SimulationConfig &config,
                                             QWidget *parent = nullptr);
    ~FaustSpinExchangeIntegralDialog() override;
    void accept() override;

private:
    void updateControls();
    void configureExchange();

    Ui::FaustSpinExchangeIntegralDialog *ui;
    FaustSpinSim::SimulationConfig &config_;
    std::optional<FaustSpinSim::Interaction> selected_;
    double randomMinimum_ = -1.0;
    double randomMaximum_ = 1.0;
};
