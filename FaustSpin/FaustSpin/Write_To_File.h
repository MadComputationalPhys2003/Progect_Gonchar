#pragma once
//Ai generated! I hate writting funrions like these! Everything other in FaustSpin is not AI!
#include "Subprograms/Urfaust.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <ostream>
#include <cmath>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

namespace WriteFileUrFaust {

    inline const char* dimension_name(UrFaust::Dimension mode)
    {
        switch (mode) {
        case UrFaust::Dimension::one:   return "1D";
        case UrFaust::Dimension::two:   return "2D";
        case UrFaust::Dimension::three: return "3D";
        default:
            throw std::invalid_argument("Unknown dimension");
        }
    }

    inline const char* spin_length_mode_name(
        InitIons::Ions::SpinLengthInit mode)
    {
        switch (mode) {
        case InitIons::Ions::SpinLengthInit::constant:
            return "constant";

        case InitIons::Ions::SpinLengthInit::different:
            return "different";

        default:
            throw std::invalid_argument("Unknown spin length mode");
        }
    }

    inline const char* spin_direction_mode_name(
        InitIons::Ions::SpinDirectionInit mode)
    {
        switch (mode) {
        case InitIons::Ions::SpinDirectionInit::parallel:
            return "parallel";

        case InitIons::Ions::SpinDirectionInit::random:
            return "random";

        case InitIons::Ions::SpinDirectionInit::file:
            return "file";

        default:
            throw std::invalid_argument("Unknown spin direction mode");
        }
    }

    inline const char* exchange_mode_name(
        ExchangeIntegral::ExchangeIntegralInit mode)
    {
        switch (mode) {
        case ExchangeIntegral::ExchangeIntegralInit::constant:
            return "constant";

        case ExchangeIntegral::ExchangeIntegralInit::matrix:
            return "matrix";

        default:
            throw std::invalid_argument("Unknown exchange mode");
        }
    }

    inline std::ofstream open_txt_file(
        const std::filesystem::path& filename)
    {
        if (!filename.parent_path().empty()) {
            std::filesystem::create_directories(
                filename.parent_path()
            );
        }

        std::ofstream output;
        output.exceptions(std::ios::failbit | std::ios::badbit);
        output.open(filename, std::ios::out | std::ios::trunc);
        output.imbue(std::locale::classic());

        output << std::setprecision(
            std::numeric_limits<double>::max_digits10
        );

        return output;
    }

    inline void write_parameters_txt(
        std::ostream& output,
        UrFaust::Dimension dimension,
        std::uint64_t lx,
        std::uint64_t ly,
        std::uint64_t lz,
        InitIons::Ions::SpinLengthInit spinLengthMode,
        InitIons::Ions::SpinDirectionInit spinDirectionMode,
        ExchangeIntegral::ExchangeIntegralInit exchangeMode,
        double epsilon)
    {
        output
            << "#dimension: "
            << dimension_name(dimension) << '\n'

            << "#Lx,Ly,Lz: "
            << lx << ' ' << ly << ' ' << lz << '\n'

            << "#Spin length mode: "
            << spin_length_mode_name(spinLengthMode) << '\n'

            << "#Spin direction mode: "
            << spin_direction_mode_name(spinDirectionMode) << '\n'

            << "#Exchange Integral mode: "
            << exchange_mode_name(exchangeMode) << '\n'

            << "#Energy Error: "
            << epsilon << "\n\n";
    }

    inline void write_spin_txt(
        std::ostream& output,
        std::size_t index,
        std::size_t iteration,
        double spinLength,
        const Vector3D_Decart::Vector3D_Dec& direction)
    {
        output
            << iteration << ' '
            << index << ' '
            << spinLength << ' '
            << direction[0] << ' '
            << direction[1] << ' '
            << direction[2] << '\n';
    }

    inline void write_energy_txt(
        std::ostream& output,
        std::size_t iteration,
        double energy)
    {
        output
            << iteration << ' '
            << energy << '\n';
    }

    inline std::string tex_number(double value)
    {
        if (!std::isfinite(value)) {
            throw std::invalid_argument(
                "Cannot export a non-finite number"
            );
        }

        std::ostringstream text;
        text.imbue(std::locale::classic());
        text << std::setprecision(6) << value;

        const std::string number = text.str();
        const std::size_t position = number.find_first_of("eE");

        if (position == std::string::npos) {
            return number;
        }

        return "$" + number.substr(0, position) +
            "\\times 10^{" +
            std::to_string(std::stoi(number.substr(position + 1))) +
            "}$";
    }

    template <typename ResultType>
    inline void write_result_parameters_txt(
        std::ostream& output,
        const ResultType& result)
    {
        const auto& config = result.config;

        write_parameters_txt(
            output, config.dimension,
            config.lx, config.ly, config.lz,
            config.spinLengthMode,
            config.initialization,
            config.exchangeMode,
            config.epsilon
        );
    }

    template <typename ResultType>
    inline void write_exchange_txt(
        std::ostream& output,
        const ResultType& result)
    {
        const auto& config = result.config;

        output.imbue(std::locale::classic());
        output << std::setprecision(
            std::numeric_limits<double>::max_digits10
        );

        write_result_parameters_txt(output, result);

        if (config.exchangeMode ==
            ExchangeIntegral::ExchangeIntegralInit::constant) {
            const double value =
                std::get<double>(config.exchangeIntegral);

            output << "# Constant isotropic exchange: J = "
                << value << "\n# Matrix J*I:\n";

            for (std::size_t row = 0; row < 3; ++row) {
                for (std::size_t column = 0; column < 3; ++column) {
                    if (column != 0) {
                        output << ' ';
                    }

                    output << std::setw(24)
                        << (row == column ? value : 0.0);
                }
                output << '\n';
            }
            return;
        }

        if (config.exchangeMode !=
            ExchangeIntegral::ExchangeIntegralInit::matrix) {
            throw std::invalid_argument("Unknown exchange mode");
        }

        const auto& matrices =
            std::get<std::vector<FM::Matrix>>(
                config.exchangeIntegral
            );

        const UrFaust::InitLattice lattice = [&config]() {
            switch (config.dimension) {
            case UrFaust::Dimension::one:
                return UrFaust::InitLattice(config.lx, 0);

            case UrFaust::Dimension::two:
                return UrFaust::InitLattice(
                    config.lx, config.ly, 0, 0);

            case UrFaust::Dimension::three:
                return UrFaust::InitLattice(
                    config.lx, config.ly, config.lz, 0, 0, 0);

            default:
                throw std::invalid_argument("Unknown dimension");
            }
            }();

        const auto& neighbors = lattice.getNeighborIndices();
        const std::size_t neighborCount = lattice.getNeighborCount();

        if (matrices.size() != neighbors.size()) {
            throw std::invalid_argument("Exchange matrix count mismatch");
        }

        for (std::size_t slot = 0; slot < matrices.size(); ++slot) {
            const FM::Matrix& matrix = matrices[slot];

            if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                throw std::invalid_argument("Exchange matrix must be 3x3");
            }

            output << "# Matrix " << slot
                << ": i=" << slot / neighborCount
                << " j=" << neighbors[slot]
                << " a=" << slot % neighborCount << '\n';

            for (std::size_t row = 0; row < 3; ++row) {
                for (std::size_t column = 0; column < 3; ++column) {
                    if (column != 0) {
                        output << ' ';
                    }

                    output << std::setw(24)
                        << matrix[3 * row + column];
                }
                output << '\n';
            }
            output << '\n';
        }
    }

    template <typename ResultType>
    inline void write_all_txt(
        const ResultType& result,
        const std::filesystem::path& directory)
    {
        if (result.frames.empty()) {
            throw std::invalid_argument("No simulation results");
        }

        for (const auto& frame : result.frames) {
            if (frame.spinDirections.size() != result.spinLengths.size()) {
                throw std::invalid_argument("Spin count mismatch");
            }
        }

        std::ofstream spinOutput =
            open_txt_file(directory / "spin.txt");

        std::ofstream energyOutput =
            open_txt_file(directory / "energy.txt");

        std::ofstream exchangeOutput =
            open_txt_file(directory / "exchange_integrals.txt");

        write_result_parameters_txt(spinOutput, result);
        write_result_parameters_txt(energyOutput, result);

        spinOutput << "Iteration index spinlength nx ny nz\n";
        energyOutput << "Iteration Energy\n";

        for (const auto& frame : result.frames) {
            for (std::size_t i = 0; i < frame.spinDirections.size(); ++i) {
                write_spin_txt(
                    spinOutput, i, frame.sweep,
                    result.spinLengths[i],
                    frame.spinDirections[i]
                );
            }

            write_energy_txt(
                energyOutput, frame.sweep, frame.energy
            );
        }

        write_exchange_txt(exchangeOutput, result);

        spinOutput.close();
        energyOutput.close();
        exchangeOutput.close();
    }
   

}
namespace WriteFileUrFaust {

    template <typename ResultType>
    inline void write_result_tex(
        const ResultType& result,
        const std::filesystem::path& templatePath,
        const std::filesystem::path& outputPath,
        std::size_t rowsPerPage = 30)
    {
        std::ifstream input(
            templatePath,
            std::ios::in | std::ios::binary
        );

        if (!input) {
            throw std::runtime_error("Cannot open TeX template");
        }

        input.exceptions(std::ios::badbit);

        if (std::filesystem::exists(outputPath) &&
            std::filesystem::equivalent(templatePath, outputPath)) {
            throw std::invalid_argument(
                "TeX template and output must be different files"
            );
        }

        for (const auto& frame : result.frames) {
            if (frame.spinDirections.size() != result.spinLengths.size()) {
                throw std::invalid_argument(
                    "Spin directions and lengths have different sizes"
                );
            }
        }

        if (!outputPath.parent_path().empty()) {
            std::filesystem::create_directories(
                outputPath.parent_path()
            );
        }

        std::ofstream output;
        output.exceptions(std::ios::failbit | std::ios::badbit);

        output.open(
            outputPath,
            std::ios::out | std::ios::trunc | std::ios::binary
        );

        output.imbue(std::locale::classic());

        const auto& config = result.config;
        std::string line;
        int stage = 0;

        while (std::getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            const std::size_t first =
                line.find_first_not_of(" \t");

            const std::size_t last =
                line.find_last_not_of(" \t");

            const std::string marker =
                first == std::string::npos
                ? std::string{}
            : line.substr(first, last - first + 1);

            if (marker == "% URFAUST_PARAMETERS") {
                if (stage != 0) {
                    throw std::runtime_error(
                        "Wrong TeX template marker order"
                    );
                }

                output
                    << "Dimension: "
                    << dimension_name(config.dimension)
                    << "\\par\n"

                    << "\\[\n"
                    << "L_x = " << config.lx << ",\\qquad "
                    << "L_y = " << config.ly << ",\\qquad "
                    << "L_z = " << config.lz
                    << "\n\\]\n"

                    << "Spin length mode: "
                    << spin_length_mode_name(config.spinLengthMode)
                    << "\\par\n"

                    << "Spin direction mode: "
                    << spin_direction_mode_name(config.initialization)
                    << "\\par\n"

                    << "Exchange Integral mode: "
                    << exchange_mode_name(config.exchangeMode)
                    << "\\par\n"

                    << "Energy Error: "
                    << tex_number(config.epsilon)
                    << "\\par\n";

                stage = 1;
            }
            else if (marker == "% URFAUST_SPIN_ROWS") {
                if (stage != 1) {
                    throw std::runtime_error(
                        "Wrong TeX template marker order"
                    );
                }

                std::size_t rows = 0;

                for (const auto& frame : result.frames) {
                    for (std::size_t i = 0;
                        i < frame.spinDirections.size();
                        ++i) {

                        if (rowsPerPage != 0 && rows == rowsPerPage) {
                            output << "\\pagebreak\n";
                            rows = 0;
                        }

                        const auto& n = frame.spinDirections[i];

                        output
                            << frame.sweep << " & "
                            << i << " & "
                            << tex_number(result.spinLengths[i]) << " & "
                            << tex_number(n[0]) << " & "
                            << tex_number(n[1]) << " & "
                            << tex_number(n[2])
                            << " \\\\ \\hline\n";

                        ++rows;
                    }
                }

                stage = 2;
            }
            else if (marker == "% URFAUST_ENERGY_ROWS") {
                if (stage != 2) {
                    throw std::runtime_error(
                        "Wrong TeX template marker order"
                    );
                }

                std::size_t rows = 0;

                for (const auto& frame : result.frames) {
                    if (rowsPerPage != 0 && rows == rowsPerPage) {
                        output << "\\pagebreak\n";
                        rows = 0;
                    }

                    output
                        << frame.sweep << " & "
                        << tex_number(frame.energy)
                        << " \\\\ \\hline\n";

                    ++rows;
                }

                stage = 3;
            }
            else {
                output << line << '\n';
            }
        }

        if (!input.eof()) {
            throw std::runtime_error("Cannot read TeX template");
        }

        if (stage != 3) {
            throw std::runtime_error(
                "TeX template must contain all three URFAUST markers"
            );
        }

        output.close();
    }

}