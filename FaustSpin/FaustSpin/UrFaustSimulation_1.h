#pragma once
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include "Urfaust.h"
namespace UrFaustSim {
    struct UrFaustPseudoInterface {
        UrFaust::SpinSystem SS;
        UrFaustPseudoInterface(
            uint64_t lx, uint64_t ly, uint64_t lz,
            uint64_t x_0, uint64_t y_0, uint64_t z_0,
            double s, double j, double e,
            UrFaust::Ions::SpinInit init,
            UrFaust::Dimension ddim
        ) : SS(lx, ly, lz, x_0, y_0, z_0, s, j, e, init, ddim)
        {

        }

    };
}
