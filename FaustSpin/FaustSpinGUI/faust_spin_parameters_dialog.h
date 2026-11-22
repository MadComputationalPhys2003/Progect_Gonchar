#pragma once

#include <QDialog>
#include <array>

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

    void applyTo(FaustSpinSim::SimulationConfig &config) const;

private:
    void updateDimensionControls(int index);

    Ui::FaustSpinParametersDialog *ui;
    std::array<QSpinBox *, 3> sizeEditors{};
};