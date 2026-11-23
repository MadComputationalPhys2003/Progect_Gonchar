#ifndef MANUAL_SINGLE_ION_ANISOTROPY_DIALOG_H
#define MANUAL_SINGLE_ION_ANISOTROPY_DIALOG_H

#include <QDialog>

namespace Ui {
class ManualSingleIonAnisotropyDialog;
}

class ManualSingleIonAnisotropyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualSingleIonAnisotropyDialog(QWidget *parent = nullptr);
    ~ManualSingleIonAnisotropyDialog();

private:
    Ui::ManualSingleIonAnisotropyDialog *ui;
};

#endif // MANUAL_SINGLE_ION_ANISOTROPY_DIALOG_H
