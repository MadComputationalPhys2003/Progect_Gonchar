#ifndef FAUST_SPIN_EXCHANGE_INTEGRAL_DIALOG_H
#define FAUST_SPIN_EXCHANGE_INTEGRAL_DIALOG_H

#include <QDialog>

namespace Ui {
class FaustSpinExchangeIntegralDialog;
}

class FaustSpinExchangeIntegralDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinExchangeIntegralDialog(QWidget *parent = nullptr);
    ~FaustSpinExchangeIntegralDialog();

private:
    Ui::FaustSpinExchangeIntegralDialog *ui;
};

#endif // FAUST_SPIN_EXCHANGE_INTEGRAL_DIALOG_H
