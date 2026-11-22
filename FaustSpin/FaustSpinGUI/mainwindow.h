#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "FaustSpinSimulation.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainMenu;
}
QT_END_NAMESPACE
class MainWindow_1;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainMenu *ui;
    MainWindow_1 *urFaustWindow = nullptr;
    FaustSpinSim::SimulationConfig faustSpinConfig;
};
#endif // MAINWINDOW_H
