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