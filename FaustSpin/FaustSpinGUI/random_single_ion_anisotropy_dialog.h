#pragma once

#include <QDialog>

#include <memory>
#include <utility>

namespace Ui { class RandomSingleIonAnisotropyDialog; }

class RandomSingleIonAnisotropyDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit RandomSingleIonAnisotropyDialog(QWidget *parent = nullptr);
    ~RandomSingleIonAnisotropyDialog() override;

    std::pair<double, double> bounds() const;
    void accept() override;

private:
    std::unique_ptr<Ui::RandomSingleIonAnisotropyDialog> ui;
    double minimum_ = -1.0;
    double maximum_ = 1.0;
};