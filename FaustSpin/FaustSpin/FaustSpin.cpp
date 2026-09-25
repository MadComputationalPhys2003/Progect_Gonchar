#include "UrFaustSimulation_1.h"




int main() {
    UrFaustPseudoInterface simulation(
        Dimension::one,

        4, 0, 0,      // Lx, Ly, Lz
        0, 0, 0,      // x0, y0, z0

        1.0,           // S
        -1.0,          // J
        1.0e-10,       // eps

        Initialization::parallel
    );

    std::cout
        << "Energy: "
        << simulation.energy()
        << '\n';
}