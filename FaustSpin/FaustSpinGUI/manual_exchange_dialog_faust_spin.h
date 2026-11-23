#pragma once

#include "FaustSpin.h"

#include <QDialog>

#include <memory>
#include <vector>

namespace Ui { class ManualExchangeDialogFaustSpin; }

class ManualExchangeDialogFaustSpin final : public QDialog
{
public:
    ManualExchangeDialogFaustSpin(const FaustSpin::InitLattice &lattice,
                                  FaustSpin::Dimension dimension,
                                  const std::vector<FM::Matrix> &previous,
                                  double defaultExchange,
                                  QWidget *parent = nullptr);
    ~ManualExchangeDialogFaustSpin() override;

    const std::vector<FM::Matrix> &getMatrices() const;
    void accept() override;

private:
    class LatticeView;
    class MatrixItemDelegate;

    bool saveCurrentMatrix();
    void selectBond(int index);

    std::unique_ptr<Ui::ManualExchangeDialogFaustSpin> ui;
    const FaustSpin::InitLattice &lattice_;
    std::vector<FM::Matrix> matrices_;
    MatrixItemDelegate *matrixEditor_;
    LatticeView *view_;
    int currentBond_ = -1;
};
