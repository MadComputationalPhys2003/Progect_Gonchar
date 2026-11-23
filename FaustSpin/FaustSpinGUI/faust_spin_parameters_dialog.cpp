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
#include <QCheckBox>
#include <QDoubleSpinBox>

#include <cmath>
#include <limits>
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
    , magneticField_(config.magneticField)
{
    ui->setupUi(this);
    ui->checkBoxExchange->setChecked(config.exchangeEnabled);
    ui->checkBoxAnisotropy->setChecked(config.anisotropyEnabled);
    ui->checkBoxZeenmam->setChecked(config.zeemanEnabled);
    setupMagneticFieldEditors();
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

        SpinParametersDialog dialog(
            ionCount, spinSettings_.value_or(SpinParametersDialog::Settings{}), this);

        if (dialog.exec() == QDialog::Accepted) {
            spinSettings_ = dialog.settings();
        }
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
    config.maxSweeps = static_cast<std::size_t>(
        ui->maximumIterationsSpinBox->value());
    config.exchangeEnabled = ui->checkBoxExchange->isChecked();
    config.anisotropyEnabled = ui->checkBoxAnisotropy->isChecked();
    config.zeemanEnabled = ui->checkBoxZeenmam->isChecked();
    config.magneticField = magneticField_;

    if (!spinSettings_) {
        return;
    }

    const auto &settings = *spinSettings_;
    using DirectionMode = SpinParametersDialog::DirectionMode;
    using FaustSpinSim::SpinDirectionMode;
    using FaustSpinSim::SpinLengthMode;

    config.spinLength = settings.spinLength;
    config.spinLengthMode = std::holds_alternative<double>(settings.spinLength)
                                ? SpinLengthMode::constant
                                : SpinLengthMode::different;

    config.initialSpinDirections.reset();

    switch (settings.directionMode) {
    case DirectionMode::parallel:
        config.initialization = SpinDirectionMode::parallel;
        config.parallelDirection = settings.parallelDirection;
        break;
    case DirectionMode::random:
        config.initialization = SpinDirectionMode::random;
        config.directionSeed = settings.directionSeed;
        break;
    case DirectionMode::file:
        config.initialization = SpinDirectionMode::file;
        config.directionFile = settings.directionFile.toStdString();
        config.initialSpinDirections = settings.ionDirections;
        break;
    case DirectionMode::manual:
        config.initialization = SpinDirectionMode::parallel;
        config.initialSpinDirections = settings.ionDirections;
        break;
    }
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
    if (validateInputs()) {
        QDialog::accept();
    }
}

bool FaustSpinParametersDialog::validateInputs()
{
    FaustSpinSim::SimulationConfig geometry;
    readLattice(geometry);

    if (!exchangeFitsLattice(geometry)) {
        QMessageBox::warning(this, tr("Exchange integral"),
                             tr("The lattice has changed. Set the exchange matrices again."));
        return false;
    }

    if (!anisotropyFitsLattice(geometry)) {
        QMessageBox::warning(this, tr("Single-ion anisotropy"),
                             tr("The lattice has changed. Set the anisotropy matrices again."));
        return false;
    }

    return readMagneticField();
}
void FaustSpinParametersDialog::setupMagneticFieldEditors()
{
    auto *table = ui->magneticFieldTableWidget;
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    const double magnitude = std::hypot(
        magneticField_[0], magneticField_[1], magneticField_[2]);

    std::array<double, 4> values{magnitude, 0.0, 0.0, 1.0};
    if (magnitude > 0.0) {
        for (std::size_t i = 0; i < 3; ++i) {
            values[i + 1] = magneticField_[i] / magnitude;
        }
    }

    for (std::size_t column = 0; column < magneticFieldEditors_.size(); ++column) {
        auto *editor = new QDoubleSpinBox(table);
        editor->setDecimals(12);
        editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
        editor->setAlignment(Qt::AlignCenter);

        if (column == 0) {
            editor->setRange(0.0, std::numeric_limits<double>::max());
            editor->setSingleStep(1.0);
        } else {
            editor->setRange(-1.0, 1.0);
            editor->setSingleStep(0.1);
        }

        editor->setValue(values[column]);
        table->setCellWidget(0, static_cast<int>(column), editor);
        magneticFieldEditors_[column] = editor;
    }

    table->resizeRowsToContents();
    connect(ui->checkBoxZeenmam, &QCheckBox::toggled,
            table, &QWidget::setVisible);
    table->setVisible(ui->checkBoxZeenmam->isChecked());
}

bool FaustSpinParametersDialog::readMagneticField()
{
    if (!ui->checkBoxZeenmam->isChecked()) {
        return true;
    }

    for (auto *editor : magneticFieldEditors_) {
        editor->interpretText();
    }

    const double magnitude = magneticFieldEditors_[0]->value();
    const double nx = magneticFieldEditors_[1]->value();
    const double ny = magneticFieldEditors_[2]->value();
    const double nz = magneticFieldEditors_[3]->value();
    const double norm = std::hypot(nx, ny, nz);

    constexpr double tolerance = 1.0e-9;
    if (std::abs(norm - 1.0) > tolerance) {
        QMessageBox::warning(this, tr("Magnetic field"),
                             tr("The field direction must be a unit vector."));
        magneticFieldEditors_[1]->setFocus();
        return false;
    }

    magneticField_ = FaustSpinSim::Vector3{
        magnitude * (nx / norm),
        magnitude * (ny / norm),
        magnitude * (nz / norm)
    };
    return true;
}