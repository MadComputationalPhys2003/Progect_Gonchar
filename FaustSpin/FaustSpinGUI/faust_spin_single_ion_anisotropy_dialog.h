#ifndef FAUST_SPIN_SINGLE_ION_ANISOTROPY_DIALOG_H
#define FAUST_SPIN_SINGLE_ION_ANISOTROPY_DIALOG_H

#include <QDialog>

namespace Ui {
class FaustSpinSingleIonAnisotropyDialog;
}

class FaustSpinSingleIonAnisotropyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinSingleIonAnisotropyDialog(QWidget *parent = nullptr);
    ~FaustSpinSingleIonAnisotropyDialog();

private:
    Ui::FaustSpinSingleIonAnisotropyDialog *ui;
};

#endif // FAUST_SPIN_SINGLE_ION_ANISOTROPY_DIALOG_H
