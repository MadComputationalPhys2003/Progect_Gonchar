#include "faust_spin_single_ion_anisotropy_dialog.h"
#include "ui_faust_spin_single_ion_anisotropy_dialog.h"
#include "manual_single_ion_anisotropy_dialog.h"
#include "random_single_ion_anisotropy_dialog.h"

#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <locale>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
constexpr int manualInput = 0;
constexpr int randomInput = 1;
constexpr int fileInput = 2;
constexpr int matrixMode = 1;

using FaustSpinSim::SimulationConfig;
using FaustSpinSim::Interaction;

std::size_t ionCount(const SimulationConfig &config)
{
    using FaustSpin::Dimension;
    const std::array<std::uint64_t, 3> sizes{
        config.lx,
        config.dimension == Dimension::one ? 1 : config.ly,
        config.dimension == Dimension::three ? config.lz : 1
    };

    std::size_t count = 1;
    for (const auto size : sizes) {
        if (size == 0 || size > std::numeric_limits<std::size_t>::max() / count) {
            throw std::invalid_argument("Invalid lattice dimensions");
        }
        count *= static_cast<std::size_t>(size);
    }
    return count;
}

double finiteNumber(const QString &text)
{
    bool ok = false;
    const double value = text.trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(value)) {
        throw std::invalid_argument("Enter a finite number. Use a decimal point.");
    }
    return value;
}

QString formatNumber(double value)
{
    return QString::number(value, 'g', std::numeric_limits<double>::max_digits10);
}

Interaction readAnisotropyFile(const QString &path, bool matrix, const SimulationConfig &config)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open the anisotropy file");
    }

    QByteArray contents = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        throw std::runtime_error("Cannot read the anisotropy file");
    }
    if (contents.startsWith("\xEF\xBB\xBF")) {
        contents.remove(0, 3);
    }

    std::istringstream input(contents.toStdString());
    input.imbue(std::locale::classic());

    const auto readValue = [&] {
        double value = 0.0;
        if (!(input >> value) || !std::isfinite(value)) {
            throw std::invalid_argument("Expected finite numbers in the anisotropy file");
        }
        return value;
    };

    Interaction result;
    if (matrix) {
        std::vector<FM::Matrix> tensors(ionCount(config), FM::Matrix(3, 3));
        for (auto &tensor : tensors) {
            for (std::size_t k = 0; k < 9; ++k) {
                tensor[k] = readValue();
            }
        }
        result = std::move(tensors);
    } else {
        result = readValue();
    }

    input >> std::ws;
    if (!input.eof() || input.bad()) {
        throw std::invalid_argument("Extra data in the anisotropy file");
    }
    return result;
}

Interaction randomAnisotropy(double minimum, double maximum, bool matrix,
                             const SimulationConfig &config)
{
    std::mt19937 generator{std::random_device{}()};
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    const auto sample = [&] { return std::lerp(minimum, maximum, unit(generator)); };

    if (!matrix) {
        return sample();
    }

    std::vector<FM::Matrix> tensors(ionCount(config), FM::Matrix(3, 3));
    for (auto &tensor : tensors) {
        for (int row = 0; row < 3; ++row) {
            for (int column = row; column < 3; ++column) {
                const double value = sample();
                tensor[3 * row + column] = value;
                tensor[3 * column + row] = value;
            }
        }
    }
    return tensors;
}
} // namespace

FaustSpinSingleIonAnisotropyDialog::FaustSpinSingleIonAnisotropyDialog(
    SimulationConfig &config, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::FaustSpinSingleIonAnisotropyDialog>())
    , config_(config)
{
    ui->setupUi(this);
    setWindowTitle(tr("Set Single Ion Anisotropy"));

    const bool matrix = config_.anisotropyMode == FaustSpinSim::AnisotropyMode::matrix;
    ui->anisotropyModeComboBox->setCurrentIndex(matrix ? matrixMode : 0);
    ui->anisotropyInputComboBox->setCurrentIndex(manualInput);

    if (matrix) {
        if (const auto *values = std::get_if<std::vector<FM::Matrix>>(&config_.singleIonAnisotropy);
            values && values->size() == ionCount(config_)) {
            selected_ = *values;
        }
    } else if (const auto *value = std::get_if<double>(&config_.singleIonAnisotropy)) {
        selected_ = *value;
    }

    const auto resetSelection = [this] {
        selected_.reset();
        ui->anisotropyFileLineEdit->clear();
        updateControls();
    };

    connect(ui->anisotropyModeComboBox, &QComboBox::currentIndexChanged, this, resetSelection);
    connect(ui->anisotropyInputComboBox, &QComboBox::currentIndexChanged, this, resetSelection);
    connect(ui->anisotropySettingsButton, &QPushButton::clicked,
            this, &FaustSpinSingleIonAnisotropyDialog::configureAnisotropy);
    updateControls();
}

FaustSpinSingleIonAnisotropyDialog::~FaustSpinSingleIonAnisotropyDialog() = default;

void FaustSpinSingleIonAnisotropyDialog::updateControls()
{
    const int input = ui->anisotropyInputComboBox->currentIndex();
    ui->anisotropyFileLineEdit->setVisible(input == fileInput);

    switch (input) {
    case manualInput:
        ui->anisotropySettingsButton->setText(tr("Set Manual..."));
        break;
    case randomInput:
        ui->anisotropySettingsButton->setText(tr("Set Random..."));
        break;
    case fileInput:
        ui->anisotropySettingsButton->setText(tr("Choose File..."));
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(selected_.has_value());
}

void FaustSpinSingleIonAnisotropyDialog::configureAnisotropy()
{
    const int input = ui->anisotropyInputComboBox->currentIndex();
    const bool matrix = ui->anisotropyModeComboBox->currentIndex() == matrixMode;

    try {
        if (input == fileInput) {
            const QString path = QFileDialog::getOpenFileName(
                this, tr("Choose anisotropy file"), ui->anisotropyFileLineEdit->text(),
                tr("Text files (*.txt);;All files (*)"));
            if (path.isEmpty()) {
                return;
            }

            selected_ = readAnisotropyFile(path, matrix, config_);
            ui->anisotropyFileLineEdit->setText(path);
        } else if (input == randomInput) {
            RandomSingleIonAnisotropyDialog dialog(this);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            const auto [minimum, maximum] = dialog.bounds();
            selected_ = randomAnisotropy(minimum, maximum, matrix, config_);
        } else {
            const auto &previous = selected_ ? *selected_ : config_.singleIonAnisotropy;
            const auto *coefficient = std::get_if<double>(&previous);

            if (matrix) {
                const auto count = ionCount(config_);
                std::vector<FM::Matrix> tensors;

                if (const auto *values = std::get_if<std::vector<FM::Matrix>>(&previous);
                    values && values->size() == count) {
                    tensors = *values;
                } else {
                    FM::Matrix initial(3, 3);
                    if (coefficient) {
                        const auto axis = config_.anisotropyDirection.value_or(
                            FaustSpinSim::Vector3{0.0, 0.0, 1.0});
                        for (int row = 0; row < 3; ++row) {
                            for (int column = 0; column < 3; ++column) {
                                initial[3 * row + column] = *coefficient * axis[row] * axis[column];
                            }
                        }
                    }
                    tensors.assign(count, initial);
                }

                ManualSingleIonAnisotropyDialog dialog(std::move(tensors), this);
                if (dialog.exec() != QDialog::Accepted) {
                    return;
                }
                selected_ = dialog.takeMatrices();
            } else {
                bool accepted = false;
                const QString value = QInputDialog::getText(
                    this, tr("Set Single Ion Anisotropy"), tr("D (meV)"),
                    QLineEdit::Normal, formatNumber(coefficient ? *coefficient : 0.0), &accepted);
                if (!accepted) {
                    return;
                }
                selected_ = finiteNumber(value);
            }
        }

        updateControls();
    } catch (const std::exception &error) {
        QMessageBox::warning(this, windowTitle(), QString::fromUtf8(error.what()));
    }
}

void FaustSpinSingleIonAnisotropyDialog::accept()
{
    if (!selected_) {
        return;
    }

    config_.anisotropyMode = ui->anisotropyModeComboBox->currentIndex() == matrixMode
                                 ? FaustSpinSim::AnisotropyMode::matrix : FaustSpinSim::AnisotropyMode::constant;
    config_.singleIonAnisotropy = std::move(*selected_);
    QDialog::accept();
}