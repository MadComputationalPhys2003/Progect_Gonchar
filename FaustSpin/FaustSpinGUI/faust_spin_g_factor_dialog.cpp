#include "faust_spin_g_factor_dialog.h"
#include "manual_single_ion_anisotropy_dialog.h"
#include "ui_faust_spin_g_factor_dialog.h"

#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <locale>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
using FaustSpinSim::Interaction;

constexpr int tensorMode = 1;
constexpr int manualInput = 0;
constexpr int randomInput = 1;
constexpr int fileInput = 2;

std::size_t ionCount(const FaustSpinSim::SimulationConfig &config)
{
    using FaustSpin::Dimension;
    const std::array<std::uint64_t, 3> extents{
        config.lx,
        config.dimension == Dimension::one ? 1 : config.ly,
        config.dimension == Dimension::three ? config.lz : 1
    };

    std::size_t count = 1;
    for (const auto extent : extents) {
        if (extent == 0 || extent > std::numeric_limits<std::size_t>::max() / count) {
            throw std::invalid_argument("Invalid lattice dimensions");
        }
        count *= static_cast<std::size_t>(extent);
    }
    return count;
}

double readFinite(std::istream &input)
{
    double value = 0.0;
    if (!(input >> value) || !std::isfinite(value)) {
        throw std::invalid_argument("Expected a finite number. Use a decimal point.");
    }
    return value;
}

void requireEnd(std::istream &input)
{
    input >> std::ws;
    if (!input.eof() || input.bad()) {
        throw std::invalid_argument("Unexpected extra input");
    }
}

Interaction readFile(const QString &path, bool tensor, std::size_t count)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open the g-factor file");
    }

    QByteArray contents = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        throw std::runtime_error("Cannot read the g-factor file");
    }
    if (contents.startsWith("\xEF\xBB\xBF")) {
        contents.remove(0, 3);
    }

    std::istringstream input(contents.toStdString());
    input.imbue(std::locale::classic());

    if (!tensor) {
        const double value = readFinite(input);
        requireEnd(input);
        return value;
    }

    std::vector<FM::Matrix> matrices(count, FM::Matrix(3, 3));
    for (auto &matrix : matrices) {
        for (std::size_t k = 0; k < 9; ++k) {
            matrix[k] = readFinite(input);
        }
    }
    requireEnd(input);
    return matrices;
}

Interaction generateRandom(double minimum, double maximum, bool tensor, std::size_t count)
{
    std::mt19937 generator{std::random_device{}()};
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    const auto sample = [&] { return std::lerp(minimum, maximum, unit(generator)); };

    if (!tensor) {
        return sample();
    }

    std::vector<FM::Matrix> matrices(count, FM::Matrix(3, 3));
    for (auto &matrix : matrices) {
        for (std::size_t k = 0; k < 9; ++k) {
            matrix[k] = sample();
        }
    }
    return matrices;
}
} // namespace

FaustSpinGFactorDialog::FaustSpinGFactorDialog(QWidget *parent)
    : FaustSpinGFactorDialog(FaustSpinSim::SimulationConfig{}, parent)
{
}

FaustSpinGFactorDialog::FaustSpinGFactorDialog(
    const FaustSpinSim::SimulationConfig &config, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::FaustSpinGFactorDialog>())
    , ionCount_(ionCount(config))
    , initial_(config.gFactor)
{
    ui->setupUi(this);
    setWindowTitle(tr("Set g-factor / g-tensor"));

    const bool tensor = config.zeemanMode == FaustSpinSim::ZeemanMode::matrix;
    ui->gFactorModeComboBox->setCurrentIndex(tensor ? tensorMode : 0);
    ui->gFactorInputComboBox->setCurrentIndex(manualInput);

    if (tensor) {
        if (const auto *matrices = std::get_if<std::vector<FM::Matrix>>(&initial_);
            matrices && matrices->size() == ionCount_) {
            selected_ = initial_;
        }
    } else if (std::holds_alternative<double>(initial_)) {
        selected_ = initial_;
    }

    const auto resetSelection = [this] {
        selected_.reset();
        ui->gFactorFileLineEdit->clear();
        updateControls();
    };
    connect(ui->gFactorModeComboBox, &QComboBox::currentIndexChanged, this, resetSelection);
    connect(ui->gFactorInputComboBox, &QComboBox::currentIndexChanged, this, resetSelection);
    connect(ui->gFactorSettingsButton, &QPushButton::clicked,
            this, &FaustSpinGFactorDialog::configureValue);
    updateControls();
}

FaustSpinGFactorDialog::~FaustSpinGFactorDialog() = default;

void FaustSpinGFactorDialog::updateControls()
{
    const int input = ui->gFactorInputComboBox->currentIndex();
    ui->gFactorFileLineEdit->setVisible(input == fileInput);

    switch (input) {
    case manualInput:
        ui->gFactorSettingsButton->setText(tr("Set Manual..."));
        break;
    case randomInput:
        ui->gFactorSettingsButton->setText(tr("Set Random..."));
        break;
    case fileInput:
        ui->gFactorSettingsButton->setText(tr("Choose File..."));
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(selected_.has_value());
}

void FaustSpinGFactorDialog::configureValue()
{
    const int input = ui->gFactorInputComboBox->currentIndex();
    const bool tensor = ui->gFactorModeComboBox->currentIndex() == tensorMode;

    try {
        if (input == fileInput) {
            const QString path = QFileDialog::getOpenFileName(
                this, tr("Choose g-factor file"), ui->gFactorFileLineEdit->text(),
                tr("Text files (*.txt);;All files (*)"));
            if (path.isEmpty()) {
                return;
            }

            selected_ = readFile(path, tensor, ionCount_);
            ui->gFactorFileLineEdit->setText(path);
        } else if (input == randomInput) {
            bool accepted = false;
            const QString text = QInputDialog::getText(
                this, tr("Random g-factor / g-tensor"),
                tr("Minimum and maximum (separated by a space)"),
                QLineEdit::Normal, QStringLiteral("1 3"), &accepted);
            if (!accepted) {
                return;
            }

            std::istringstream bounds(text.toStdString());
            bounds.imbue(std::locale::classic());
            const double minimum = readFinite(bounds);
            const double maximum = readFinite(bounds);
            requireEnd(bounds);

            if (minimum > maximum) {
                throw std::invalid_argument("Minimum must not exceed maximum");
            }
            selected_ = generateRandom(minimum, maximum, tensor, ionCount_);
        } else {
            const auto &previous = selected_ ? *selected_ : initial_;
            const auto *scalar = std::get_if<double>(&previous);
            const double g = scalar ? *scalar : 2.0;

            if (tensor) {
                std::vector<FM::Matrix> matrices;
                if (const auto *previousMatrices = std::get_if<std::vector<FM::Matrix>>(&previous);
                    previousMatrices && previousMatrices->size() == ionCount_) {
                    matrices = *previousMatrices;
                } else {
                    FM::Matrix isotropic(3, 3);
                    isotropic[0] = isotropic[4] = isotropic[8] = g;
                    matrices.assign(ionCount_, isotropic);
                }

                ManualSingleIonAnisotropyDialog dialog(std::move(matrices), this);
                dialog.setWindowTitle(tr("Manual g-tensors"));
                if (dialog.exec() != QDialog::Accepted) {
                    return;
                }
                selected_ = dialog.takeMatrices();
            } else {
                bool accepted = false;
                const QString text = QInputDialog::getText(
                    this, tr("Set g-factor"), tr("g (dimensionless)"), QLineEdit::Normal,
                    QString::number(g, 'g', std::numeric_limits<double>::max_digits10), &accepted);
                if (!accepted) {
                    return;
                }

                std::istringstream value(text.toStdString());
                value.imbue(std::locale::classic());
                const double number = readFinite(value);
                requireEnd(value);
                selected_ = number;
            }
        }

        updateControls();
    } catch (const std::exception &error) {
        QMessageBox::warning(this, windowTitle(), QString::fromUtf8(error.what()));
    }
}

void FaustSpinGFactorDialog::accept()
{
    if (selected_) {
        QDialog::accept();
    }
}

void FaustSpinGFactorDialog::applyTo(FaustSpinSim::SimulationConfig &config) const
{
    if (result() != QDialog::Accepted || !selected_) {
        return;
    }

    config.gFactor = *selected_;
    config.zeemanMode = std::holds_alternative<double>(config.gFactor)
                            ? FaustSpinSim::ZeemanMode::constant : FaustSpinSim::ZeemanMode::matrix;
}