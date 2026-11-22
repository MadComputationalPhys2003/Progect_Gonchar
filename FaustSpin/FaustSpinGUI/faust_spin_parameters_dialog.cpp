#include "FaustSpinSimulation.h"
#include "faust_spin_parameters_dialog.h"
#include "ui_faust_spin_parameters_dialog.h"
#include <QKeyEvent>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace {
using FaustSpin::Dimension;

constexpr std::array dimensions{Dimension::one, Dimension::two, Dimension::three};
constexpr std::uint64_t minExtent = 2;
class LatticeSizeSpinBox final : public QSpinBox
{
public:
    LatticeSizeSpinBox(QTableWidget *table, int column)
        : QSpinBox(table), table_(table), column_(column)
    {
        setButtonSymbols(QAbstractSpinBox::NoButtons);
        setSingleStep(1);
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        const int key = event->key();
        if (key != Qt::Key_Left && key != Qt::Key_Right) {
            QSpinBox::keyPressEvent(event);
            return;
        }

        const int nextColumn = column_ + (key == Qt::Key_Left ? -1 : 1);
        auto *next = table_->cellWidget(0, nextColumn);
        if (next && next->isEnabled()) {
            next->setFocus();
        }

        event->accept();
    }

private:
    QTableWidget *table_;
    int column_;
};
constexpr std::uint64_t maxExtent = 99;
}

FaustSpinParametersDialog::FaustSpinParametersDialog(
    const FaustSpinSim::SimulationConfig &config, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaustSpinParametersDialog)
{
    ui->setupUi(this);

    auto *table = ui->latticeSizeTableWidget;
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const std::array initialSizes{config.lx, config.ly, config.lz};
    const std::array names{"LxSpinBox", "LySpinBox", "LzSpinBox"};
    int rowHeight = table->verticalHeader()->minimumSectionSize();

    for (std::size_t i = 0; i < sizeEditors.size(); ++i) {
        auto *editor = new LatticeSizeSpinBox(table, static_cast<int>(i));
        editor->setObjectName(QString::fromLatin1(names[i]));
        editor->setRange(minExtent, maxExtent);
        editor->setValue(static_cast<int>(
            std::clamp(initialSizes[i], minExtent, maxExtent)));
        editor->setAlignment(Qt::AlignCenter);
        table->setCellWidget(0, static_cast<int>(i), editor);
        sizeEditors[i] = editor;
        rowHeight = std::max(rowHeight, editor->sizeHint().height());
    }

    table->setRowHeight(0, rowHeight);
    table->setFixedHeight(table->horizontalHeader()->sizeHint().height()
                          + rowHeight + 2 * table->frameWidth());

    for (std::size_t i = 0; i < dimensions.size(); ++i) {
        ui->dimensionComboBox->setItemData(
            static_cast<int>(i), static_cast<int>(dimensions[i]));
    }

    const int index = ui->dimensionComboBox->findData(static_cast<int>(config.dimension));
    ui->dimensionComboBox->setCurrentIndex(index >= 0 ? index : 0);
    ui->energyErrorSpinBox->setValue(config.epsilon);
    ui->maximumIterationsSpinBox->setValue(static_cast<int>(std::clamp(
        config.maxSweeps, std::size_t{1},
        static_cast<std::size_t>(ui->maximumIterationsSpinBox->maximum()))));

    connect(ui->dimensionComboBox, &QComboBox::currentIndexChanged,
            this, &FaustSpinParametersDialog::updateDimensionControls);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateDimensionControls(ui->dimensionComboBox->currentIndex());
}

FaustSpinParametersDialog::~FaustSpinParametersDialog()
{
    delete ui;
}

void FaustSpinParametersDialog::updateDimensionControls(int index)
{
    sizeEditors[1]->setEnabled(index >= 1);
    sizeEditors[2]->setEnabled(index >= 2);
}

void FaustSpinParametersDialog::applyTo(FaustSpinSim::SimulationConfig &config) const
{
    const int index = ui->dimensionComboBox->currentIndex();

    config.dimension = static_cast<Dimension>(ui->dimensionComboBox->currentData().toInt());
    config.lx = static_cast<std::uint64_t>(sizeEditors[0]->value());
    config.ly = index >= 1 ? static_cast<std::uint64_t>(sizeEditors[1]->value()) : 1;
    config.lz = index >= 2 ? static_cast<std::uint64_t>(sizeEditors[2]->value()) : 1;
    config.epsilon = ui->energyErrorSpinBox->value();
    config.maxSweeps = static_cast<std::size_t>(ui->maximumIterationsSpinBox->value());
}