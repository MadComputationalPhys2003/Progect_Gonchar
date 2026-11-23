#include "random_single_ion_anisotropy_dialog.h"
#include "ui_random_single_ion_anisotropy_dialog.h"

#include <QMessageBox>

#include <cmath>

RandomSingleIonAnisotropyDialog::RandomSingleIonAnisotropyDialog(QWidget *parent)
    : QDialog(parent), ui(std::make_unique<Ui::RandomSingleIonAnisotropyDialog>())
{
    ui->setupUi(this);
    setWindowTitle(tr("Random Single Ion Anisotropy"));
    ui->anisotropyMinimumLineEdit->setText(QStringLiteral("-1"));
    ui->anisotropyMaximumLineEdit->setText(QStringLiteral("1"));
}

RandomSingleIonAnisotropyDialog::~RandomSingleIonAnisotropyDialog() = default;

std::pair<double, double> RandomSingleIonAnisotropyDialog::bounds() const
{
    return {minimum_, maximum_};
}

void RandomSingleIonAnisotropyDialog::accept()
{
    bool minimumOk = false;
    bool maximumOk = false;
    const double minimum = ui->anisotropyMinimumLineEdit->text().trimmed().toDouble(&minimumOk);
    const double maximum = ui->anisotropyMaximumLineEdit->text().trimmed().toDouble(&maximumOk);

    if (!minimumOk || !maximumOk || !std::isfinite(minimum) || !std::isfinite(maximum) ||
        minimum > maximum) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Enter finite bounds with minimum <= maximum. Use a decimal point."));
        return;
    }

    minimum_ = minimum;
    maximum_ = maximum;
    QDialog::accept();
}