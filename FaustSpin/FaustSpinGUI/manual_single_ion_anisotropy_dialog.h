#pragma once

#include "Faust math/Faust_Matrix.h"

#include <QDialog>

#include <array>
#include <memory>
#include <vector>

class QLineEdit;
namespace Ui { class ManualSingleIonAnisotropyDialog; }

class ManualSingleIonAnisotropyDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ManualSingleIonAnisotropyDialog(std::vector<FM::Matrix> matrices,
                                             QWidget *parent = nullptr);
    ~ManualSingleIonAnisotropyDialog() override;

    std::vector<FM::Matrix> takeMatrices();
    void accept() override;

private:
    void showIon(int index);
    bool saveMatrix();

    std::unique_ptr<Ui::ManualSingleIonAnisotropyDialog> ui;
    std::vector<FM::Matrix> matrices_;
    std::array<QLineEdit *, 9> entries_{};
    int ion_ = 0;
};