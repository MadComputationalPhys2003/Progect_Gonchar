#include "faust_spin_visualization.h"
#include "ui_faust_spin_visualization.h"

FaustSpinVisualization::FaustSpinVisualization(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaustSpinVisualization)
{
    ui->setupUi(this);
}

FaustSpinVisualization::~FaustSpinVisualization()
{
    delete ui;
}
