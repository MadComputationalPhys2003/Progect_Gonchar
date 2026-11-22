#pragma once
//Ai generated! I hate writting funrions like these! Everything other in FaustSpin is not AI!
#include "Subprograms/Urfaust.h"
#include "FaustSpinSimulation.h"

#include <algorithm>
#include <array>
#include <string_view>
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

        const auto& pairs = lattice.getUniquePairs();

        if (matrices.size() != pairs.size()) {
            throw std::invalid_argument("Exchange matrix count mismatch");
        }

        for (std::size_t index = 0; index < matrices.size(); ++index) {
            const FM::Matrix& matrix = matrices[index];

            if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                throw std::invalid_argument("Exchange matrix must be 3x3");
            }

            output << "# Matrix " << index
                << ": i=" << pairs[index].first
                << " j=" << pairs[index].second << '\n';

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
namespace WriteFileFaustSpinSimulation {
    namespace detail {

        enum class Format { txt, tex };

        inline const char* dimension_name(FaustSpin::Dimension dimension)
        {
            switch (dimension) {
            case FaustSpin::Dimension::one:   return "1D";
            case FaustSpin::Dimension::two:   return "2D";
            case FaustSpin::Dimension::three: return "3D";
            }
            throw std::invalid_argument("Unknown FaustSpin dimension");
        }

        template <typename ResultType>
        inline void validate_result(const ResultType& result)
        {
            const std::size_t count = result.spinLengths.size();
            if (result.frames.empty() || count == 0) {
                throw std::invalid_argument("No FaustSpin results");
            }
            if (result.spinIndices.size() != count) {
                throw std::invalid_argument("Spin index and length counts differ");
            }
            for (const auto& frame : result.frames) {
                if (frame.spinDirections.size() != count) {
                    throw std::invalid_argument("Spin direction and length counts differ");
                }
            }

            const auto check = [](const auto& interaction, std::size_t expected) {
                if (const auto* matrices = std::get_if<std::vector<FM::Matrix>>(&interaction)) {
                    if (matrices->size() != expected) {
                        throw std::invalid_argument("Interaction matrix count mismatch");
                    }
                    for (const auto& matrix : *matrices) {
                        if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                            throw std::invalid_argument("Interaction matrix must be 3x3");
                        }
                    }
                }
                };

            check(result.config.exchangeIntegral, result.exchangePairs.size());
            check(result.config.singleIonAnisotropy, count);
            check(result.config.gFactor, count);
        }

        inline void write_number(std::ostream& output, double value, Format format)
        {
            if (format == Format::tex) {
                output << WriteFileUrFaust::tex_number(value);
            }
            else {
                output << value;
            }
        }

        template <typename IndexLabel>
        inline void write_interaction(
            std::ostream& output, const char* title, bool enabled,
            const FaustSpinSim::Interaction& interaction, const char* indexHeader,
            IndexLabel indexLabel, Format format)
        {
            const bool tex = format == Format::tex;
            const char* end = tex ? "\\par\n" : "\n";
            const auto* scalar = std::get_if<double>(&interaction);

            if (tex) {
                output << "\n\\par\\medskip\\noindent\\textbf{" << title << "}\\par\n";
            }
            else {
                output << "\n# " << title << '\n';
            }

            output << (enabled ? "Enabled" : "Disabled")
                << ", " << (scalar ? "constant" : "matrix") << " mode" << end;

            if (scalar) {
                output << "Value: ";
                write_number(output, *scalar, format);
                output << end;
                return;
            }

            const auto& matrices = std::get<std::vector<FM::Matrix>>(interaction);
            const char* separator = tex ? " & " : " ";
            const char* rowEnd = tex ? " \\\\ \\hline\n" : "\n";

            if (tex) {
                output << "\\begin{longtable}{|c|c|r|r|r|}\n\\hline\n";
            }
            else {
                output << "# ";
            }
            output << indexHeader << separator << "Row"
                << separator << "Col0" << separator << "Col1" << separator << "Col2"
                << rowEnd;
            if (tex) {
                output << "\\endhead\n";
            }

            for (std::size_t k = 0; k < matrices.size(); ++k) {
                const auto index = indexLabel(k);
                for (std::size_t row = 0; row < 3; ++row) {
                    output << index << separator << row;
                    for (std::size_t column = 0; column < 3; ++column) {
                        output << separator;
                        write_number(output, matrices[k][3 * row + column], format);
                    }
                    output << rowEnd;
                }
            }
            if (tex) {
                output << "\\end{longtable}\n";
            }
        }

        template <typename ResultType>
        inline void write_parameters(
            std::ostream& output, const ResultType& result, Format format)
        {
            const auto& config = result.config;
            const bool tex = format == Format::tex;
            const char* end = tex ? "\\par\n" : "\n";

            output << "Dimension: " << dimension_name(config.dimension) << end;
            if (tex) {
                output << "\\[\n"
                    << "L_x = " << config.lx << ",\\qquad "
                    << "L_y = " << config.ly << ",\\qquad "
                    << "L_z = " << config.lz << "\n\\]\n";
            }
            else {
                output << "Lx Ly Lz: " << config.lx << ' ' << config.ly
                    << ' ' << config.lz << '\n';
            }

            output << "Spin length mode: "
                << WriteFileUrFaust::spin_length_mode_name(config.spinLengthMode) << end
                << "Spin direction mode: "
                << WriteFileUrFaust::spin_direction_mode_name(config.initialization) << end
                << "Energy unit: meV" << end
                << "Completed sweeps: " << result.completedSweeps << end
                << "Converged: " << (result.converged ? "yes" : "no") << end;

            const auto number = [&](const char* label, double value) {
                output << label;
                write_number(output, value, format);
                output << end;
                };
            const auto vector = [&](const char* label, const FaustSpinSim::Vector3& value) {
                output << label << '(';
                write_number(output, value[0], format);
                output << ", ";
                write_number(output, value[1], format);
                output << ", ";
                write_number(output, value[2], format);
                output << ')' << end;
                };

            number("Energy tolerance (meV): ", config.epsilon);
            number("Direction tolerance: ", config.directionTolerance);
            number("Bohr magneton (meV/T): ", result.bohrMagneton);
            vector("Magnetic field B (T): ", config.magneticField);

            if (std::holds_alternative<double>(config.singleIonAnisotropy)) {
                vector("Anisotropy axis: ", config.anisotropyDirection.value_or(
                    FaustSpinSim::Vector3{ 0.0, 0.0, 1.0 }));
            }

            const auto pairIndex = [&](std::size_t k) {
                const auto& [i, j] = result.exchangePairs[k];
                return "(" + std::to_string(i) + "," + std::to_string(j) + ")";
                };
            const auto ionIndex = [&](std::size_t k) {
                return result.spinIndices[k];
                };

            write_interaction(output, "Exchange J (meV)", config.exchangeEnabled,
                config.exchangeIntegral, "Pair (i,j)", pairIndex, format);
            write_interaction(output, "Single-ion anisotropy D (meV)", config.anisotropyEnabled,
                config.singleIonAnisotropy, "Ion", ionIndex, format);
            write_interaction(output, "Zeeman g-factor or G tensor", config.zeemanEnabled,
                config.gFactor, "Ion", ionIndex, format);
        }

    } // namespace detail

    template <typename ResultType>
    inline void write_all_txt(
        const ResultType& result, const std::filesystem::path& directory)
    {
        detail::validate_result(result);

        auto spins = WriteFileUrFaust::open_txt_file(directory / "spin.txt");
        auto energy = WriteFileUrFaust::open_txt_file(directory / "energy.txt");
        auto parameters = WriteFileUrFaust::open_txt_file(directory / "parameters.txt");

        spins << "Iteration Index SpinLength nx ny nz\n";
        energy << "Iteration Energy_meV\n";

        for (const auto& frame : result.frames) {
            for (std::size_t i = 0; i < result.spinIndices.size(); ++i) {
                WriteFileUrFaust::write_spin_txt(
                    spins, result.spinIndices[i], frame.sweep,
                    result.spinLengths[i], frame.spinDirections[i]);
            }
            WriteFileUrFaust::write_energy_txt(energy, frame.sweep, frame.energy);
        }
        detail::write_parameters(parameters, result, detail::Format::txt);

        spins.close();
        energy.close();
        parameters.close();
    }

    template <typename ResultType>
    inline void write_result_tex(
        const ResultType& result, const std::filesystem::path& templatePath,
        const std::filesystem::path& outputPath, std::size_t rowsPerPage = 30)
    {
        detail::validate_result(result);

        std::ifstream input(templatePath, std::ios::binary);
        if (!input) {
            throw std::runtime_error("Cannot open TeX template");
        }
        if (std::filesystem::exists(outputPath) &&
            std::filesystem::equivalent(templatePath, outputPath)) {
            throw std::invalid_argument("TeX template and output must be different files");
        }

        constexpr std::array<std::string_view, 3> markers{
            "% URFAUST_PARAMETERS",
            "% URFAUST_SPIN_ROWS",
            "% URFAUST_ENERGY_ROWS"
        };
        std::array<std::size_t, markers.size()> positions{};
        std::vector<std::string> lines;
        std::size_t stage = 0;

        for (std::string line; std::getline(input, line);) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            const auto first = line.find_first_not_of(" \t");
            const auto last = line.find_last_not_of(" \t");
            if (first != std::string::npos) {
                const auto text = std::string_view(line).substr(first, last - first + 1);
                if (std::find(markers.begin(), markers.end(), text) != markers.end()) {
                    if (stage == markers.size() || text != markers[stage]) {
                        throw std::runtime_error("Wrong TeX template marker order");
                    }
                    positions[stage++] = lines.size();
                }
            }
            lines.push_back(std::move(line));
        }
        if (input.bad() || !input.eof()) {
            throw std::runtime_error("Cannot read TeX template");
        }
        if (stage != markers.size()) {
            throw std::runtime_error("TeX template must contain all three URFAUST markers");
        }
        input.close();

        auto output = WriteFileUrFaust::open_txt_file(outputPath);
        const auto pageBreak = [&](std::size_t& rows) {
            if (rowsPerPage != 0 && rows == rowsPerPage) {
                output << "\\pagebreak\n";
                rows = 0;
            }
            ++rows;
            };

        for (std::size_t line = 0; line < lines.size(); ++line) {
            if (line == positions[0]) {
                detail::write_parameters(output, result, detail::Format::tex);
            }
            else if (line == positions[1]) {
                std::size_t rows = 0;
                for (const auto& frame : result.frames) {
                    for (std::size_t i = 0; i < result.spinIndices.size(); ++i) {
                        pageBreak(rows);
                        const auto& n = frame.spinDirections[i];
                        output << frame.sweep << " & " << result.spinIndices[i] << " & "
                            << WriteFileUrFaust::tex_number(result.spinLengths[i]) << " & "
                            << WriteFileUrFaust::tex_number(n[0]) << " & "
                            << WriteFileUrFaust::tex_number(n[1]) << " & "
                            << WriteFileUrFaust::tex_number(n[2]) << " \\\\ \\hline\n";
                    }
                }
            }
            else if (line == positions[2]) {
                std::size_t rows = 0;
                for (const auto& frame : result.frames) {
                    pageBreak(rows);
                    output << frame.sweep << " & " << WriteFileUrFaust::tex_number(frame.energy)
                        << " \\\\ \\hline\n";
                }
            }
            else {
                output << lines[line] << '\n';
            }
        }
        output.close();
    }

} // namespace WriteFileFaustSpinSimulation
