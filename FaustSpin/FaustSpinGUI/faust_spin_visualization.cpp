#include "faust_spin_visualization.h"
#include "ui_faust_spin_visualization.h"
#include <QSizePolicy>
#include "FaustSpinSimulation.h"
#include "save_frame_dialog.h"
#include "spin_opengl_widget.h"
#include <QCheckBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>

#include <algorithm>
#include <cstddef>
#include <limits>

FaustSpinVisualization::FaustSpinVisualization(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaustSpinVisualization)
{
    ui->setupUi(this);
    setWindowTitle(tr("FaustSpin Visualization"));

    ui->frameSlider->setRange(0, 0);
    ui->frameSpinBox->setRange(0, 0);
    ui->frameSlider->setEnabled(false);
    ui->frameSpinBox->setEnabled(false);
    ui->saveSvgButton->setEnabled(false);
    ui->checkBoxInduction->setEnabled(false);
    ui->horizontalSliderdensity->setRange(1, 5);
    ui->horizontalSliderdensity->setValue(5);
    ui->horizontalSliderdensity->setEnabled(false);

    connect(ui->checkBoxInduction, &QCheckBox::toggled, this, [this](bool show) {
        ui->horizontalSliderdensity->setEnabled(show);
        updateFrame(ui->frameSpinBox->value());
    });
    connect(ui->horizontalSliderdensity, &QSlider::valueChanged, this, [this] {
        updateFrame(ui->frameSpinBox->value());
    });

    connect(ui->frameSlider, &QSlider::valueChanged,
            ui->frameSpinBox, &QSpinBox::setValue);
    connect(ui->frameSpinBox, &QSpinBox::valueChanged,
            ui->frameSlider, &QSlider::setValue);
    connect(ui->frameSpinBox, &QSpinBox::valueChanged,
            this, &FaustSpinVisualization::updateFrame);

    connect(ui->viewXButton, &QPushButton::clicked,
            ui->spinOpenGLWidget, &SpinOpenGLWidget::setViewAlongX);
    connect(ui->viewYButton, &QPushButton::clicked,
            ui->spinOpenGLWidget, &SpinOpenGLWidget::setViewAlongY);
    connect(ui->viewZButton, &QPushButton::clicked,
            ui->spinOpenGLWidget, &SpinOpenGLWidget::setViewAlongZ);
    connect(ui->resetViewButton, &QPushButton::clicked,
            ui->spinOpenGLWidget, &SpinOpenGLWidget::resetView);
    connect(ui->saveSvgButton, &QPushButton::clicked,
            this, &FaustSpinVisualization::openSaveFrameDialog);
}

FaustSpinVisualization::~FaustSpinVisualization()
{
    delete ui;
}

void FaustSpinVisualization::setResult(const FaustSpinSim::SimulationResult &result)
{
    result_ = &result;
    const QSignalBlocker sliderBlocker(ui->frameSlider);
    const QSignalBlocker spinBoxBlocker(ui->frameSpinBox);

    const bool hasFrames = !result.frames.empty();
    const auto maxIndex = static_cast<std::size_t>(std::numeric_limits<int>::max());
    const int lastFrame = hasFrames
                              ? static_cast<int>(std::min(result.frames.size() - 1, maxIndex))
                              : 0;

    ui->frameSlider->setRange(0, lastFrame);
    ui->frameSlider->setValue(lastFrame);
    ui->frameSlider->setEnabled(lastFrame > 0);

    ui->frameSpinBox->setRange(0, lastFrame);
    ui->frameSpinBox->setValue(lastFrame);
    ui->frameSpinBox->setEnabled(lastFrame > 0);

    ui->saveSvgButton->setEnabled(hasFrames);
    const auto &field = result.config.magneticField;
    const bool hasField = hasFrames && result.config.zeemanEnabled &&
                          (field[0] != 0.0 || field[1] != 0.0 || field[2] != 0.0);

    const QSignalBlocker fieldToggleBlocker(ui->checkBoxInduction);
    ui->checkBoxInduction->setEnabled(hasField);
    if (!hasField) {
        ui->checkBoxInduction->setChecked(false);
    }
    ui->horizontalSliderdensity->setEnabled(
        hasField && ui->checkBoxInduction->isChecked());
    if (hasFrames) {
        updateFrame(lastFrame);
    } else {
        ui->spinOpenGLWidget->setFrame({}, {});
        ui->frameInfoLabel->setText(tr("No results"));
    }
}

void FaustSpinVisualization::updateFrame(int frameIndex)
{
    if (!result_ || frameIndex < 0 ||
        static_cast<std::size_t>(frameIndex) >= result_->frames.size()) {
        return;
    }

    const auto &config = result_->config;
    const auto &frame = result_->frames[static_cast<std::size_t>(frameIndex)];

    UrFaustSim::SimulationConfig viewConfig;
    viewConfig.lx = config.lx;
    viewConfig.ly = config.ly;
    viewConfig.lz = config.lz;

    switch (config.dimension) {
    case FaustSpin::Dimension::one:
        viewConfig.dimension = UrFaust::Dimension::one;
        break;
    case FaustSpin::Dimension::two:
        viewConfig.dimension = UrFaust::Dimension::two;
        break;
    case FaustSpin::Dimension::three:
        viewConfig.dimension = UrFaust::Dimension::three;
        break;
    }

    QVector3D fieldDirection;
    if (config.zeemanEnabled && ui->checkBoxInduction->isChecked()) {
        const auto &field = config.magneticField;
        const double scale = std::max({
            std::abs(field[0]), std::abs(field[1]), std::abs(field[2])});
        if (scale > 0.0) {
            fieldDirection = QVector3D(
                static_cast<float>(field[0] / scale),
                static_cast<float>(field[1] / scale),
                static_cast<float>(field[2] / scale));
        }
    }

    const auto fieldStride = static_cast<std::size_t>(
        ui->horizontalSliderdensity->maximum() -
        ui->horizontalSliderdensity->value() + 1);

    ui->spinOpenGLWidget->setFrame(
        viewConfig, {frame.sweep, frame.energy, frame.spinDirections},
        fieldDirection, fieldStride);

    ui->frameInfoLabel->setText(tr("Iteration: %1 | Energy: %2 meV | Spins: %3")
                                    .arg(static_cast<qulonglong>(frame.sweep))
                                    .arg(frame.energy, 0, 'g', 12)
                                    .arg(static_cast<qulonglong>(frame.spinDirections.size())));
}

void FaustSpinVisualization::openSaveFrameDialog()
{
    if (!result_ || result_->frames.empty()) {
        return;
    }

    SaveFrameDialog dialog(this);
    dialog.setFrameSource(ui->spinOpenGLWidget, ui->frameSpinBox->value());
    dialog.exec();
}