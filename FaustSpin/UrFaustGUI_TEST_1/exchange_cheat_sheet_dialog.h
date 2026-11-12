#pragma once

#include <QDialog>
#include <QPointF>

#include "../FaustSpin/Faust math/Faust_Matrix.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui {
class ExchangeCheatSheetDialog;
}
QT_END_NAMESPACE

class QEvent;
class QGraphicsEllipseItem;
class QGraphicsPathItem;
class QGraphicsScene;
class QGraphicsTextItem;

class ExchangeCheatSheetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExchangeCheatSheetDialog(QWidget* parent = nullptr);
    ~ExchangeCheatSheetDialog() override;

    void setData(
        std::uint64_t lx,
        std::uint64_t ly,
        std::uint64_t lz,
        const std::vector<std::pair<std::size_t, std::size_t>>& pairs,
        const std::vector<FM::Matrix>& matrices
        );

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Ui::ExchangeCheatSheetDialog* ui;
    QGraphicsScene* scene;

    std::array<std::uint64_t, 3> dimensions{{1, 1, 1}};
    std::size_t nodeCount = 0;

    std::vector<std::pair<std::size_t, std::size_t>> pairsValue;
    std::vector<FM::Matrix> matricesValue;

    std::vector<QGraphicsPathItem*> bonds;
    std::vector<QGraphicsTextItem*> bondLabels;
    std::vector<QGraphicsEllipseItem*> nodes;
    std::vector<bool> periodicBonds;

    std::array<std::uint64_t, 3> coords(std::size_t index) const;
    QPointF position(std::size_t index) const;

    bool crossesBoundary(std::size_t i, std::size_t j) const;

    void drawScene();
    void showMatrix(int number);
    void fitView();
};