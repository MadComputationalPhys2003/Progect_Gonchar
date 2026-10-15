#include "random_spin_length_dialog.h"
#include "ui_random_spin_length_dialog.h"

#include <QDoubleSpinBox>
#include <QLocale>
#include <QMessageBox>
#include <QString>

#include <cmath>

RandomSpinLengthDialog::RandomSpinLengthDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RandomSpinLengthDialog)
{
    ui->setupUi(this);

    ui->minimumLengthDoubleSpinBox->setLocale(QLocale::c());
    ui->maximumLengthDoubleSpinBox->setLocale(QLocale::c());
}

RandomSpinLengthDialog::~RandomSpinLengthDialog()
{
    delete ui;
}

void RandomSpinLengthDialog::setBounds(
    double minimum,
    double maximum)
{
    ui->minimumLengthDoubleSpinBox->setValue(minimum);
    ui->maximumLengthDoubleSpinBox->setValue(maximum);
}

double RandomSpinLengthDialog::getMinimumLength() const
{
    return ui->minimumLengthDoubleSpinBox->value();
}

double RandomSpinLengthDialog::getMaximumLength() const
{
    return ui->maximumLengthDoubleSpinBox->value();
}

void RandomSpinLengthDialog::accept()
{
    ui->minimumLengthDoubleSpinBox->interpretText();
    ui->maximumLengthDoubleSpinBox->interpretText();

    const double minimum = getMinimumLength();
    const double maximum = getMaximumLength();

    if (!std::isfinite(minimum) ||
        !std::isfinite(maximum) ||
        minimum <= 0.0 ||
        minimum >= maximum) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid spin lengths"),
            QStringLiteral(
                "Spin lengths must satisfy: 0 < minimum < maximum."
                )
            );

        return;
    }

    QDialog::accept();
}