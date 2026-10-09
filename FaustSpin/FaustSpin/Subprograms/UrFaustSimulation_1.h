#pragma once

#include "Urfaust.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace UrFaustSim {

	using SpinDirection = Vector3D_Decart::Vector3D_Dec;

	struct SimulationConfig {
		std::uint64_t lx{ 1 };
		std::uint64_t ly{ 1 };
		std::uint64_t lz{ 1 };

		double spinLength{ 1.0 };
		double exchangeIntegral{ -1.0 };
		double epsilon{ 1.0e-10 };

		std::size_t maxSweeps{ 1000 };
		UrFaust::Ions::SpinInit initialization{
			UrFaust::Ions::SpinInit::parallel
		};
		UrFaust::Dimension dimension{ UrFaust::Dimension::one };
	};

	struct SimulationFrame {
		std::size_t sweep{};
		double energy{};
		std::vector<SpinDirection> spinDirections;
	};

	struct SimulationResult {
		SimulationConfig config;
		std::vector<SimulationFrame> frames;
		std::size_t completedSweeps{};
		bool converged{};
	};

	class Simulation {
	private:
		SimulationConfig config;
		UrFaust::SpinSystem spinSystem;

		static SimulationConfig validateConfig(SimulationConfig newConfig)
		{
			if (!std::isfinite(newConfig.spinLength) ||
				newConfig.spinLength <= 0.0) {
				throw std::invalid_argument(
					"Spin length must be finite and greater than zero"
				);
			}

			if (!std::isfinite(newConfig.exchangeIntegral)) {
				throw std::invalid_argument(
					"Exchange integral must be finite"
				);
			}

			if (!std::isfinite(newConfig.epsilon) ||
				newConfig.epsilon <= 0.0) {
				throw std::invalid_argument(
					"Epsilon must be finite and greater than zero"
				);
			}

			switch (newConfig.dimension) {
			case UrFaust::Dimension::one:
				if (newConfig.lx == 0) {
					throw std::invalid_argument(
						"Lx must be greater than zero for a 1D lattice"
					);
				}
				break;

			case UrFaust::Dimension::two:
				if (newConfig.lx == 0 || newConfig.ly == 0) {
					throw std::invalid_argument(
						"Lx and Ly must be greater than zero for a 2D lattice"
					);
				}
				break;

			case UrFaust::Dimension::three:
				if (newConfig.lx == 0 ||
					newConfig.ly == 0 ||
					newConfig.lz == 0) {
					throw std::invalid_argument(
						"Lx, Ly and Lz must be greater than zero for a 3D lattice"
					);
				}
				break;

			default:
				throw std::invalid_argument("Unknown lattice dimension");
			}

			if (newConfig.initialization !=
				UrFaust::Ions::SpinInit::parallel &&
				newConfig.initialization !=
				UrFaust::Ions::SpinInit::random) {
				throw std::invalid_argument("Unknown spin initialization mode");
			}

			return newConfig;
		}

		static void validateEnergy(double energy)
		{
			if (!std::isfinite(energy)) {
				throw std::runtime_error("Simulation energy must be finite");
			}
		}

		[[nodiscard]] SimulationFrame captureFrame(
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
		explicit Simulation(SimulationConfig newConfig)
			: config(validateConfig(std::move(newConfig))),
			spinSystem(
				config.lx,
				config.ly,
				config.lz,
				0, // x0
				0, // y0
				0, // z0
				config.spinLength,
				config.exchangeIntegral,
				config.epsilon,
				config.initialization,
				config.dimension)
		{
		}

		[[nodiscard]] const SimulationConfig& getConfig() const
		{
			return config;
		}

		[[nodiscard]] double getEnergy() const
		{
			const double currentEnergy = spinSystem.energy();
			validateEnergy(currentEnergy);
			return currentEnergy;
		}

		[[nodiscard]] std::vector<SpinDirection> getSpinDirections() const
		{
			return spinSystem.getSpinDirections();
		}

		[[nodiscard]] SimulationResult run()
		{
			SimulationResult result;
			result.config = config;

			double previousEnergy = getEnergy();
			result.frames.push_back(captureFrame(0, previousEnergy));

			for (std::size_t sweep = 0; sweep < config.maxSweeps; ++sweep) {
				spinSystem.relaxation_sweep();

				const double currentEnergy = getEnergy();
				result.completedSweeps = sweep + 1;
				result.frames.push_back(captureFrame(
					result.completedSweeps,
					currentEnergy
				));

				if (std::abs(currentEnergy - previousEnergy) < config.epsilon) {
					result.converged = true;
					break;
				}

				previousEnergy = currentEnergy;
			}

			return result;
		}
	};

} // namespace UrFaustSim
