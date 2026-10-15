#ifndef RANDOM_EXCHANGE_INTEGRAL_DIALOG_H
#define RANDOM_EXCHANGE_INTEGRAL_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class RandomExchangeIntegralDialog;
}
QT_END_NAMESPACE

class RandomExchangeIntegralDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RandomExchangeIntegralDialog(
        QWidget* parent = nullptr
        );

    ~RandomExchangeIntegralDialog() override;

    void setBounds(double minimum, double maximum);

    double getMinimumExchange() const;
    double getMaximumExchange() const;

public slots:
    void accept() override;

private:
    Ui::RandomExchangeIntegralDialog* ui;
};

#endif // RANDOM_EXCHANGE_INTEGRAL_DIALOG_H