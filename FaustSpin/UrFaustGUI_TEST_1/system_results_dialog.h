#ifndef SYSTEM_RESULTS_DIALOG_H
#define SYSTEM_RESULTS_DIALOG_H
#include "UrFaustSimulation_1.h"
#include <QDialog>

namespace Ui {
class SystemResultsDialog;
}

class SystemResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemResultsDialog(QWidget *parent = nullptr);
    ~SystemResultsDialog();
    void setResult(const UrFaustSim::SimulationResult &result);
private:
    Ui::SystemResultsDialog *ui;
};

#endif // SYSTEM_RESULTS_DIALOG_H
