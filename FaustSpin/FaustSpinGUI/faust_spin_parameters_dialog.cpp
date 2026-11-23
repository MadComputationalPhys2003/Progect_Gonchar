#include "faust_spin_parameters_dialog.h"
#include "ui_faust_spin_parameters_dialog.h"
#include "faust_spin_exchange_integral_dialog.h"
#include "faust_spin_single_ion_anisotropy_dialog.h"
#include "faust_spin_g_factor_dialog.h"
#include "spin_parameters_dialog.h"
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace {
using FaustSpin::Dimension;

constexpr std::array dimensions{Dimension::one, Dimension::two, Dimension::three};
constexpr std::uint64_t minExtent = 2;
constexpr std::uint64_t maxExtent = 99;

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

bool sameLattice(const FaustSpinSim::SimulationConfig &a,
                 const FaustSpinSim::SimulationConfig &b)
{
    return a.dimension == b.dimension && a.lx == b.lx && a.ly == b.ly && a.lz == b.lz;
}
} // namespace

FaustSpinParametersDialog::FaustSpinParametersDialog(
    const FaustSpinSim::SimulationConfig &config, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaustSpinParametersDialog)
    , exchangeConfig_(config)
    , anisotropyConfig_{
          .lx = config.lx,
          .ly = config.ly,
          .lz = config.lz,
          .dimension = config.dimension,
          .singleIonAnisotropy = config.singleIonAnisotropy,
          .anisotropyMode = config.anisotropyMode,
          .anisotropyDirection = config.anisotropyDirection
      }
{
    ui->setupUi(this);
    ui->checkBoxExchange->setChecked(config.exchangeEnabled);
    ui->checkBoxAnisotropy->setChecked(config.anisotropyEnabled);
    ui->checkBoxZeenmam->setChecked(config.zeemanEnabled);

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

    connect(ui->setExchangeIntegralButton, &QPushButton::clicked, this, [this] {
        FaustSpinSim::SimulationConfig candidate;
        readLattice(candidate);
        candidate.exchangeMode = exchangeConfig_.exchangeMode;

        if (exchangeFitsLattice(candidate)) {
            candidate.exchangeIntegral = exchangeConfig_.exchangeIntegral;
        } else {
            candidate.exchangeIntegral = std::vector<FM::Matrix>{};
        }

        FaustSpinExchangeIntegralDialog dialog(candidate, this);
        if (dialog.exec() == QDialog::Accepted) {
            exchangeConfig_ = std::move(candidate);
        }
    });

    connect(ui->pushButtonAnisotropy, &QPushButton::clicked, this, [this] {
        FaustSpinSim::SimulationConfig candidate;
        readLattice(candidate);
        candidate.anisotropyMode = anisotropyConfig_.anisotropyMode;
        candidate.anisotropyDirection = anisotropyConfig_.anisotropyDirection;

        if (anisotropyFitsLattice(candidate)) {
            candidate.singleIonAnisotropy = anisotropyConfig_.singleIonAnisotropy;
        } else {
            candidate.singleIonAnisotropy = std::vector<FM::Matrix>{};
        }

        FaustSpinSingleIonAnisotropyDialog dialog(candidate, this);
        if (dialog.exec() == QDialog::Accepted) {
            anisotropyConfig_ = std::move(candidate);
        }
    });
    connect(ui->pushButtonG, &QPushButton::clicked, this, [this] {
        FaustSpinGFactorDialog dialog(this);
        dialog.exec();
    });
    connect(ui->pushButtonSetSpitParametrs, &QPushButton::clicked, this, [this] {
        FaustSpinSim::SimulationConfig lattice;
        readLattice(lattice);

        const auto ionCount =
            static_cast<std::size_t>(lattice.lx * lattice.ly * lattice.lz);

        SpinParametersDialog dialog(ionCount, {}, this);
        dialog.exec();
    });
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
    readLattice(config);
    config.exchangeMode = exchangeConfig_.exchangeMode;
    config.exchangeIntegral = exchangeConfig_.exchangeIntegral;
    config.anisotropyMode = anisotropyConfig_.anisotropyMode;
    config.singleIonAnisotropy = anisotropyConfig_.singleIonAnisotropy;
    config.epsilon = ui->energyErrorSpinBox->value();
    config.maxSweeps = static_cast<std::size_t>(ui->maximumIterationsSpinBox->value());
    config.exchangeEnabled = ui->checkBoxExchange->isChecked();
    config.anisotropyEnabled = ui->checkBoxAnisotropy->isChecked();
    config.zeemanEnabled = ui->checkBoxZeenmam->isChecked();
}

void FaustSpinParametersDialog::readLattice(FaustSpinSim::SimulationConfig &config) const
{
    const int index = ui->dimensionComboBox->currentIndex();
    config.dimension = static_cast<FaustSpin::Dimension>(
        ui->dimensionComboBox->currentData().toInt());
    config.lx = static_cast<std::uint64_t>(sizeEditors[0]->value());
    config.ly = index >= 1 ? static_cast<std::uint64_t>(sizeEditors[1]->value()) : 1;
    config.lz = index >= 2 ? static_cast<std::uint64_t>(sizeEditors[2]->value()) : 1;
}

bool FaustSpinParametersDialog::exchangeFitsLattice(
    const FaustSpinSim::SimulationConfig &config) const
{
    return exchangeConfig_.exchangeMode == FaustSpinSim::ExchangeMode::constant ||
           sameLattice(exchangeConfig_, config);
}

bool FaustSpinParametersDialog::anisotropyFitsLattice(
    const FaustSpinSim::SimulationConfig &config) const
{
    return anisotropyConfig_.anisotropyMode == FaustSpinSim::AnisotropyMode::constant ||
           sameLattice(anisotropyConfig_, config);
}

void FaustSpinParametersDialog::accept()
{
    FaustSpinSim::SimulationConfig geometry;
    readLattice(geometry);

    if (!exchangeFitsLattice(geometry)) {
        QMessageBox::warning(this, tr("Exchange integral"),
                             tr("The lattice has changed. Set the exchange matrices again."));
        return;
    }

    if (!anisotropyFitsLattice(geometry)) {
        QMessageBox::warning(this, tr("Single-ion anisotropy"),
                             tr("The lattice has changed. Set the anisotropy matrices again."));
        return;
    }

    QDialog::accept();
}