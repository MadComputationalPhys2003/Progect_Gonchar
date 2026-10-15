#ifndef MANUAL_SPIN_LENGTH_DIALOG_H
#define MANUAL_SPIN_LENGTH_DIALOG_H

#include <QDialog>
#include <cstddef>
#include <vector>

namespace Ui {
class ManualSpinLengthDialog;
}

class ManualSpinLengthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualSpinLengthDialog(QWidget *parent = nullptr);
    ~ManualSpinLengthDialog() override;

    void setSpinLength(double value);
    double getSpinLength() const;

    void setSpinLengths(
        std::size_t count,
        const std::vector<double>& previous,
        double defaultLength = 1.0
        );

    const std::vector<double>& getSpinLengths() const;

public slots:
    void accept() override;

private:
    Ui::ManualSpinLengthDialog *ui;

    std::vector<double> lengthsValue;
};

#endif // MANUAL_SPIN_LENGTH_DIALOG_H