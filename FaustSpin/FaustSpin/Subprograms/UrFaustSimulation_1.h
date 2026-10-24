#pragma once

#include "Urfaust.h"

#include <optional>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <filesystem>
#include <fstream>

namespace UrFaustSim {

    struct SimulationConfig {
        std::uint64_t lx{ 2 };
        std::uint64_t ly{ 1 };
        std::uint64_t lz{ 1 };

        UrFaust::Dimension dimension{
            UrFaust::Dimension::one
        };

        std::variant<double, std::vector<double>>
            spinLength{ 1.0 };

        InitIons::Ions::SpinLengthInit spinLengthMode{
            InitIons::Ions::SpinLengthInit::constant
        };

        std::variant<double, std::vector<FM::Matrix>>
            exchangeIntegral{ -1.0 };

        ExchangeIntegral::ExchangeIntegralInit exchangeMode{
            ExchangeIntegral::ExchangeIntegralInit::constant
        };

        InitIons::Ions::SpinDirectionInit initialization{
            InitIons::Ions::SpinDirectionInit::parallel
        };
        Vector3D_Decart::Vector3D_Dec parallelDirection{
         1.0, 0.0, 0.0
        };
        std::optional<int> directionSeed;
        std::string directionFile;

        double epsilon{ 1.0e-10 };
        std::size_t maxSweeps{ 1000 };
        std::filesystem::path outputDirectory{ "Output" };

        bool exportSpinTxt{ false };
        bool exportEnergyTxt{ false };
        bool exportResultTex{ false };

        std::filesystem::path texTemplatePath{
            "Latex templates/UrFaustResult.tex"
        };

        std::size_t texRowsPerPage{ 30 };
    };

    struct SimulationFrame {
        std::size_t sweep{};
        double energy{};

        std::vector<Vector3D_Decart::Vector3D_Dec>
            spinDirections;
    };

    struct SimulationResult {
        SimulationConfig config;
        std::vector<SimulationFrame> frames;

        std::size_t completedSweeps{};
        bool converged{};

        // Äëèíû ôèêñèðîâàíû âî âðåìÿ ðåëàêñàöèè.
        // Ïîýòîìó ñîõðàíÿåì èõ îäèí ðàç íà âåñü ðàñ÷¸ò.
        std::vector<double> spinLengths;
    };

    class Simulation {
    private:
        SimulationConfig config;

        std::size_t count;
        UrFaust::SpinSystem spinSystem;

        static std::size_t checkedProduct(
            std::size_t left,
            std::uint64_t right)
        {
            if (left == 0 || right == 0 ||
                right >
                std::numeric_limits<std::size_t>::max() / left) {
                throw std::invalid_argument(
                    "Lattice size is zero or exceeds the index range"
                );
            }

            return left * static_cast<std::size_t>(right);
        }

        static std::size_t spinCount(
            const SimulationConfig& value)
        {
            std::size_t result =
                checkedProduct(1, value.lx);

            result = checkedProduct(result, value.ly);

            return checkedProduct(result, value.lz);
        }

        static std::size_t neighborCount(
            UrFaust::Dimension dimension)
        {
            switch (dimension) {
            case UrFaust::Dimension::one:
                return 2;

            case UrFaust::Dimension::two:
                return 4;

            case UrFaust::Dimension::three:
                return 6;

            default:
                throw std::invalid_argument(
                    "Unknown lattice dimension"
                );
            }
        }

        static std::size_t uniquePairCount(
            const SimulationConfig& value)
        {
            switch (value.dimension) {
            case UrFaust::Dimension::one:
                return UrFaust::InitLattice(value.lx, 0)
                    .getUniquePairs().size();

            case UrFaust::Dimension::two:
                return UrFaust::InitLattice(value.lx, value.ly, 0, 0)
                    .getUniquePairs().size();

            case UrFaust::Dimension::three:
                return UrFaust::InitLattice(
                    value.lx, value.ly, value.lz, 0, 0, 0
                ).getUniquePairs().size();

            default:
                throw std::invalid_argument("Unknown lattice dimension");
            }
        }

        static void validateLength(double value)
        {
            if (!std::isfinite(value) || value <= 0.0) {
                throw std::invalid_argument(
                    "Spin length must be finite and greater than zero"
                );
            }
        }

        static SimulationConfig validateConfig(
            SimulationConfig value)
        {
            // Íåàêòèâíûå ðàçìåðû ðàâíû åäèíèöå.
            // Àêòèâíûå ðàçìåðû >= 2 èñêëþ÷àþò ñâÿçè óçëà ñ ñîáîé.
            switch (value.dimension) {
            case UrFaust::Dimension::one:
                if (value.lx < 2) {
                    throw std::invalid_argument(
                        "Lx must be at least 2"
                    );
                }

                value.ly = 1;
                value.lz = 1;
                break;

            case UrFaust::Dimension::two:
                if (value.lx < 2 || value.ly < 2) {
                    throw std::invalid_argument(
                        "Lx and Ly must be at least 2"
                    );
                }

                value.lz = 1;
                break;

            case UrFaust::Dimension::three:
                if (value.lx < 2 ||
                    value.ly < 2 ||
                    value.lz < 2) {
                    throw std::invalid_argument(
                        "Lx, Ly and Lz must be at least 2"
                    );
                }
                break;

            default:
                throw std::invalid_argument(
                    "Unknown lattice dimension"
                );
            }

            const std::size_t n = spinCount(value);

            // Check neighbor-list size before allocating the lattice.
            checkedProduct(n, neighborCount(value.dimension));

            switch (value.spinLengthMode) {
            case InitIons::Ions::SpinLengthInit::constant: {
                const double* length =
                    std::get_if<double>(&value.spinLength);

                if (length == nullptr) {
                    throw std::invalid_argument(
                        "Constant spin length requires one number"
                    );
                }

                validateLength(*length);
                break;
            }

            case InitIons::Ions::SpinLengthInit::different: {
                const std::vector<double>* lengths =
                    std::get_if<std::vector<double>>(
                        &value.spinLength
                    );

                if (lengths == nullptr ||
                    lengths->size() != n) {
                    throw std::invalid_argument(
                        "Different spin lengths require one value per ion"
                    );
                }

                for (double length : *lengths) {
                    validateLength(length);
                }
                break;
            }

            default:
                throw std::invalid_argument(
                    "Unknown spin length mode"
                );
            }

            switch (value.exchangeMode) {
            case ExchangeIntegral::ExchangeIntegralInit::constant: {
                const double* exchange =
                    std::get_if<double>(
                        &value.exchangeIntegral
                    );

                if (exchange == nullptr ||
                    !std::isfinite(*exchange)) {
                    throw std::invalid_argument(
                        "Constant exchange requires one finite number"
                    );
                }
                break;
            }

            case ExchangeIntegral::ExchangeIntegralInit::matrix: {
                const std::vector<FM::Matrix>* matrices =
                    std::get_if<std::vector<FM::Matrix>>(
                        &value.exchangeIntegral
                    );

                if (matrices == nullptr ||
                    matrices->size() != uniquePairCount(value)) {
                    throw std::invalid_argument(
                        "Matrix exchange requires one matrix per unique ion pair"
                    );
                }

                // Ðàçìåð 3x3 è êîíå÷íîñòü ýëåìåíòîâ
                // ïðîâåðÿåò ñåòòåð ExchangeIntegral.
                break;
            }

            default:
                throw std::invalid_argument(
                    "Unknown exchange mode"
                );
            }

            switch (value.initialization) {
            case InitIons::Ions::SpinDirectionInit::parallel: {
                const double length =
                    Vector3D_Decart::norm(value.parallelDirection);

                if (!std::isfinite(length) ||
                    std::abs(length - 1.0) > 1.0e-12) {
                    throw std::invalid_argument(
                        "Parallel direction must be a finite unit vector");
                }

                break;
            }
            case InitIons::Ions::SpinDirectionInit::random:
                if (value.directionSeed.has_value() &&
                    *value.directionSeed < 0) {
                    throw std::invalid_argument(
                        "Direction seed must be nonnegative");
                }
            break;

            case InitIons::Ions::SpinDirectionInit::file:
                if (value.directionFile.empty()) {
                    throw std::invalid_argument(
                        "Spin directions filename is empty"
                    );
                }
                break;

            default:
                throw std::invalid_argument(
                    "Unknown spin direction initialization"
                );
            }

            if (!std::isfinite(value.epsilon) ||
                value.epsilon <= 0.0) {
                throw std::invalid_argument(
                    "Epsilon must be finite and greater than zero"
                );
            }

            if (value.maxSweeps == 0) {
                throw std::invalid_argument(
                    "Maximum iterations must be greater than zero"
                );
            }

            return value;
        }

        static void validateEnergy(double energy)
        {
            if (!std::isfinite(energy)) {
                throw std::runtime_error(
                    "Simulation energy must be finite"
                );
            }
        }

        SimulationFrame captureFrame(
            std::size_t sweep,
            double energy) const
        {
            validateEnergy(energy);

            return SimulationFrame{
                sweep,
                energy,
                spinSystem.getSpinDirections()
            };
        }


    public:
        explicit Simulation(SimulationConfig value)
            : config(validateConfig(std::move(value))),
            count(spinCount(config)),
            spinSystem(
                config.lx,
                config.ly,
                config.lz,
                0,
                0,
                0,
                config.spinLength,
                config.exchangeIntegral,
                config.spinLengthMode,
                config.initialization,
                config.exchangeMode,
                config.dimension,
                config.epsilon,
                config.directionFile,
                config.directionSeed
            )
        {
            {
                if (config.initialization ==
                    InitIons::Ions::SpinDirectionInit::parallel) {
                    spinSystem.setParallelDirection(config.parallelDirection);
                }
            }
        }

        const SimulationConfig& getConfig() const
        {
            return config;
        }

        std::size_t getSpinCount() const
        {
            return count;
        }

        double getEnergy() const
        {
            const double energy = spinSystem.energy();

            validateEnergy(energy);

            return energy;
        }

        std::vector<Vector3D_Decart::Vector3D_Dec>
            getSpinDirections() const
        {
            return spinSystem.getSpinDirections();
        }

        std::vector<double> getSpinLengths() const
        {
            if (const double* length =
                std::get_if<double>(
                    &config.spinLength
                )) {
                return std::vector<double>(
                    count,
                    *length
                );
            }

            return std::get<std::vector<double>>(
                config.spinLength
            );
        }

        SimulationResult run()
        {
            SimulationResult result;
            result.config = config;
            result.spinLengths = getSpinLengths();

            double previousEnergy = getEnergy();

            result.frames.push_back(
                captureFrame(0, previousEnergy)
            );

            for (std::size_t sweep = 0;
                sweep < config.maxSweeps;
                ++sweep) {

                spinSystem.relaxation_sweep();

                const double energy = getEnergy();
                result.completedSweeps = sweep + 1;

                result.frames.push_back(
                    captureFrame(result.completedSweeps, energy)
                );

                if (std::abs(energy - previousEnergy) < config.epsilon) {
                    result.converged = true;
                    break;
                }

                previousEnergy = energy;
            }

            return result;
        }
    };

};

