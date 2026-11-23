#include "random_single_ion_anisotropy_dialog.h"
#include "ui_random_single_ion_anisotropy_dialog.h"

RandomSingleIonAnisotropyDialog::RandomSingleIonAnisotropyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RandomSingleIonAnisotropyDialog)
{
    ui->setupUi(this);
}

RandomSingleIonAnisotropyDialog::~RandomSingleIonAnisotropyDialog()
{
    delete ui;
}
