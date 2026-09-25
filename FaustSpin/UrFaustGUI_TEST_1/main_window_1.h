#ifndef MAIN_WINDOW_1_H
#define MAIN_WINDOW_1_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow_1;
}
QT_END_NAMESPACE

class MainWindow_1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow_1(QWidget *parent = nullptr);
    ~MainWindow_1() override;

private:
    Ui::MainWindow_1 *ui;
};
#endif // MAIN_WINDOW_1_H
