#include "UrFaustSimulation_1.h"

#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include<fstream>
#include <string_view>
#include <string>
#include <stdexcept>

namespace {

	[[nodiscard]] std::string_view dimensionName(UrFaust::Dimension dimension)
	{
		switch (dimension) {
		case UrFaust::Dimension::one:
			return "1D";
		case UrFaust::Dimension::two:
			return "2D";
		case UrFaust::Dimension::three:
			return "3D";
		default:
			return "unknown";
		}
	}

	void runExample(
		UrFaust::Dimension dimension,
		std::uint64_t lx,
		std::uint64_t ly,
		std::uint64_t lz)
	{
		UrFaustSim::SimulationConfig config;
		config.lx = lx;
		config.ly = ly;
		config.lz = lz;
		config.spinLength = 1.0;
		config.exchangeIntegral = -1.0;
		config.epsilon = 1.0e-10;
		config.maxSweeps = 1000;
		config.initialization = UrFaust::Ions::SpinInit::random;
		config.dimension = dimension;

		UrFaustSim::Simulation simulation(config);
		const UrFaustSim::SimulationResult result = simulation.run();
		std::ofstream spinFile(
			"spins_" + std::string(dimensionName(dimension)) + ".csv"
		);

		if (!spinFile) {
			throw std::runtime_error("Cannot create spins file.");
		}

		spinFile << std::setprecision(17);
		spinFile << "sweep,energy,index,nx,ny,nz\n";

		for (const auto& frame : result.frames) {
			for (std::size_t i = 0; i < frame.spinDirections.size(); ++i) {
				const auto& n = frame.spinDirections[i];

				spinFile << frame.sweep << ','
					<< frame.energy << ','
					<< i << ','
					<< n[0] << ','
					<< n[1] << ','
					<< n[2] << '\n';
			}
		}

		spinFile.close();

		if (!spinFile) {
			throw std::runtime_error("Cannot save spins file.");
		}
		std::ofstream file("energy.csv");

		file << "sweep,energy\n";
		file << std::setprecision(17);

		for (const auto& frame : result.frames) {
			file << frame.sweep << ',' << frame.energy << '\n';
		}
		const UrFaustSim::SimulationFrame& initialFrame = result.frames.front();
		const UrFaustSim::SimulationFrame& finalFrame = result.frames.back();

		std::cout
			<< "\n" << dimensionName(dimension) << " simulation\n"
			<< "Lattice:        " << lx;

		if (dimension != UrFaust::Dimension::one) {
			std::cout << " x " << ly;
		}
		if (dimension == UrFaust::Dimension::three) {
			std::cout << " x " << lz;
		}

		std::cout
			<< '\n'
			<< "Spins:          " << initialFrame.spinDirections.size() << '\n'
			<< "Initial energy: " << initialFrame.energy << '\n'
			<< "Final energy:   " << finalFrame.energy << '\n'
			<< "Sweeps:         " << result.completedSweeps << '\n'
			<< "Frames:         " << result.frames.size() << '\n'
			<< "Converged:      " << (result.converged ? "yes" : "no") << '\n';
	}

} // namespace

int main()
{
	try {
		std::cout << std::setprecision(12);

		runExample(UrFaust::Dimension::one, 8, 1, 1);
		runExample(UrFaust::Dimension::two, 5, 5, 1);
		runExample(UrFaust::Dimension::three, 4, 4, 4);

		return 0;
	}
	catch (const std::exception& exception) {
		std::cerr << "Simulation failed: " << exception.what() << '\n';
		return 1;
	}
}
