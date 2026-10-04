#include "spin_visualization_dialog.h"
#include "ui_spin_visualization_dialog.h"
#include "spin_opengl_widget.h"
#include <QSignalBlocker>
#include <QSpinBox>
#include <QString>
#include <cstddef>

SpinVisualizationDialog::SpinVisualizationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SpinVisualizationDialog)
{
    ui->setupUi(this);

    ui->frameSpinBox->setRange(0, 0);
    ui->frameSpinBox->setEnabled(false);

    ui->frameInfoLabel->setText(QStringLiteral("No results"));

    connect(ui->frameSpinBox, &QSpinBox::valueChanged,
            this, &SpinVisualizationDialog::updateFrame);
}

SpinVisualizationDialog::~SpinVisualizationDialog()
{
    delete ui;
}

void SpinVisualizationDialog::setResult(
    const UrFaustSim::SimulationResult &result)
{
    simulationResult = &result;

    const QSignalBlocker blocker(ui->frameSpinBox);

    if (result.frames.empty()) {
        ui->frameSpinBox->setRange(0, 0);
        ui->frameSpinBox->setValue(0);
        ui->frameSpinBox->setEnabled(false);

        ui->frameInfoLabel->setText(QStringLiteral("No results"));
        return;
    }

    const int lastFrame =
        static_cast<int>(result.frames.size() - 1);

    ui->frameSpinBox->setRange(0, lastFrame);
    ui->frameSpinBox->setValue(lastFrame);
    ui->frameSpinBox->setEnabled(true);

    updateFrame(lastFrame);
}

void SpinVisualizationDialog::updateFrame(int frameIndex)
{
    if (!simulationResult ||
        frameIndex < 0 ||
        static_cast<std::size_t>(frameIndex) >=
            simulationResult->frames.size()) {
        ui->frameInfoLabel->setText(QStringLiteral("No frame"));
        return;
    }

    const auto &frame =
        simulationResult->frames[
            static_cast<std::size_t>(frameIndex)];
    ui->spinOpenGLWidget->setFrame(
        simulationResult->config,
        frame
        );
    ui->frameInfoLabel->setText(
        QStringLiteral("Iteration: %1 | Energy: %2 | Spins: %3")
            .arg(static_cast<qulonglong>(frame.sweep))
            .arg(QString::number(frame.energy, 'g', 12))
            .arg(static_cast<qulonglong>(
                frame.spinDirections.size()))
        );
}