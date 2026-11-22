#include "FaustSpinSimulation.h"
#include "Write_To_File.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace {

    namespace fs = std::filesystem;

    const char* dimensionName(FaustSpin::Dimension dimension)
    {
        switch (dimension) {
        case FaustSpin::Dimension::one:   return "1D";
        case FaustSpin::Dimension::two:   return "2D";
        case FaustSpin::Dimension::three: return "3D";
        }
        throw std::invalid_argument("Unknown lattice dimension");
    }

    fs::path findTexTemplate()
    {
        const auto relativePath = fs::path("Latex templates") / "UrFaustResult.tex";
        const fs::path candidates[] = {
            relativePath,
            fs::path("FaustSpin") / relativePath,
            fs::path("FaustSpin") / "FaustSpin" / relativePath,
            fs::path(__FILE__).parent_path() / relativePath
        };
        for (const auto& path : candidates) {
            if (fs::is_regular_file(path)) {
                return path;
            }
        }
        return {};
    }

    void runExample(FaustSpin::Dimension dimension, std::uint64_t lx,
        std::uint64_t ly, std::uint64_t lz, const fs::path& templatePath)
    {
        FaustSpinSim::SimulationConfig config;
        config.dimension = dimension;
        config.lx = lx;
        config.ly = ly;
        config.lz = lz;
        config.spinLength = 1.0;
        config.initialization = FaustSpinSim::SpinDirectionMode::random;
        config.directionSeed = 42;

        config.exchangeEnabled = true;
        config.exchangeIntegral = -1.0;  // meV
        config.anisotropyEnabled = true;
        config.singleIonAnisotropy = 0.2;  // meV, easy plane perpendicular to z
        config.anisotropyDirection = FaustSpinSim::Vector3{ 0.0, 0.0, 1.0 };
        config.zeemanEnabled = true;
        config.gFactor = 2.0;
        config.magneticField = FaustSpinSim::Vector3{ 0.0, 0.0, 1.0 };  // T

        FaustSpinSim::Simulation simulation(config);
        const auto result = simulation.run();

        const char* name = dimensionName(dimension);
        const auto directory = fs::path("Output") / "FaustSpin" / name;
        WriteFileFaustSpinSimulation::write_all_txt(result, directory);
        if (!templatePath.empty()) {
            WriteFileFaustSpinSimulation::write_result_tex(
                result, templatePath, directory / "FaustSpinResult.tex");
        }

        std::cout << '\n'
            << "FaustSpin " << name << '\n'
            << "Lattice:        " << lx << " x " << ly << " x " << lz << '\n'
            << "Spins:          " << result.spinIndices.size() << '\n'
            << "Energy (meV):   " << result.frames.front().energy
            << " -> " << result.frames.back().energy << '\n'
            << "Sweeps:         " << result.completedSweeps << '\n'
            << "Converged:      " << (result.converged ? "yes" : "no") << '\n'
            << "Output:         " << directory << '\n';
    }

} // namespace

int main(int argc, char* argv[])
{
    if (argc > 2) {
        std::cerr << "Usage: FaustSpin [path-to-LaTeX-template]\n";
        return 2;
    }

    try {
        const auto templatePath = argc == 2 ? fs::path(argv[1]) : findTexTemplate();
        if (argc == 2 && !fs::is_regular_file(templatePath)) {
            std::cerr << "LaTeX template not found: " << templatePath << '\n';
            return 1;
        }
        if (templatePath.empty()) {
            std::cerr << "UrFaustResult.tex not found. Writing TXT results only.\n";
        }

        std::cout << std::setprecision(12);
        runExample(FaustSpin::Dimension::one, 8, 1, 1, templatePath);
        runExample(FaustSpin::Dimension::two, 5, 5, 1, templatePath);
        runExample(FaustSpin::Dimension::three, 4, 4, 4, templatePath);
    }
    catch (const std::exception& error) {
        std::cerr << "FaustSpin failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
