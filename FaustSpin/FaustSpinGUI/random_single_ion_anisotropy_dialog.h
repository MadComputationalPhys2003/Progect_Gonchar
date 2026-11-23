#ifndef RANDOM_SINGLE_ION_ANISOTROPY_DIALOG_H
#define RANDOM_SINGLE_ION_ANISOTROPY_DIALOG_H

#include <QDialog>

namespace Ui {
class RandomSingleIonAnisotropyDialog;
}

class RandomSingleIonAnisotropyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RandomSingleIonAnisotropyDialog(QWidget *parent = nullptr);
    ~RandomSingleIonAnisotropyDialog();

private:
    Ui::RandomSingleIonAnisotropyDialog *ui;
};

#endif // RANDOM_SINGLE_ION_ANISOTROPY_DIALOG_H
