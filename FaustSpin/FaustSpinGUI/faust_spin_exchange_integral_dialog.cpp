#include "faust_spin_exchange_integral_dialog.h"
#include "ui_faust_spin_exchange_integral_dialog.h"

#include "manual_exchange_dialog_faust_spin.h"
#include "manual_exchange_integral_dialog.h"
#include "random_exchange_integral_dialog.h"

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include <cmath>
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

FaustSpin::InitLattice makeLattice(const FaustSpinSim::SimulationConfig &config)
{
    using FaustSpin::Dimension;
    switch (config.dimension) {
    case Dimension::one:
        return {config.lx, 0};
    case Dimension::two:
        return {config.lx, config.ly, 0, 0};
    case Dimension::three:
        return {config.lx, config.ly, config.lz, 0, 0, 0};
    }
    throw std::invalid_argument("Unknown lattice dimension");
}

FaustSpinSim::Interaction readExchangeFile(
    const QString &path, bool matrix, const FaustSpinSim::SimulationConfig &config)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open the exchange integral file");
    }

    QByteArray contents = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        throw std::runtime_error("Cannot read the exchange integral file");
    }
    if (contents.startsWith("\xEF\xBB\xBF")) {
        contents.remove(0, 3);
    }

    std::istringstream input(contents.toStdString());
    input.imbue(std::locale::classic());

    const auto readValue = [&input] {
        double value;
        if (!(input >> value) || !std::isfinite(value)) {
            throw std::runtime_error("Expected finite numbers in the exchange integral file");
        }
        return value;
    };

    FaustSpinSim::Interaction result;
    if (matrix) {
        const auto count = makeLattice(config).getUniquePairs().size();
        std::vector<FM::Matrix> matrices(count, FM::Matrix(3, 3));
        for (auto &entry : matrices) {
            for (std::size_t k = 0; k < 9; ++k) {
                entry[k] = readValue();
            }
        }
        result = std::move(matrices);
    } else {
        result = readValue();
    }

    input >> std::ws;
    if (!input.eof() || input.bad()) {
        throw std::runtime_error("Extra data in the exchange integral file");
    }
    return result;
}
}

FaustSpinExchangeIntegralDialog::FaustSpinExchangeIntegralDialog(
    FaustSpinSim::SimulationConfig &config, QWidget *parent)
    : QDialog(parent), ui(new Ui::FaustSpinExchangeIntegralDialog), config_(config)
{
    ui->setupUi(this);
    ui->exchangeModeComboBox->setCurrentIndex(
        config_.exchangeMode == FaustSpinSim::ExchangeMode::matrix ? matrixMode : 0);
    ui->exchangeInputComboBox->setCurrentIndex(manualInput);

    const auto *matrices = std::get_if<std::vector<FM::Matrix>>(&config_.exchangeIntegral);
    if (!matrices || !matrices->empty()) {
        selected_ = config_.exchangeIntegral;
    }

    const auto resetSelection = [this] {
        selected_.reset();
        ui->exchangeIntegralFileLineEdit->clear();
        updateControls();
    };
    connect(ui->exchangeModeComboBox, &QComboBox::currentIndexChanged, this, resetSelection);
    connect(ui->exchangeInputComboBox, &QComboBox::currentIndexChanged, this, resetSelection);
    connect(ui->exchangeSettingsButton, &QPushButton::clicked,
            this, &FaustSpinExchangeIntegralDialog::configureExchange);
    updateControls();
}

FaustSpinExchangeIntegralDialog::~FaustSpinExchangeIntegralDialog()
{
    delete ui;
}

void FaustSpinExchangeIntegralDialog::updateControls()
{
    const int inputMode = ui->exchangeInputComboBox->currentIndex();
    ui->exchangeIntegralFileLineEdit->setVisible(inputMode == fileInput);

    switch (inputMode) {
    case manualInput:
        ui->exchangeSettingsButton->setText(tr("Set Manual..."));
        break;
    case randomInput:
        ui->exchangeSettingsButton->setText(tr("Set Random..."));
        break;
    case fileInput:
        ui->exchangeSettingsButton->setText(tr("Choose File..."));
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(selected_.has_value());
}

void FaustSpinExchangeIntegralDialog::configureExchange()
{
    const int inputMode = ui->exchangeInputComboBox->currentIndex();
    const bool matrix = ui->exchangeModeComboBox->currentIndex() == matrixMode;

    try {
        if (inputMode == fileInput) {
            const QString path = QFileDialog::getOpenFileName(
                this, tr("Choose exchange integral file"),
                ui->exchangeIntegralFileLineEdit->text(),
                tr("Text files (*.txt);;All files (*)"));
            if (path.isEmpty()) {
                return;
            }

            selected_ = readExchangeFile(path, matrix, config_);
            ui->exchangeIntegralFileLineEdit->setText(path);
        } else if (inputMode == randomInput) {
            if (!matrix) {
                QMessageBox::information(this, windowTitle(), tr("Select Matrix for random input."));
                return;
            }

            RandomExchangeIntegralDialog dialog(this);
            dialog.setBounds(randomMinimum_, randomMaximum_);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            randomMinimum_ = dialog.getMinimumExchange();
            randomMaximum_ = dialog.getMaximumExchange();

            const auto count = makeLattice(config_).getUniquePairs().size();
            std::vector<FM::Matrix> matrices(count, FM::Matrix(3, 3));
            std::mt19937 generator{std::random_device{}()};
            std::uniform_real_distribution<double> distribution(randomMinimum_, randomMaximum_);
            for (auto &entry : matrices) {
                for (std::size_t k = 0; k < 9; ++k) {
                    entry[k] = distribution(generator);
                }
            }
            selected_ = std::move(matrices);
        } else {
            const auto &previous = selected_ ? *selected_ : config_.exchangeIntegral;
            const auto *scalar = std::get_if<double>(&previous);
            const double initialJ = scalar ? *scalar : -1.0;

            if (matrix) {
                const auto lattice = makeLattice(config_);
                const auto *matrices = std::get_if<std::vector<FM::Matrix>>(&previous);
                const std::vector<FM::Matrix> empty;

                ManualExchangeDialogFaustSpin dialog(
                    lattice, config_.dimension, matrices ? *matrices : empty, initialJ, this);

                if (dialog.exec() != QDialog::Accepted) {
                    return;
                }
                selected_ = dialog.getMatrices();
            } else {
                ManualExchangeIntegralDialog dialog(this);
                dialog.setExchangeIntegral(initialJ);

                if (dialog.exec() != QDialog::Accepted) {
                    return;
                }
                selected_ = dialog.getExchangeIntegral();
            }
        }

        updateControls();
    } catch (const std::exception &error) {
        QMessageBox::warning(this, windowTitle(), QString::fromUtf8(error.what()));
    }
}

void FaustSpinExchangeIntegralDialog::accept()
{
    if (!selected_) {
        return;
    }

    config_.exchangeMode = ui->exchangeModeComboBox->currentIndex() == matrixMode
                               ? FaustSpinSim::ExchangeMode::matrix : FaustSpinSim::ExchangeMode::constant;
    config_.exchangeIntegral = std::move(*selected_);
    QDialog::accept();
}
