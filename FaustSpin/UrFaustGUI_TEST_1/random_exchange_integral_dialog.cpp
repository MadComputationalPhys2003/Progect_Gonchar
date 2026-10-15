#include "random_exchange_integral_dialog.h"
#include "ui_random_exchange_integral_dialog.h"

#include <QDoubleSpinBox>
#include <QLocale>
#include <QMessageBox>
#include <QString>

#include <cmath>

RandomExchangeIntegralDialog::RandomExchangeIntegralDialog(
    QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RandomExchangeIntegralDialog)
{
    ui->setupUi(this);

    ui->minimumExchangeDoubleSpinBox->setLocale(
        QLocale::c()
        );

    ui->maximumExchangeDoubleSpinBox->setLocale(
        QLocale::c()
        );
}

RandomExchangeIntegralDialog::~RandomExchangeIntegralDialog()
{
    delete ui;
}

void RandomExchangeIntegralDialog::setBounds(
    double minimum,
    double maximum)
{
    ui->minimumExchangeDoubleSpinBox->setValue(minimum);
    ui->maximumExchangeDoubleSpinBox->setValue(maximum);
}

double RandomExchangeIntegralDialog::getMinimumExchange() const
{
    return ui->minimumExchangeDoubleSpinBox->value();
}

double RandomExchangeIntegralDialog::getMaximumExchange() const
{
    return ui->maximumExchangeDoubleSpinBox->value();
}

void RandomExchangeIntegralDialog::accept()
{
    ui->minimumExchangeDoubleSpinBox->interpretText();
    ui->maximumExchangeDoubleSpinBox->interpretText();

    const double minimum = getMinimumExchange();
    const double maximum = getMaximumExchange();

    if (!std::isfinite(minimum) ||
        !std::isfinite(maximum) ||
        minimum >= maximum) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid exchange range"),
            QStringLiteral(
                "Matrix element bounds must satisfy: "
                "minimum < maximum."
                )
            );

        return;
    }

    QDialog::accept();
}