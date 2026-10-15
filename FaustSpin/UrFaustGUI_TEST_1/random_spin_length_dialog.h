#ifndef RANDOM_SPIN_LENGTH_DIALOG_H
#define RANDOM_SPIN_LENGTH_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class RandomSpinLengthDialog;
}
QT_END_NAMESPACE

class RandomSpinLengthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RandomSpinLengthDialog(QWidget* parent = nullptr);
    ~RandomSpinLengthDialog() override;

    void setBounds(double minimum, double maximum);

    double getMinimumLength() const;
    double getMaximumLength() const;

public slots:
    void accept() override;

private:
    Ui::RandomSpinLengthDialog* ui;
};

#endif // RANDOM_SPIN_LENGTH_DIALOG_H