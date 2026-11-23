#include "faust_spin_single_ion_anisotropy_dialog.h"
#include "ui_faust_spin_single_ion_anisotropy_dialog.h"

FaustSpinSingleIonAnisotropyDialog::FaustSpinSingleIonAnisotropyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaustSpinSingleIonAnisotropyDialog)
{
    ui->setupUi(this);
}

FaustSpinSingleIonAnisotropyDialog::~FaustSpinSingleIonAnisotropyDialog()
{
    delete ui;
}
