#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "FaustSpinSimulation.h"
#include <QMainWindow>
#include <optional>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainMenu;
}
QT_END_NAMESPACE
class MainWindow_1;
class QAction;
class QThread;
class FaustSpinParametersDialog;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
  void startFaustSpin(FaustSpinParametersDialog &dialog);
    void saveFaustSpinTxt();

    Ui::MainMenu *ui;
    MainWindow_1 *urFaustWindow = nullptr;
    FaustSpinSim::SimulationConfig faustSpinConfig;
    std::optional<FaustSpinSim::SimulationResult> faustSpinResult_;
    QThread *faustSpinWorker_ = nullptr;
    QAction *saveFaustSpinTxtAction_ = nullptr;
};
#endif // MAINWINDOW_H
