#include "faust_spin_exchange_integral_dialog.h"
#include "ui_faust_spin_exchange_integral_dialog.h"

FaustSpinExchangeIntegralDialog::FaustSpinExchangeIntegralDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaustSpinExchangeIntegralDialog)
{
    ui->setupUi(this);
}

FaustSpinExchangeIntegralDialog::~FaustSpinExchangeIntegralDialog()
{
    delete ui;
}
