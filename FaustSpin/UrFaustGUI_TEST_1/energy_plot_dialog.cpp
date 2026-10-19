#include "energy_plot_dialog.h"
#include "ui_energy_plot_dialog.h"
#include <QBrush>
#include <QColor>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <algorithm>
#include <cmath>
#include <QByteArray>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageWriter>
#include <QMessageBox>
#include <QPaintDevice>
#include <QPushButton>
#include <QSaveFile>
#include <QSize>
#include <QSizeF>
#include <QSvgGenerator>
EnergyPlotDialog::EnergyPlotDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EnergyPlotDialog)
{
    ui->setupUi(this);
    setWindowFlags(
        (windowFlags() & ~Qt::WindowContextHelpButtonHint)
        | Qt::CustomizeWindowHint
        | Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowMinimizeButtonHint
        | Qt::WindowCloseButtonHint
        );
    scene = new QGraphicsScene(this);
    ui->energyGraphicsView->setScene(scene);
    ui->energyGraphicsView->setRenderHint(QPainter::Antialiasing);
    ui->energyGraphicsView->setBackgroundBrush(QBrush(Qt::white));
    ui->savePlotButton->setAutoDefault(false);

    connect(ui->savePlotButton, &QPushButton::clicked,
            this, &EnergyPlotDialog::savePlot);
}

EnergyPlotDialog::~EnergyPlotDialog()
{
    delete ui;
}
void EnergyPlotDialog::setResult(
    const UrFaustSim::SimulationResult &result)
{
    scene->clear();

    if (result.frames.empty()) {
        return;
    }

    double minEnergy = result.frames.front().energy;
    double maxEnergy = minEnergy;

    for (const auto &frame : result.frames) {
        minEnergy = std::min(minEnergy, frame.energy);
        maxEnergy = std::max(maxEnergy, frame.energy);
    }

    double padding = (maxEnergy - minEnergy) * 0.1;

    if (padding == 0.0) {
        padding = std::max(1.0, std::abs(minEnergy) * 0.05);
    }

    minEnergy -= padding;
    maxEnergy += padding;

    const double maxIteration = std::max(
        1.0,
        static_cast<double>(result.frames.back().sweep)
        );

    const QRectF plot(110.0, 40.0, 640.0, 350.0);

    const auto mapX = [&](double iteration) {
        return plot.left()
        + iteration / maxIteration * plot.width();
    };

    const auto mapY = [&](double energy) {
        return plot.bottom()
        - (energy - minEnergy)
            / (maxEnergy - minEnergy) * plot.height();
    };

    const QPen gridPen(QColor(220, 220, 220));
    const QPen axisPen(Qt::black);

    for (int i = 0; i <= 4; ++i) {
        const double energy =
            maxEnergy - (maxEnergy - minEnergy) * i / 4.0;

        const double y = mapY(energy);

        scene->addLine(
            plot.left(), y, plot.right(), y, gridPen
            );

        auto *label = scene->addSimpleText(
            QString::number(energy, 'g', 12)
            );

        label->setPos(
            plot.left() - label->boundingRect().width() - 8.0,
            y - label->boundingRect().height() / 2.0
            );
    }

    const int xTicks =
        static_cast<int>(std::min(5.0, maxIteration));

    for (int i = 0; i <= xTicks; ++i) {
        const double iteration =
            std::round(maxIteration * i / xTicks);

        const double x = mapX(iteration);

        scene->addLine(
            x, plot.top(), x, plot.bottom(), gridPen
            );

        auto *label = scene->addSimpleText(
            QString::number(iteration, 'f', 0)
            );

        label->setPos(
            x - label->boundingRect().width() / 2.0,
            plot.bottom() + 8.0
            );
    }

    scene->addLine(
        plot.left(), plot.top(),
        plot.left(), plot.bottom(), axisPen
        );

    scene->addLine(
        plot.left(), plot.bottom(),
        plot.right(), plot.bottom(), axisPen
        );

    scene->addSimpleText(QStringLiteral("Energy"))
        ->setPos(20.0, 8.0);

    scene->addSimpleText(QStringLiteral("Iteration"))
        ->setPos(plot.center().x() - 30.0, plot.bottom() + 38.0);

    QPen curvePen(QColor(40, 110, 210));
    curvePen.setWidthF(2.0);

    QPainterPath curve;

    for (std::size_t i = 0; i < result.frames.size(); ++i) {
        const auto &frame = result.frames[i];

        const QPointF point(
            mapX(static_cast<double>(frame.sweep)),
            mapY(frame.energy)
            );

        if (i == 0) {
            curve.moveTo(point);
        } else {
            curve.lineTo(point);
        }

        if (result.frames.size() <= 100) {
            scene->addEllipse(
                point.x() - 3.0, point.y() - 3.0,
                6.0, 6.0,
                curvePen, QBrush(curvePen.color())
                );
        }
    }

    scene->addPath(curve, curvePen);

    scene->setSceneRect(
        scene->itemsBoundingRect().adjusted(
            -20.0, -20.0, 20.0, 20.0
            )
        );

    ui->energyGraphicsView->fitInView(
        scene->sceneRect(), Qt::KeepAspectRatio
        );
}
void EnergyPlotDialog::savePlot()
{
    if (!scene || scene->items().isEmpty()) {
        QMessageBox::information(
            this,
            QStringLiteral("Save energy plot"),
            QStringLiteral("Run a simulation first.")
            );
        return;
    }

    const auto showError = [this](const QString &message) {
        QMessageBox::critical(
            this,
            QStringLiteral("Save energy plot"),
            message
            );
    };

    const int formatIndex =
        ui->savePlotFormatComboBox->currentIndex();

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
        QStringLiteral("Save energy plot")
        );

    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(filter);
    dialog.setDefaultSuffix(suffix);

    dialog.selectFile(
        QStringLiteral("energy_plot.%1").arg(suffix)
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

    const QRectF source = scene->sceneRect();

    if (!source.isValid()) {
        showError(QStringLiteral("The plot has an invalid size."));
        return;
    }

    const QSize baseSize(
        std::max(
            1,
            static_cast<int>(std::ceil(source.width()))
            ),
        std::max(
            1,
            static_cast<int>(std::ceil(source.height()))
            )
        );

    // Для PNG и JPEG увеличиваем разрешение в три раза.
    const QSize outputSize =
        formatIndex == 0 ? baseSize : baseSize * 3;

    const QRectF target(
        QPointF(0.0, 0.0),
        QSizeF(outputSize)
        );

    // Одинаковая отрисовка для всех трёх форматов.
    const auto drawPlot =
        [this, &source, &target](QPaintDevice *device) {
            QPainter painter;

            if (!painter.begin(device)) {
                return false;
            }

            painter.setRenderHints(
                QPainter::Antialiasing |
                QPainter::TextAntialiasing
                );

            painter.fillRect(target, Qt::white);

            scene->render(
                &painter,
                target,
                source,
                Qt::KeepAspectRatio
                );

            return painter.end();
        };

    QSaveFile output(fileName);

    if (!output.open(QIODevice::WriteOnly)) {
        showError(output.errorString());
        return;
    }

    if (formatIndex == 0) {
        QSvgGenerator generator(
            QSvgGenerator::SvgVersion::Svg11
            );

        generator.setOutputDevice(&output);
        generator.setSize(outputSize);
        generator.setViewBox(target);
        generator.setResolution(96);
        generator.setTitle(QStringLiteral("Energy Plot"));

        if (!drawPlot(&generator)) {
            showError(
                QStringLiteral("Cannot render the SVG plot.")
                );
            return;
        }
    }
    else {
        QImage image(outputSize, QImage::Format_RGB32);

        if (image.isNull()) {
            showError(
                QStringLiteral("Cannot create the plot image.")
                );
            return;
        }

        image.fill(Qt::white);

        if (!drawPlot(&image)) {
            showError(
                QStringLiteral("Cannot render the plot image.")
                );
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
    }

    if (!output.commit()) {
        showError(output.errorString());
    }
}