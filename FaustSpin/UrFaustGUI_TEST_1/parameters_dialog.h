#ifndef PARAMETERS_DIALOG_H
#define PARAMETERS_DIALOG_H
#include "UrFaustSimulation_1.h"
#include <QDialog>
#include <QString>
#include <array>
#include <cstdint>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <optional>
#include <vector>
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
    void saveTxtResults();
    void saveTexResult();
    void updateInitialDirectionControls();
    void configureInitialDirections();

    Vector3D_Decart::Vector3D_Dec parallelDirection{
        1.0, 0.0, 0.0
    };
    std::optional<int> randomDirectionSeed;
    QString lastExportDirectory;
    QString initialDirectionFileName;
    std::vector<Vector3D_Decart::Vector3D_Dec> manualDirections;
    double constantSpinLength{1.0};
std::vector<double> manualSpinLengths;
    void configureSpinLength();
double randomSpinLengthMinimum{0.95};
double randomSpinLengthMaximum{1.05};

void updateSpinLengthControls();
std::variant<double, std::vector<double>> spinLengthsFromFile{1.0};

bool spinLengthFileLoaded{false};

QString spinLengthFileName;
std::vector<FM::Matrix> manualExchangeMatrices;

std::array<std::uint64_t, 4> manualExchangeLatticeKey{};
void chooseSpinLengthFile();
double constantExchangeIntegral{-1.0};

void configureExchangeIntegral();
void updateExchangeControls();
double randomExchangeMinimum{-1.0};
double randomExchangeMaximum{1.0};
std::variant<double, std::vector<FM::Matrix>>
    exchangeIntegralFromFile{-1.0};

bool exchangeIntegralFileLoaded{false};

QString exchangeIntegralFileName;

std::array<std::uint64_t, 4> exchangeFileLatticeKey{};

void chooseExchangeIntegralFile();
};

#endif // PARAMETERS_DIALOG_H
