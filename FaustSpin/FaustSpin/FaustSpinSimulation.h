#pragma once

#include "FaustSpin.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace FaustSpinSim {

using Vector3 = Vector3D_Decart::Vector3D_Dec;
using Interaction = std::variant<double, std::vector<FM::Matrix>>;
using ExchangeMode = ExchangeIntegral::ExchangeIntegralInit;
using AnisotropyMode = Single_Ion_Anisotropy::Single_Ion_Anisotropy_Init;
using ZeemanMode = Zeeman_Term::Zeeman_Term_Init;
using SpinLengthMode = InitIons::Ions::SpinLengthInit;
using SpinDirectionMode = InitIons::Ions::SpinDirectionInit;

struct SimulationConfig {
    std::uint64_t lx{2};
    std::uint64_t ly{1};
    std::uint64_t lz{1};
    FaustSpin::Dimension dimension{FaustSpin::Dimension::one};

    std::variant<double, std::vector<double>> spinLength{1.0};
    SpinLengthMode spinLengthMode{SpinLengthMode::constant};
    SpinDirectionMode initialization{SpinDirectionMode::parallel};
    Vector3 parallelDirection{1.0, 0.0, 0.0};
    std::optional<int> directionSeed;
    std::string directionFile;

    bool exchangeEnabled{true};
    Interaction exchangeIntegral{-1.0};  // meV, in exchangePairs order
    ExchangeMode exchangeMode{ExchangeMode::constant};

    bool anisotropyEnabled{false};
    Interaction singleIonAnisotropy{0.0};  // meV, in spinIndices order
    AnisotropyMode anisotropyMode{AnisotropyMode::constant};
    std::optional<Vector3> anisotropyDirection;  // Unit axis, defaults to z.

    bool zeemanEnabled{false};
    Interaction gFactor{2.0};  // Dimensionless, in spinIndices order.
    ZeemanMode zeemanMode{ZeemanMode::constant};
    Vector3 magneticField{0.0, 0.0, 0.0};  // T

    double epsilon{1.0e-10};  // Energy tolerance in meV.
    double directionTolerance{1.0e-7};  // Chord length on the unit sphere.
    std::size_t maxSweeps{1000};
};

struct SimulationFrame {
    std::size_t sweep{};
    double energy{};  // Total Hamiltonian in meV.
    std::vector<Vector3> spinDirections;
};

struct SimulationResult {
    SimulationConfig config;
    std::vector<SimulationFrame> frames;  // Initial state followed by completed sweeps.
    std::vector<std::size_t> spinIndices;
    std::vector<double> spinLengths;
    // In matrix mode, exchangeIntegral[k] acts from j to i for exchangePairs[k] = (i, j).
    std::vector<std::pair<std::size_t, std::size_t>> exchangePairs;
    double bohrMagneton{};  // meV/T
    std::size_t completedSweeps{};
    bool converged{};

    Vector3 spinVector(std::size_t frameIndex, std::size_t ionIndex) const
    {
        return spinLengths.at(ionIndex) * frames.at(frameIndex).spinDirections.at(ionIndex);
    }
};

namespace detail {

using Components = std::array<double, 3>;
using Matrix3 = std::array<Components, 3>;
using Vector3D_Decart::scalar_product;

struct AnisotropySpectrum {
    double scale{};
    Components gaps{};
    std::array<Vector3, 3> axes{
        Vector3{1.0, 0.0, 0.0},
        Vector3{0.0, 1.0, 0.0},
        Vector3{0.0, 0.0, 1.0}
    };
};

inline double length(const Components& values)
{
    return std::hypot(values[0], values[1], values[2]);
}

inline bool finite(const Vector3& value)
{
    return std::isfinite(value[0]) && std::isfinite(value[1]) && std::isfinite(value[2]);
}

inline double maxAbs(const Matrix3& matrix)
{
    double largest = 0.0;
    for (const auto& row : matrix) {
        for (const double element : row) {
            largest = std::max(largest, std::abs(element));
        }
    }
    return largest;
}

inline AnisotropySpectrum diagonalize(Matrix3 matrix)
{
    AnisotropySpectrum spectrum;
    spectrum.scale = maxAbs(matrix);
    if (spectrum.scale == 0.0) {
        return spectrum;
    }

    for (auto& row : matrix) {
        for (auto& element : row) {
            element /= spectrum.scale;
        }
    }
    // A multiple of the identity adds the same energy for all spin directions.
    const double shift = matrix[0][0];
    for (std::size_t i = 0; i < 3; ++i) {
        matrix[i][i] -= shift;
    }

    const double tolerance = 8.0 * std::numeric_limits<double>::epsilon() * maxAbs(matrix);
    // Jacobi rotations accumulate the eigenvectors in the columns of the basis.
    for (std::size_t rotation = 0; rotation < 32; ++rotation) {
        std::size_t p = 0;
        std::size_t q = 1;
        for (std::size_t i = 0; i < 3; ++i) {
            for (std::size_t j = i + 1; j < 3; ++j) {
                if (std::abs(matrix[i][j]) > std::abs(matrix[p][q])) {
                    p = i;
                    q = j;
                }
            }
        }
        const double offDiagonal = matrix[p][q];
        if (std::abs(offDiagonal) <= tolerance) {
            break;
        }

        const double tau = (matrix[q][q] - matrix[p][p]) / (2.0 * offDiagonal);
        const double tangent = std::copysign(1.0, tau) / (std::abs(tau) + std::hypot(1.0, tau));
        const double cosine = 1.0 / std::hypot(1.0, tangent);
        const double sine = tangent * cosine;

        matrix[p][p] -= tangent * offDiagonal;
        matrix[q][q] += tangent * offDiagonal;
        matrix[p][q] = matrix[q][p] = 0.0;
        for (std::size_t k = 0; k < 3; ++k) {
            if (k != p && k != q) {
                const double first = matrix[k][p];
                const double second = matrix[k][q];
                matrix[k][p] = matrix[p][k] = cosine * first - sine * second;
                matrix[k][q] = matrix[q][k] = sine * first + cosine * second;
            }
        }

        const auto firstAxis = spectrum.axes[p];
        const auto secondAxis = spectrum.axes[q];
        spectrum.axes[p] = cosine * firstAxis - sine * secondAxis;
        spectrum.axes[q] = sine * firstAxis + cosine * secondAxis;
    }

    const double smallest = std::min({matrix[0][0], matrix[1][1], matrix[2][2]});
    for (std::size_t i = 0; i < 3; ++i) {
        const double gap = matrix[i][i] - smallest;
        spectrum.gaps[i] = gap <= 4.0 * tolerance ? 0.0 : gap;
    }
    return spectrum;
}

inline Components coordinatesAtShift(const Components& gaps, const Components& drive, double shift)
{
    return {drive[0] / (gaps[0] + shift),
            drive[1] / (gaps[1] + shift),
            drive[2] / (gaps[2] + shift)};
}

// Solve (A + lambda I)n = d, |n| = 1, with A + lambda I positive semidefinite.
// Hager, Minimizing a Quadratic Over a Sphere (2001), section 2.
inline Components minimizeOnSphere(const Components& gaps, const Components& drive,
                                   const Components& current)
{
    Components minimumSpaceDrive{};
    Components minimumSpaceCurrent{};
    Components otherComponents{};
    for (std::size_t i = 0; i < 3; ++i) {
        if (gaps[i] == 0.0) {
            minimumSpaceDrive[i] = drive[i];
            minimumSpaceCurrent[i] = current[i];
        } else {
            otherComponents[i] = drive[i] / gaps[i];
        }
    }

    double lower = length(minimumSpaceDrive);
    const double otherLength = length(otherComponents);
    if (lower == 0.0 && otherLength <= 1.0) {
        // With zero shift, fill the remaining length in the smallest
        // eigenspace, preferring the direction closest to the current one.
        const double radius = std::sqrt((1.0 - otherLength) * (1.0 + otherLength));
        const double projectedLength = length(minimumSpaceCurrent);
        if (projectedLength > 0.0) {
            for (std::size_t i = 0; i < 3; ++i) {
                otherComponents[i] += radius * (minimumSpaceCurrent[i] / projectedLength);
            }
        } else {
            const auto index = std::find(gaps.begin(), gaps.end(), 0.0) - gaps.begin();
            otherComponents[static_cast<std::size_t>(index)] = radius;
        }
        return otherComponents;
    }

    double upper = length(drive);
    if (otherLength < 1.0) {
        const double radius = std::sqrt((1.0 - otherLength) * (1.0 + otherLength));
        upper = std::min(upper, lower / radius);
    }
    for (std::size_t iteration = 0; iteration < 80; ++iteration) {
        const double middle = lower > 0.0 && lower < upper / 16.0
            ? std::sqrt(lower) * std::sqrt(upper)
            : lower + (upper - lower) / 2.0;
        if (middle == lower || middle == upper) {
            break;
        }
        if (length(coordinatesAtShift(gaps, drive, middle)) > 1.0) {
            lower = middle;
        } else {
            upper = middle;
        }
    }
    return coordinatesAtShift(gaps, drive, upper);
}

inline Vector3 minimizeDirection(const Vector3& current, double spinLength, const Vector3& field,
                                 const AnisotropySpectrum* spectrum)
{
    if (!finite(field)) {
        throw std::runtime_error("Local linear field must be finite");
    }
    const double fieldScale = std::max({std::abs(field[0]), std::abs(field[1]), std::abs(field[2])});
    if (spectrum == nullptr || spectrum->gaps == Components{}) {
        if (fieldScale == 0.0) {
            return current;
        }
        return Vector3D_Decart::normalize(
            Vector3{field[0] / fieldScale, field[1] / fieldScale, field[2] / fieldScale});
    }

    // Divide the local energy by the spin length and rescale its coefficients.
    const double quadraticScale = spinLength * spectrum->scale;
    const double scale = std::max(fieldScale, quadraticScale);
    if (!std::isfinite(scale)) {
        throw std::runtime_error("Local anisotropy scale must be finite");
    }
    if (scale == 0.0) {
        return current;
    }
    const Vector3 halfField{0.5 * (field[0] / scale),
                            0.5 * (field[1] / scale),
                            0.5 * (field[2] / scale)};
    Components gaps{};
    Components drive{};
    Components direction{};
    for (std::size_t i = 0; i < 3; ++i) {
        gaps[i] = (quadraticScale / scale) * spectrum->gaps[i];
        drive[i] = scalar_product(halfField, spectrum->axes[i]);
        direction[i] = scalar_product(current, spectrum->axes[i]);
    }
    const auto optimum = minimizeOnSphere(gaps, drive, direction);
    Vector3 candidate{0.0, 0.0, 0.0};
    for (std::size_t i = 0; i < 3; ++i) {
        candidate += optimum[i] * spectrum->axes[i];
    }
    return Vector3D_Decart::normalize(candidate);
}

} // namespace detail

class Simulation {
private:
    SimulationConfig config_;
    FaustSpin::FaustSpinSystemInit system_;
    Vector3 anisotropyAxis_;
    std::vector<detail::AnisotropySpectrum> spectra_;
    std::vector<Vector3> zeemanFields_;

    static std::size_t checkedProduct(std::size_t left, std::uint64_t right)
    {
        if (right == 0 || right > std::numeric_limits<std::size_t>::max() / left) {
            throw std::invalid_argument("Lattice extent is zero or the lattice is too large");
        }
        return left * static_cast<std::size_t>(right);
    }

    static SimulationConfig validateConfig(SimulationConfig config)
    {
        std::size_t neighbors = 0;
        switch (config.dimension) {
        case FaustSpin::Dimension::one:
            config.ly = 1;
            config.lz = 1;
            neighbors = 2;
            break;
        case FaustSpin::Dimension::two:
            config.lz = 1;
            neighbors = 4;
            break;
        case FaustSpin::Dimension::three:
            neighbors = 6;
            break;
        default:
            throw std::invalid_argument("Unknown lattice dimension");
        }
        const std::size_t count = checkedProduct(checkedProduct(1, config.lx), config.ly);
        checkedProduct(checkedProduct(count, config.lz), neighbors);

        if (!std::isfinite(config.epsilon) || config.epsilon <= 0.0 ||
            !std::isfinite(config.directionTolerance) || config.directionTolerance <= 0.0 ||
            config.maxSweeps == 0) {
            throw std::invalid_argument("Tolerances and maximum sweeps must be positive");
        }
        if (!detail::finite(config.magneticField)) {
            throw std::invalid_argument("Magnetic field must be finite");
        }
        switch (config.initialization) {
        case SpinDirectionMode::parallel:
        case SpinDirectionMode::random:
        case SpinDirectionMode::file:
            break;
        default:
            throw std::invalid_argument("Unknown spin direction initialization");
        }
        return config;
    }

    void prepareAnisotropy()
    {
        if (!config_.anisotropyEnabled) {
            return;
        }
        const auto& anisotropy = config_.singleIonAnisotropy;
        if (const auto* coefficient = std::get_if<double>(&anisotropy)) {
            detail::Matrix3 matrix{};
            for (std::size_t row = 0; row < 3; ++row) {
                for (std::size_t column = row; column < 3; ++column) {
                    const double element = *coefficient * anisotropyAxis_[row] * anisotropyAxis_[column];
                    matrix[row][column] = matrix[column][row] = element;
                }
            }
            spectra_.push_back(detail::diagonalize(matrix));
            return;
        }
        const auto& tensors = std::get<std::vector<FM::Matrix>>(anisotropy);
        spectra_.reserve(tensors.size());
        for (const auto& tensor : tensors) {
            detail::Matrix3 symmetric{};
            for (std::size_t row = 0; row < 3; ++row) {
                for (std::size_t column = row; column < 3; ++column) {
                    const double element =
                        0.5 * tensor[3 * row + column] + 0.5 * tensor[3 * column + row];
                    symmetric[row][column] = symmetric[column][row] = element;
                }
            }
            spectra_.push_back(detail::diagonalize(symmetric));
        }
    }

    void prepareZeemanFields()
    {
        if (!config_.zeemanEnabled) {
            return;
        }
        if (const auto* g = std::get_if<double>(&config_.gFactor)) {
            zeemanFields_.push_back((system_.mu_B * *g) * config_.magneticField);
            return;
        }
        const auto& tensors = std::get<std::vector<FM::Matrix>>(config_.gFactor);
        zeemanFields_.reserve(tensors.size());
        for (const auto& tensor : tensors) {
            Vector3 field{0.0, 0.0, 0.0};
            // Components of mu_B G^T B, with the same G as in the Hamiltonian.
            for (std::size_t column = 0; column < 3; ++column) {
                for (std::size_t row = 0; row < 3; ++row) {
                    field[column] += tensor[3 * row + column] * config_.magneticField[row];
                }
            }
            zeemanFields_.push_back(system_.mu_B * field);
        }
    }

    Vector3 linearField(std::size_t i) const
    {
        Vector3 field{0.0, 0.0, 0.0};
        if (!zeemanFields_.empty()) {
            field = zeemanFields_[zeemanFields_.size() == 1 ? 0 : i];
        }
        if (!config_.exchangeEnabled) {
            return field;
        }

        const auto& lattice = system_.getLattice();
        const auto& neighbors = lattice.getNeighborIndices();
        const auto& pairsBySlot = lattice.getMatrixIndexBySlot();
        const std::size_t neighborCount = lattice.getNeighborCount();
        const std::size_t firstSlot = i * neighborCount;
        const auto firstPair = pairsBySlot.begin() + firstSlot;
        for (std::size_t offset = 0; offset < neighborCount; ++offset) {
            const std::size_t j = static_cast<std::size_t>(neighbors[firstSlot + offset]);
            const auto pair = firstPair + offset;
            // With periodic boundaries, multiple neighbor slots can name the same bond.
            if (j != i && std::find(firstPair, pair, *pair) == pair) {
                field += FaustSpin::HamiltonianTermsAndMinAlgorithms::exchange_field(
                    config_.exchangeIntegral, config_.exchangeMode, system_.getIons(), i, j, *pair);
            }
        }
        return field;
    }

    double energyChange(std::size_t i, const Vector3& field, const Vector3& difference,
                        const Vector3& sum) const
    {
        using Vector3D_Decart::scalar_product;
        double change = -scalar_product(field, difference);
        if (config_.anisotropyEnabled) {
            if (const auto* D = std::get_if<double>(&config_.singleIonAnisotropy)) {
                change += *D * scalar_product(difference, anisotropyAxis_) *
                          scalar_product(sum, anisotropyAxis_);
            } else {
                const auto& D_i = std::get<std::vector<FM::Matrix>>(config_.singleIonAnisotropy)[i];
                change += 0.5 * scalar_product(difference, D_i * sum) +
                          0.5 * scalar_product(sum, D_i * difference);
            }
        }
        if (!std::isfinite(change)) {
            throw std::runtime_error("Local energy change must be finite");
        }
        return change;
    }

    double minimizationSweep()
    {
        auto& ions = system_.getIons().ions;
        double largestChange = 0.0;
        for (std::size_t i = 0; i < ions.size(); ++i) {
            auto& ion = ions[i];
            const auto& direction = ion.get_direction();
            const double spinLength = ion.get_ion_spin_length();
            const auto field = linearField(i);
            const auto* spectrum = spectra_.empty()
                ? nullptr
                : &spectra_[spectra_.size() == 1 ? 0 : i];
            const auto candidate = detail::minimizeDirection(direction, spinLength, field, spectrum);
            const auto difference = candidate - direction;
            const double change =
                energyChange(i, field, spinLength * difference, spinLength * (candidate + direction));
            if (change < 0.0) {
                largestChange = std::max(largestChange, Vector3D_Decart::norm(difference));
                ion.set_direction(candidate);
            }
        }
        return largestChange;
    }

    void restoreDirections(const std::vector<Vector3>& directions)
    {
        auto& ions = system_.getIons().ions;
        for (std::size_t i = 0; i < ions.size(); ++i) {
            ions[i].set_direction(directions[i]);
        }
    }

public:
    explicit Simulation(SimulationConfig config)
        : config_(validateConfig(std::move(config))),
          system_(config_.lx, config_.ly, config_.lz, 0, 0, 0,
                  config_.spinLength, config_.exchangeIntegral,
                  config_.singleIonAnisotropy, config_.gFactor,
                  config_.spinLengthMode, config_.initialization, config_.exchangeMode,
                  config_.anisotropyMode, config_.zeemanMode, config_.magneticField,
                  config_.dimension, config_.epsilon,
                  config_.exchangeEnabled, config_.anisotropyEnabled, config_.zeemanEnabled,
                  config_.directionFile, config_.directionSeed, config_.anisotropyDirection),
          anisotropyAxis_(config_.anisotropyDirection.value_or(Vector3{0.0, 0.0, 1.0}))
    {
        if (config_.initialization == SpinDirectionMode::parallel) {
            system_.setParallelDirection(config_.parallelDirection);
        }
        prepareAnisotropy();
        prepareZeemanFields();
    }

    const SimulationConfig& getConfig() const
    {
        return config_;
    }

    std::size_t getSpinCount() const
    {
        return system_.getLattice().getN();
    }

    double getEnergy()
    {
        const double energy = FaustSpin::Hamiltonan(system_);
        if (!std::isfinite(energy)) {
            throw std::runtime_error("Simulation energy must be finite");
        }
        return energy;
    }

    std::vector<Vector3> getSpinDirections() const
    {
        return system_.getSpinDirections();
    }

    std::vector<double> getSpinLengths() const
    {
        std::vector<double> lengths;
        lengths.reserve(getSpinCount());
        for (const auto& ion : system_.getIons().ions) {
            lengths.push_back(ion.get_ion_spin_length());
        }
        return lengths;
    }

    // Each run starts from the current state and counts its own sweeps from zero.
    SimulationResult run()
    {
        SimulationResult result;
        result.config = config_;
        result.spinIndices.resize(getSpinCount());
        std::iota(result.spinIndices.begin(), result.spinIndices.end(), std::size_t{0});
        result.spinLengths = getSpinLengths();
        result.exchangePairs = system_.getLattice().getUniquePairs();
        result.bohrMagneton = system_.mu_B;
        result.frames.push_back({0, getEnergy(), getSpinDirections()});

        for (std::size_t sweep = 0; sweep < config_.maxSweeps; ++sweep) {
            const auto& previous = result.frames.back();
            try {
                const double directionChange = minimizationSweep();
                const double energy = getEnergy();
                const bool converged = directionChange <= config_.directionTolerance &&
                    std::abs(previous.energy - energy) <= config_.epsilon;
                if (energy > previous.energy) {
                    // The full energy can round differently from the local increments.
                    restoreDirections(previous.spinDirections);
                    result.converged = converged;
                    break;
                }
                result.frames.push_back({sweep + 1, energy, getSpinDirections()});
                result.completedSweeps = sweep + 1;
                result.converged = converged;
                if (converged) {
                    break;
                }
            } catch (...) {
                restoreDirections(result.frames.back().spinDirections);
                throw;
            }
        }
        return result;
    }
};

} // namespace FaustSpinSim
