#pragma once
#include "Lattice2D.h"

namespace Neighbors2D {
	struct NeighborData {
		std::vector<uint64_t> indices;
		std::uint8_t count;
	};

	inline NeighborData get_neighbors_periodic_BC_2D(const Lattice2D::Lattice2D_Data& ld) {
		NeighborData nd;
		nd.indices.resize(4 * ld.get_N());
		uint64_t lx = ld.get_Lx();
		uint64_t ly = ld.get_Ly();
		nd.count = 4;
		size_t idx = 0;
		for (uint64_t x = 0; x < lx; x++) {
			for (uint64_t y = 0; y < ly; y++) {
				idx = 4 * (ly * x + y);
				if (x + 1 == lx) {
					nd.indices[idx] = ld.get_index(0, y);
				}
				else {
					nd.indices[idx] = ld.get_index(x + 1, y);
				}
				if (static_cast<int64_t>(x) - 1 < 0) {//ондслюрэ
					nd.indices[idx + 1] = ld.get_index(lx - 1, y);
				}
				else {
					nd.indices[idx + 1] = ld.get_index(x - 1, y);
				}
				if (y + 1 == ly) {
					nd.indices[idx + 2] = ld.get_index(x, 0);
				}
				else {
					nd.indices[idx + 2] = ld.get_index(x, y + 1);
				}
				if (static_cast<int64_t> (y) - 1 < 0) {
					nd.indices[idx + 3] = ld.get_index(x, ly - 1);
				}
				else {
					nd.indices[idx + 3] = ld.get_index(x, y - 1);
				}
			}
		}
		return nd;
	}
}