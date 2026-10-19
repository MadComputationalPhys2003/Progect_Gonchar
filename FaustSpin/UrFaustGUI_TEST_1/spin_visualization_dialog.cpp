#include "spin_visualization_dialog.h"
#include "ui_spin_visualization_dialog.h"
#include "spin_opengl_widget.h"
#include <QSignalBlocker>
#include <QSpinBox>
#include <QString>
#include <cstddef>
#include <QPushButton>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QMessageBox>
#include <QSaveFile>
SpinVisualizationDialog::SpinVisualizationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SpinVisualizationDialog)
{
    ui->setupUi(this);
    ui->frameSlider->setRange(0, 0);
    ui->frameSlider->setEnabled(false);
    ui->frameSlider->setSingleStep(1);
    ui->frameSlider->setPageStep(10);
    ui->frameSlider->setTracking(true);

    connect(
        ui->frameSlider,
        &QSlider::valueChanged,
        ui->frameSpinBox,
        &QSpinBox::setValue
        );

    connect(
        ui->frameSpinBox,
        &QSpinBox::valueChanged,
        ui->frameSlider,
        &QSlider::setValue
        );
    connect(ui->viewXButton, &QPushButton::clicked,
            ui->spinOpenGLWidget,
            &SpinOpenGLWidget::setViewAlongX);

    connect(ui->viewYButton, &QPushButton::clicked,
            ui->spinOpenGLWidget,
            &SpinOpenGLWidget::setViewAlongY);

    connect(ui->viewZButton, &QPushButton::clicked,
            ui->spinOpenGLWidget,
            &SpinOpenGLWidget::setViewAlongZ);

    connect(ui->resetViewButton, &QPushButton::clicked,
            ui->spinOpenGLWidget,
            &SpinOpenGLWidget::resetView);
    setWindowFlags(
        (windowFlags() & ~Qt::WindowContextHelpButtonHint)
        | Qt::CustomizeWindowHint
        | Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowMaximizeButtonHint
        | Qt::WindowCloseButtonHint
        );
    ui->frameSpinBox->setRange(0, 0);
    ui->frameSpinBox->setEnabled(false);

    ui->frameInfoLabel->setText(QStringLiteral("No results"));

    connect(ui->frameSpinBox, &QSpinBox::valueChanged,
            this, &SpinVisualizationDialog::updateFrame);
    connect(
        ui->saveSvgButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!simulationResult ||
                simulationResult->frames.empty()) {
                QMessageBox::information(
                    this,
                    QStringLiteral("Save frame"),
                    QStringLiteral("Run a simulation first.")
                    );
                return;
            }

            const auto showError =
                [this](const QString &message)
            {
                QMessageBox::critical(
                    this,
                    QStringLiteral("Save frame"),
                    message
                    );
            };

            const int formatIndex =
                ui->saveFramtocomboBox->currentIndex();

            QString suffix;
            QString filter;

            switch (formatIndex) {
            case 0:
                suffix = QStringLiteral("svg");
                filter = QStringLiteral("SVG image (*.svg)");
                break;

            case 1:
                suffix = QStringLiteral("png");
                filter = QStringLiteral("PNG image (*.png)");
                break;

            case 2:
                suffix = QStringLiteral("jpg");
                filter = QStringLiteral("JPEG image (*.jpg *.jpeg)");
                break;

            default:
                return;
            }

            QFileDialog dialog(
                this,
                QStringLiteral("Save current frame")
                );

            dialog.setAcceptMode(QFileDialog::AcceptSave);
            dialog.setNameFilter(filter);
            dialog.setDefaultSuffix(suffix);

            dialog.selectFile(
                QStringLiteral("spin_frame_%1.%2")
                    .arg(ui->frameSpinBox->value())
                    .arg(suffix)
                );

            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            const QString fileName =
                dialog.selectedFiles().first();

            const QString extension =
                QFileInfo(fileName).suffix().toLower();

            const bool correctExtension =
                extension == suffix ||
                (formatIndex == 2 &&
                 extension == QStringLiteral("jpeg"));

            if (!correctExtension) {
                showError(
                    QStringLiteral(
                        "Use the .%1 extension for the selected format."
                        ).arg(suffix)
                    );
                return;
            }

            // Векторный экспорт.
            if (formatIndex == 0) {
                QString errorMessage;

                if (!ui->spinOpenGLWidget->saveSvg(
                        fileName,
                        errorMessage)) {
                    showError(errorMessage);
                }

                return;
            }

            // Снимок текущего изображения OpenGL.
            const QImage image =
                ui->spinOpenGLWidget->grabFramebuffer();

            if (image.isNull()) {
                showError(
                    QStringLiteral("Cannot capture the current view.")
                    );
                return;
            }

            QSaveFile output(fileName);

            if (!output.open(QIODevice::WriteOnly)) {
                showError(output.errorString());
                return;
            }

            const QByteArray imageFormat =
                formatIndex == 1
                    ? QByteArray("png")
                    : QByteArray("jpeg");

            QImageWriter writer(&output, imageFormat);

            if (formatIndex == 2) {
                writer.setQuality(95);
            }

            if (!writer.write(image)) {
                showError(writer.errorString());
                return;
            }

            if (!output.commit()) {
                showError(output.errorString());
            }
        }
        );
}

SpinVisualizationDialog::~SpinVisualizationDialog()
{
    delete ui;
}

void SpinVisualizationDialog::setResult(
    const UrFaustSim::SimulationResult &result)
{
    simulationResult = &result;

    const QSignalBlocker spinBoxBlocker(ui->frameSpinBox);
    const QSignalBlocker sliderBlocker(ui->frameSlider);

    if (result.frames.empty()) {
        ui->frameSpinBox->setRange(0, 0);
        ui->frameSpinBox->setValue(0);
        ui->frameSpinBox->setEnabled(false);

        ui->frameSlider->setRange(0, 0);
        ui->frameSlider->setValue(0);
        ui->frameSlider->setEnabled(false);

        ui->frameInfoLabel->setText(
            QStringLiteral("No results")
            );
        return;
    }

    const int lastFrame =
        static_cast<int>(result.frames.size() - 1);

    ui->frameSpinBox->setRange(0, lastFrame);
    ui->frameSpinBox->setValue(lastFrame);
    ui->frameSpinBox->setEnabled(true);

    ui->frameSlider->setRange(0, lastFrame);
    ui->frameSlider->setValue(lastFrame);
    ui->frameSlider->setEnabled(lastFrame > 0);

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