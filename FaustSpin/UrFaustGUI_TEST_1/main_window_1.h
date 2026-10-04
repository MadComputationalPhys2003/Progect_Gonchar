#ifndef MAIN_WINDOW_1_H
#define MAIN_WINDOW_1_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow_1;
}
QT_END_NAMESPACE
class SystemResultsDialog;
class ParametersDialog;
class EnergyPlotDialog;
class SpinVisualizationDialog;
class MainWindow_1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow_1(QWidget *parent = nullptr);
    ~MainWindow_1() override;

private:
    Ui::MainWindow_1 *ui;
    ParametersDialog *parametersDialog = nullptr;
    SystemResultsDialog *systemResultsDialog = nullptr;
    EnergyPlotDialog *energyPlotDialog = nullptr;
    SpinVisualizationDialog *spinVisualizationDialog = nullptr;
};

#endif