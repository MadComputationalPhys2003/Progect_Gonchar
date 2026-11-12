#ifndef MANUAL_EXCHANGE_INTEGRAL_DIALOG_H
#define MANUAL_EXCHANGE_INTEGRAL_DIALOG_H

#include <QDialog>

#include "../FaustSpin/Faust math/Faust_Matrix.h"

#include <cstddef>
#include <utility>
#include <vector>
#include <array>
#include <cstdint>
QT_BEGIN_NAMESPACE
namespace Ui {
class ManualExchangeIntegralDialog;
}
QT_END_NAMESPACE

class ManualExchangeIntegralDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualExchangeIntegralDialog(
        QWidget* parent = nullptr
        );

    ~ManualExchangeIntegralDialog() override;

    void setExchangeIntegral(double value);
    double getExchangeIntegral() const;

    void setMatrices(
        const std::vector<std::pair<std::size_t, std::size_t>>& pairs,
        const std::vector<FM::Matrix>& previous,
        double defaultExchange = -1.0
        );

    const std::vector<FM::Matrix>& getMatrices() const;
    void setLatticeSize(
        std::uint64_t lx,
        std::uint64_t ly,
        std::uint64_t lz
        );
public slots:
    void accept() override;

private:
    Ui::ManualExchangeIntegralDialog* ui;

    std::vector<FM::Matrix> matricesValue;
    std::vector<std::pair<std::size_t, std::size_t>> pairsValue;
std::array<std::uint64_t, 3> latticeSize{{0, 0, 0}};
    void updateMatrixCell(int row);
    void editMatrix(int row, int column);
    void findMatrix(bool nextMatch);
};

#endif // MANUAL_EXCHANGE_INTEGRAL_DIALOG_H
