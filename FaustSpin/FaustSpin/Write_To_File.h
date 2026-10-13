#pragma once

#include "Subprograms/Urfaust.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <ostream>
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

    // Заглушки до согласования формата TeX.
    inline void write_result_tex(
        UrFaust::Dimension,
        std::size_t,
        std::uint64_t,
        const Vector3D_Decart::Vector3D_Dec&)
    {
    }

   

}