#pragma once
#include "Latice3D.h"

namespace Neighbors3D {
    struct NeighborData {
        std::vector<uint64_t> indices;
        std::uint8_t count{};
    };
    inline NeighborData get_neighbors_periodic_BC_3D(
        const Lattice3D::Lattice3D_Data& ld
    ) {
        NeighborData nd;

        nd.count = 6;

        nd.indices.resize(
            static_cast<size_t>(nd.count) *
            ld.get_N()
        );

        //auto& neighbors = nd.indices;

        const uint64_t lx = ld.get_Lx();
        const uint64_t ly = ld.get_Ly();
        const uint64_t lz = ld.get_Lz();

        for (uint64_t x = 0; x < lx; ++x) {
            for (uint64_t y = 0; y < ly; ++y) {
                for (uint64_t z = 0; z < lz; ++z) {

                    const size_t i =
                        ld.get_index(x, y, z);

                    const size_t idx =
                        static_cast<size_t>(nd.count) * i;

                    if (x + 1 == lx) {
                        nd.indices[idx] =
                            ld.get_index(0, y, z);
                    }
                    else {
                        nd.indices[idx] =
                            ld.get_index(x + 1, y, z);
                    }

                    if (x == 0) {
                        nd.indices[idx + 1] =
                            ld.get_index(lx - 1, y, z);
                    }
                    else {
                        nd.indices[idx + 1] =
                            ld.get_index(x - 1, y, z);
                    }

                    if (y + 1 == ly) {
                        nd.indices[idx + 2] =
                            ld.get_index(x, 0, z);
                    }
                    else {
                        nd.indices[idx + 2] =
                            ld.get_index(x, y + 1, z);
                    }

                    if (y == 0) {
                        nd.indices[idx + 3] =
                            ld.get_index(x, ly - 1, z);
                    }
                    else {
                        nd.indices[idx + 3] =
                            ld.get_index(x, y - 1, z);
                    }

                    if (z + 1 == lz) {
                        nd.indices[idx + 4] =
                            ld.get_index(x, y, 0);
                    }
                    else {
                        nd.indices[idx + 4] =
                            ld.get_index(x, y, z + 1);
                    }

                    if (z == 0) {
                        nd.indices[idx + 5] =
                            ld.get_index(x, y, lz - 1);
                    }
                    else {
                        nd.indices[idx + 5] =
                            ld.get_index(x, y, z - 1);
                    }
                }
            }
        }

        return nd;
    }
}