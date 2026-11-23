#include "manual_single_ion_anisotropy_dialog.h"
#include "ui_manual_single_ion_anisotropy_dialog.h"

ManualSingleIonAnisotropyDialog::ManualSingleIonAnisotropyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualSingleIonAnisotropyDialog)
{
    ui->setupUi(this);
}

ManualSingleIonAnisotropyDialog::~ManualSingleIonAnisotropyDialog()
{
    delete ui;
}
