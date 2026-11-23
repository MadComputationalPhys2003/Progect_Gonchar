#ifndef FAUST_SPIN_VISUALIZATION_H
#define FAUST_SPIN_VISUALIZATION_H

#include <QDialog>

namespace Ui {
class FaustSpinVisualization;
}

class FaustSpinVisualization : public QDialog
{
    Q_OBJECT

public:
    explicit FaustSpinVisualization(QWidget *parent = nullptr);
    ~FaustSpinVisualization();

private:
    Ui::FaustSpinVisualization *ui;
};

#endif // FAUST_SPIN_VISUALIZATION_H
