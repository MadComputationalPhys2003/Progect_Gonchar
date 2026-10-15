#ifndef MANUAL_EXCHANGE_INTEGRAL_DIALOG_H
#define MANUAL_EXCHANGE_INTEGRAL_DIALOG_H

#include <QDialog>

#include "../FaustSpin/Faust math/Faust_Matrix.h"

#include <cstddef>
#include <vector>

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
        std::size_t ionCount,
        std::size_t neighborCount,
        const std::vector<FM::Matrix>& previous,
        double defaultExchange = -1.0
        );

    const std::vector<FM::Matrix>& getMatrices() const;

public slots:
    void accept() override;

private:
    Ui::ManualExchangeIntegralDialog* ui;

    std::vector<FM::Matrix> matricesValue;
    std::size_t neighborCountValue{0};

    void updateMatrixCell(int row);
    void editMatrix(int row, int column);
};

#endif // MANUAL_EXCHANGE_INTEGRAL_DIALOG_H