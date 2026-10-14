#pragma once

#include "UrFaustSimulation_1.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace UrFaustProgram {

    inline const char* dimensionName(
        UrFaust::Dimension dimension)
    {
        switch (dimension) {
        case UrFaust::Dimension::one:
            return "1D";

        case UrFaust::Dimension::two:
            return "2D";

        case UrFaust::Dimension::three:
            return "3D";

        default:
            throw std::invalid_argument("Unknown dimension");
        }
    }

    inline void runExample(
        UrFaust::Dimension dimension,
        std::uint64_t lx,
        std::uint64_t ly,
        std::uint64_t lz,
        const std::filesystem::path& outputRoot,
        const std::filesystem::path& templatePath)
    {
        UrFaustSim::SimulationConfig config;

        config.dimension = dimension;
        config.lx = lx;
        config.ly = ly;
        config.lz = lz;

        config.spinLength = 1.0;
        config.exchangeIntegral = -1.0;
        config.epsilon = 1.0e-10;
        config.maxSweeps = 1000;

        config.initialization =
            InitIons::Ions::SpinDirectionInit::random;

        config.outputDirectory =
            outputRoot / dimensionName(dimension);

        config.texTemplatePath = templatePath;
        config.texRowsPerPage = 30;

        config.exportSpinTxt = true;
        config.exportEnergyTxt = true;
        config.exportResultTex = true;

        UrFaustSim::Simulation simulation(config);

        const UrFaustSim::SimulationResult result =
            simulation.run();

        const auto& first = result.frames.front();
        const auto& last = result.frames.back();

        std::cout
            << std::setprecision(12)
            << "\n" << dimensionName(dimension)
            << " simulation\n"
            << "Lattice:        " << result.config.lx;

        if (dimension != UrFaust::Dimension::one) {
            std::cout << " x " << result.config.ly;
        }

        if (dimension == UrFaust::Dimension::three) {
            std::cout << " x " << result.config.lz;
        }

        std::cout
            << '\n'
            << "Spins:          "
            << first.spinDirections.size() << '\n'
            << "Initial energy: " << first.energy << '\n'
            << "Final energy:   " << last.energy << '\n'
            << "Sweeps:         "
            << result.completedSweeps << '\n'
            << "Frames:         "
            << result.frames.size() << '\n'
            << "Converged:      "
            << (result.converged ? "yes" : "no") << '\n'
            << "Output:         "
            << config.outputDirectory << '\n';
    }

    inline int run(
        const std::filesystem::path& projectDirectory)
    {
        try {
            const std::filesystem::path root =
                std::filesystem::absolute(projectDirectory);

            const std::filesystem::path templatePath =
                root / "Latex templates" / "UrFaustResult.tex";

            if (!std::filesystem::is_regular_file(templatePath)) {
                throw std::runtime_error(
                    "UrFaustResult.tex template is missing"
                );
            }

            const std::filesystem::path outputRoot =
                root / "Output";

            runExample(
                UrFaust::Dimension::one,
                8, 1, 1,
                outputRoot,
                templatePath
            );

            runExample(
                UrFaust::Dimension::two,
                5, 5, 1,
                outputRoot,
                templatePath
            );

            runExample(
                UrFaust::Dimension::three,
                4, 4, 4,
                outputRoot,
                templatePath
            );

            return 0;
        }
        catch (const std::exception& error) {
            std::cerr
                << "UrFaust failed: "
                << error.what() << '\n';

            return 1;
        }
    }

}
