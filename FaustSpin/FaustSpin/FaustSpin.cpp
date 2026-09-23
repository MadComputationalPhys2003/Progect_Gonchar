#include "Urfaust.h"

#include <cmath>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>

int main()
{
    try {
        constexpr uint64_t Lx = 100;
        constexpr uint64_t Ly = 100;
        constexpr uint64_t Lz = 100;

        constexpr uint64_t x0 = 0;
        constexpr uint64_t y0 = 0;
        constexpr uint64_t z0 = 0;

        constexpr double S = 1.0;
        constexpr double J = -1.0;
        constexpr double eps = 1.0e-10;

        const double N =
            static_cast<double>(Lx) *
            static_cast<double>(Ly) *
            static_cast<double>(Lz);

        UrFaust3D::SpinSystem3D parallel_system(
            Lx, Ly, Lz,
            x0, y0, z0,
            S, J, eps,
            UrFaust3D::Ions::SpinInit::parallel
        );

        UrFaust3D::SpinSystem3D random_system(
            Lx, Ly, Lz,
            x0, y0, z0,
            S, J, eps,
            UrFaust3D::Ions::SpinInit::random
        );

        const double parallel_energy = parallel_system.energy();
        const double random_energy = random_system.energy();

        const double expected_parallel_energy =
            3.0 * N * J * S * S;

        const double comparison_tolerance =
            eps * (1.0 + std::abs(expected_parallel_energy));

        std::cout << std::setprecision(17);

        std::cout
            << "Number of ions: " << N << '\n'
            << "Parallel energy: " << parallel_energy << '\n'
            << "Expected energy: " << expected_parallel_energy << '\n'
            << "Random energy:   " << random_energy << '\n';

        if (std::abs(
            parallel_energy -
            expected_parallel_energy
        ) > comparison_tolerance) {

            std::cerr
                << "ERROR: parallel-state energy is incorrect.\n";

            return 1;
        }

        if (!std::isfinite(random_energy)) {
            std::cerr
                << "ERROR: random-state energy is not finite.\n";

            return 1;
        }

        const double energy_bound =
            3.0 * N * std::abs(J) * S * S;

        if (std::abs(random_energy) >
            energy_bound + comparison_tolerance) {

            std::cerr
                << "ERROR: random-state energy is outside "
                << "the theoretical bounds.\n";

            return 1;
        }

        std::cout << "All initialization and energy tests passed.\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr
            << "Exception: "
            << error.what()
            << '\n';

        return 1;
    }
}