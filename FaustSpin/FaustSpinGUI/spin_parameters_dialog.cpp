#include "spin_parameters_dialog.h"
#include "ui_spin_parameters_dialog.h"

#include "manual_direction_dialog.h"
#include "manual_spin_length_dialog.h"
#include "parallel_direction_dialog.h"
#include "random_direction_dialog.h"
#include "random_spin_length_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
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

std::istringstream openNumbers(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open the selected file.");
    }
    auto text = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        throw std::runtime_error("Cannot read the selected file.");
    }
    if (text.startsWith("\xEF\xBB\xBF")) {
        text.remove(0, 3);
    }

    std::istringstream input(text.toStdString());
    input.imbue(std::locale::classic());
    return input;
}

double readNumber(std::istream &input)
{
    double value;
    if (!(input >> value) || !std::isfinite(value)) {
        throw std::runtime_error("Expected a finite number written with a decimal point.");
    }
    return value;
}

void requireEnd(std::istream &input)
{
    input >> std::ws;
    if (!input.eof() || input.bad()) {
        throw std::runtime_error("The file contains extra data.");
    }
}

} // namespace

SpinParametersDialog::SpinParametersDialog(QWidget *parent)
    : SpinParametersDialog(0, Settings{}, parent)
{
}

SpinParametersDialog::SpinParametersDialog(
    std::size_t ionCount, const Settings &previous, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SpinParametersDialog)
    , ionCount_(ionCount)
    , settings_(previous)
{
    ui->setupUi(this);
    setWindowTitle(tr("Spin Parameters"));
    ui->gridLayout->addWidget(ui->spinLengthFileLineEdit, 5, 0);

    ui->InitialDirectionComboBox->setCurrentIndex(static_cast<int>(settings_.directionMode));
    ui->spinLengthModeComboBox->setCurrentIndex(
        std::holds_alternative<std::vector<double>>(settings_.spinLength) ? 1 : 0);
    ui->spinLengthInputComboBox->setCurrentIndex(static_cast<int>(settings_.lengthInput));

    for (auto *combo : {ui->InitialDirectionComboBox, ui->spinLengthModeComboBox,
                        ui->spinLengthInputComboBox}) {
        connect(combo, &QComboBox::currentIndexChanged,
                this, &SpinParametersDialog::updateControls);
    }
    connect(ui->initialDirectionSettingsButton, &QPushButton::clicked,
            this, &SpinParametersDialog::configureDirections);
    connect(ui->spinLengthSettingsButton, &QPushButton::clicked,
            this, &SpinParametersDialog::configureLengths);
    updateControls();
}

SpinParametersDialog::~SpinParametersDialog()
{
    delete ui;
}

const SpinParametersDialog::Settings &SpinParametersDialog::settings() const
{
    return settings_;
}

SpinParametersDialog::DirectionMode SpinParametersDialog::directionMode() const
{
    return static_cast<DirectionMode>(ui->InitialDirectionComboBox->currentIndex());
}

SpinParametersDialog::LengthInput SpinParametersDialog::lengthInput() const
{
    return static_cast<LengthInput>(ui->spinLengthInputComboBox->currentIndex());
}

bool SpinParametersDialog::differentLengths() const
{
    return ui->spinLengthModeComboBox->currentIndex() == 1;
}

bool SpinParametersDialog::directionsReady() const
{
    if (directionMode() != settings_.directionMode) {
        return false;
    }

    if (directionMode() == DirectionMode::manual || directionMode() == DirectionMode::file) {
        return ionCount_ != 0 && settings_.ionDirections.size() == ionCount_;
    }
    return true;
}

bool SpinParametersDialog::lengthsReady() const
{
    if (lengthInput() != settings_.lengthInput) {
        return false;
    }

    if (!differentLengths()) {
        return std::holds_alternative<double>(settings_.spinLength);
    }
    const auto *lengths = std::get_if<std::vector<double>>(&settings_.spinLength);
    return lengths && ionCount_ != 0 && lengths->size() == ionCount_;
}

std::size_t SpinParametersDialog::requireIonCount() const
{
    if (ionCount_ == 0) {
        throw std::runtime_error("The number of ions was not passed from Parameters.");
    }
    return ionCount_;
}

void SpinParametersDialog::updateControls()
{
    const bool directionFromFile = directionMode() == DirectionMode::file;
    ui->initialDirectionFileLineEdit->setVisible(directionFromFile);
    ui->initialDirectionFileLineEdit->setText(
        directionFromFile && directionsReady() ? settings_.directionFile : QString{});

    switch (directionMode()) {
    case DirectionMode::parallel:
        ui->initialDirectionSettingsButton->setText(tr("Set Parallel..."));
        break;
    case DirectionMode::random:
        ui->initialDirectionSettingsButton->setText(tr("Set Random..."));
        break;
    case DirectionMode::file:
        ui->initialDirectionSettingsButton->setText(tr("Choose File..."));
        break;
    case DirectionMode::manual:
        ui->initialDirectionSettingsButton->setText(tr("Set Manual..."));
        break;
    }

    const bool lengthFromFile = lengthInput() == LengthInput::file;
    ui->spinLengthFileLineEdit->setVisible(lengthFromFile);
    ui->spinLengthFileLineEdit->setText(
        lengthFromFile && lengthsReady() ? settings_.lengthFile : QString{});

    switch (lengthInput()) {
    case LengthInput::manual:
        ui->spinLengthSettingsButton->setText(tr("Set Manual..."));
        break;
    case LengthInput::random:
        ui->spinLengthSettingsButton->setText(tr("Set Random..."));
        break;
    case LengthInput::file:
        ui->spinLengthSettingsButton->setText(tr("Choose File..."));
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
        directionsReady() && lengthsReady());
}

void SpinParametersDialog::configureDirections()
{
    const auto mode = directionMode();
    try {
        switch (mode) {
        case DirectionMode::parallel: {
            ParallelDirectionDialog dialog(this);
            dialog.setDirection(settings_.parallelDirection);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }
            settings_.parallelDirection = dialog.getDirection();
            break;
        }
        case DirectionMode::random: {
            RandomDirectionDialog dialog(this);
            dialog.setSeed(settings_.directionSeed);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }
            settings_.directionSeed = dialog.getSeed();
            break;
        }
        case DirectionMode::manual: {
            ManualDirectionDialog dialog(this);
            dialog.setDirections(requireIonCount(), settings_.ionDirections);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }
            settings_.ionDirections = dialog.getDirections();
            break;
        }
        case DirectionMode::file: {
            const auto count = requireIonCount();
            const QString path = QFileDialog::getOpenFileName(
                this, tr("Choose spin directions file"), settings_.directionFile,
                tr("Text files (*.txt);;All files (*)"));
            if (path.isEmpty()) {
                return;
            }

            auto input = openNumbers(path);
            std::vector<Vector3> directions;
            directions.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                const double x = readNumber(input);
                const double y = readNumber(input);
                const double z = readNumber(input);
                if (std::abs(std::hypot(x, y, z) - 1.0) > 1.0e-12) {
                    throw std::runtime_error(
                        "Each spin direction must be a unit vector.");
                }
                directions.emplace_back(x, y, z);
            }
            requireEnd(input);
            settings_.ionDirections = std::move(directions);
            settings_.directionFile = path;
            break;
        }
        }
        settings_.directionMode = mode;
        updateControls();
    } catch (const std::exception &error) {
        QMessageBox::warning(this, windowTitle(), QString::fromUtf8(error.what()));
    }
}

void SpinParametersDialog::configureLengths()
{
    const auto inputMode = lengthInput();
    try {
        switch (inputMode) {
        case LengthInput::manual: {
            ManualSpinLengthDialog dialog(this);
            const auto *scalar = std::get_if<double>(&settings_.spinLength);
            if (differentLengths()) {
                const auto *lengths = std::get_if<std::vector<double>>(&settings_.spinLength);
                const std::vector<double> empty;
                dialog.setSpinLengths(requireIonCount(), lengths ? *lengths : empty,
                                      scalar ? *scalar : 1.0);
            } else {
                dialog.setSpinLength(scalar ? *scalar : 1.0);
            }
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            if (differentLengths()) {
                settings_.spinLength = dialog.getSpinLengths();
            } else {
                settings_.spinLength = dialog.getSpinLength();
            }
            break;
        }
        case LengthInput::random: {
            if (!differentLengths()) {
                QMessageBox::information(this, windowTitle(),
                                         tr("Select Different for random spin lengths."));
                return;
            }
            const auto count = requireIonCount();
            RandomSpinLengthDialog dialog(this);
            dialog.setBounds(settings_.randomMinimum, settings_.randomMaximum);
            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            const double minimum = dialog.getMinimumLength();
            const double maximum = dialog.getMaximumLength();
            std::mt19937 generator{std::random_device{}()};
            std::uniform_real_distribution<double> distribution(minimum, maximum);
            std::vector<double> lengths(count);
            for (double &length : lengths) {
                length = distribution(generator);
            }
            settings_.spinLength = std::move(lengths);
            settings_.randomMinimum = minimum;
            settings_.randomMaximum = maximum;
            break;
        }
        case LengthInput::file: {
            const bool different = differentLengths();
            const auto count = different ? requireIonCount() : std::size_t{1};
            const QString path = QFileDialog::getOpenFileName(
                this, tr("Choose spin lengths file"), settings_.lengthFile,
                tr("Text files (*.txt);;All files (*)"));
            if (path.isEmpty()) {
                return;
            }

            auto input = openNumbers(path);
            std::vector<double> lengths(count);
            for (double &length : lengths) {
                length = readNumber(input);
                if (length <= 0.0) {
                    throw std::runtime_error("Spin lengths must be positive.");
                }
            }
            requireEnd(input);
            if (different) {
                settings_.spinLength = std::move(lengths);
            } else {
                settings_.spinLength = lengths.front();
            }
            settings_.lengthFile = path;
            break;
        }
        }
        settings_.lengthInput = inputMode;
        updateControls();
    } catch (const std::exception &error) {
        QMessageBox::warning(this, windowTitle(), QString::fromUtf8(error.what()));
    }
}

void SpinParametersDialog::accept()
{
    if (directionsReady() && lengthsReady()) {
        QDialog::accept();
    }
}