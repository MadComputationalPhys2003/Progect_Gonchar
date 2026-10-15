#include "random_direction_dialog.h"
#include "ui_random_direction_dialog.h"

#include <QCheckBox>
#include <QSpinBox>
#include <limits>
#include <stdexcept>

RandomDirectionDialog::RandomDirectionDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RandomDirectionDialog)
{
    ui->setupUi(this);

    ui->seedSpinBox->setRange(
        0, std::numeric_limits<int>::max());

    connect(ui->fixedSeedCheckBox,
            &QCheckBox::toggled,
            ui->seedSpinBox,
            &QSpinBox::setEnabled);

    setSeed(std::nullopt);
}

RandomDirectionDialog::~RandomDirectionDialog()
{
    delete ui;
}

void RandomDirectionDialog::setSeed(
    const std::optional<int>& seed)
{
    if (seed.has_value() && *seed < 0) {
        throw std::invalid_argument(
            "Direction seed must be nonnegative");
    }

    seedValue = seed;

    ui->fixedSeedCheckBox->setChecked(seed.has_value());
    ui->seedSpinBox->setValue(seed.value_or(0));
    ui->seedSpinBox->setEnabled(seed.has_value());
}

std::optional<int> RandomDirectionDialog::getSeed() const
{
    return seedValue;
}

void RandomDirectionDialog::accept()
{
    ui->seedSpinBox->interpretText();

    if (ui->fixedSeedCheckBox->isChecked()) {
        seedValue = ui->seedSpinBox->value();
    }
    else {
        seedValue = std::nullopt;
    }

    QDialog::accept();
}
