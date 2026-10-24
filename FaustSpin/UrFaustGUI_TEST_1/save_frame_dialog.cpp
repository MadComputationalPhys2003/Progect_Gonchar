#include "save_frame_dialog.h"
#include "ui_save_frame_dialog.h"
#include "spin_opengl_widget.h"

#include <QByteArray>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>

SaveFrameDialog::SaveFrameDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SaveFrameDialog)
{
    ui->setupUi(this);

    setWindowFlags(
        windowFlags() & ~Qt::WindowContextHelpButtonHint
        );

    ui->backgroundColorButton->setAutoDefault(false);
    ui->backgroundColorPreviewLabel->setMinimumSize(48, 24);
    ui->backgroundColorPreviewLabel->setText(QString());

    connect(
        ui->backgroundColorButton,
        &QPushButton::clicked,
        this,
        &SaveFrameDialog::chooseBackgroundColor
        );

    updateBackgroundPreview();
}

SaveFrameDialog::~SaveFrameDialog()
{
    delete ui;
}

void SaveFrameDialog::setFrameSource(
    SpinOpenGLWidget* widget,
    int index)
{
    frameWidget = widget;
    frameIndex = index;
}

void SaveFrameDialog::chooseBackgroundColor()
{
    const QColor chosen = QColorDialog::getColor(
        backgroundColor,
        this,
        QStringLiteral("Background color")
        );

    if (!chosen.isValid()) {
        return;
    }

    backgroundColor = chosen;
    backgroundColor.setAlpha(255);

    updateBackgroundPreview();
}

void SaveFrameDialog::updateBackgroundPreview()
{
    ui->backgroundColorPreviewLabel->setStyleSheet(
        QStringLiteral(
            "background-color: %1;"
            "border: 1px solid #808080;"
            ).arg(backgroundColor.name(QColor::HexRgb))
        );

    ui->backgroundColorPreviewLabel->setToolTip(
        backgroundColor.name(QColor::HexRgb)
        );
}

void SaveFrameDialog::accept()
{
    if (!frameWidget || frameIndex < 0) {
        QMessageBox::warning(
            this,
            QStringLiteral("Save frame"),
            QStringLiteral("No frame is available.")
            );
        return;
    }

    const int formatIndex =
        ui->formatComboBox->currentIndex();

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

    QFileDialog fileDialog(
        this,
        QStringLiteral("Save current frame")
        );

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setNameFilter(filter);
    fileDialog.setDefaultSuffix(suffix);

    fileDialog.selectFile(
        QStringLiteral("spin_frame_%1.%2")
            .arg(frameIndex)
            .arg(suffix)
        );

    if (fileDialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString fileName =
        fileDialog.selectedFiles().first();

    const QString extension =
        QFileInfo(fileName).suffix().toLower();

    const auto showError = [this](const QString& text)
    {
        QMessageBox::critical(
            this,
            QStringLiteral("Save frame"),
            text
            );
    };

    if (extension != suffix &&
        !(formatIndex == 2 &&
          extension == QStringLiteral("jpeg"))) {
        showError(
            QStringLiteral(
                "Use the .%1 extension for the selected format."
                ).arg(suffix)
            );
        return;
    }

    if (!frameWidget) {
        return;
    }

    if (formatIndex == 0) {
        QString error;

        if (!frameWidget->saveSvg(
                fileName,
                error,
                backgroundColor)) {
            showError(error);
            return;
        }
    }
    else {
        const QImage image =
            frameWidget->captureFrame(backgroundColor);

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

        QImageWriter writer(
            &output,
            formatIndex == 1
                ? QByteArray("png")
                : QByteArray("jpeg")
            );

        if (formatIndex == 2) {
            writer.setQuality(95);
        }

        if (!writer.write(image)) {
            showError(writer.errorString());
            return;
        }

        if (!output.commit()) {
            showError(output.errorString());
            return;
        }
    }

    QDialog::accept();
}