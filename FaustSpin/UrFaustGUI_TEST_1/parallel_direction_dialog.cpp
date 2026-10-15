#include "parallel_direction_dialog.h"
#include "ui_parallel_direction_dialog.h"

#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QString>
#include <cmath>

ParallelDirectionDialog::ParallelDirectionDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ParallelDirectionDialog)
{
    ui->setupUi(this);
    setDirection(directionValue);
}

ParallelDirectionDialog::~ParallelDirectionDialog()
{
    delete ui;
}

void ParallelDirectionDialog::setDirection(
    const Vector3D_Decart::Vector3D_Dec& direction)
{
    directionValue = direction;

    ui->nxDoubleSpinBox->setValue(direction[0]);
    ui->nyDoubleSpinBox->setValue(direction[1]);
    ui->nzDoubleSpinBox->setValue(direction[2]);
}

const Vector3D_Decart::Vector3D_Dec&
ParallelDirectionDialog::getDirection() const
{
    return directionValue;
}

void ParallelDirectionDialog::accept()
{
    ui->nxDoubleSpinBox->interpretText();
    ui->nyDoubleSpinBox->interpretText();
    ui->nzDoubleSpinBox->interpretText();

    const double x = ui->nxDoubleSpinBox->value();
    const double y = ui->nyDoubleSpinBox->value();
    const double z = ui->nzDoubleSpinBox->value();

    const double length = std::hypot(x, y, z);

    if (!std::isfinite(length) || length <= 0.0) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid direction"),
            QStringLiteral("Direction must be a nonzero vector."));
        return;
    }

    directionValue = Vector3D_Decart::Vector3D_Dec(
        x / length,
        y / length,
        z / length);

    QDialog::accept();
}