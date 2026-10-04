#include "system_results_dialog.h"
#include "ui_system_results_dialog.h"
#include <cmath>
#include <QString>
SystemResultsDialog::SystemResultsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SystemResultsDialog)
{
    ui->setupUi(this);
    ui->finalSpinDirectionValueLabel->setTextInteractionFlags(
        Qt::TextSelectableByMouse |
        Qt::TextSelectableByKeyboard
        );
}

SystemResultsDialog::~SystemResultsDialog()
{
    delete ui;
}
void SystemResultsDialog::setResult(
    const UrFaustSim::SimulationResult &result)
{
    ui->finalSpinDirectionValueLabel->setText(QStringLiteral("-"));
    if (result.frames.empty()) {
        ui->initialEnergyValueLabel->setText(QStringLiteral("-"));
        ui->finalEnergyValueLabel->setText(QStringLiteral("-"));
        ui->completedIterationsValueLabel->setText(QStringLiteral("-"));
        ui->savedFramesValueLabel->setText(QStringLiteral("-"));
        ui->convergenceValueLabel->setText(QStringLiteral("-"));
        return;
    }

    ui->initialEnergyValueLabel->setText(
        QString::number(result.frames.front().energy, 'g', 12)
        );

    ui->finalEnergyValueLabel->setText(
        QString::number(result.frames.back().energy, 'g', 12)
        );

    ui->completedIterationsValueLabel->setText(
        QString::number(
            static_cast<qulonglong>(result.completedSweeps)
            )
        );

    ui->savedFramesValueLabel->setText(
        QString::number(
            static_cast<qulonglong>(result.frames.size())
            )
        );

    ui->convergenceValueLabel->setText(
        result.converged
            ? QStringLiteral("Yes")
            : QStringLiteral("No")
        );
    const auto &spins = result.frames.back().spinDirections;

    if (spins.empty()) {
        return;
    }

    double meanX = 0.0;
    double meanY = 0.0;
    double meanZ = 0.0;

    for (const auto &spin : spins) {
        meanX += spin[0];
        meanY += spin[1];
        meanZ += spin[2];
    }

    const double count = static_cast<double>(spins.size());

    meanX /= count;
    meanY /= count;
    meanZ /= count;

    const double meanLength =
        std::hypot(meanX, meanY, meanZ);

    if (!std::isfinite(meanLength) || meanLength <= 1.0e-10) {
        ui->finalSpinDirectionValueLabel->setText(
            QStringLiteral("Undefined (mean spin near zero)")
            );
        return;
    }

    ui->finalSpinDirectionValueLabel->setText(
        QStringLiteral("(%1, %2, %3)")
            .arg(meanX / meanLength, 0, 'f', 6)
            .arg(meanY / meanLength, 0, 'f', 6)
            .arg(meanZ / meanLength, 0, 'f', 6)
        );
}