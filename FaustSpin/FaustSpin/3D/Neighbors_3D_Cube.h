#pragma once
#include "Latice3D.h"

namespace Lattice3D_Neighbors {

	struct NeighborListCSR {
		std::vector<uint64_t> neighbors; // все соседи всех узлов подряд, без вложенности
		std::vector<size_t> offsets;     // offsets[i]..offsets[i+1] — диапазон для узла i, размер N+1
	};

	inline NeighborListCSR build_open_BC(const Lattice3D::Lattice3D_Data& ld) {
		NeighborListCSR result;
		size_t N = ld.get_N();
		result.offsets.reserve(N + 1);
		result.neighbors.reserve(N * 6); // верхняя оценка: не больше 6 соседей на узел в 3D
		result.offsets.push_back(0);

		int64_t Lx = static_cast<int64_t>(ld.get_Lx());
		int64_t Ly = static_cast<int64_t>(ld.get_Ly());
		int64_t Lz = static_cast<int64_t>(ld.get_Lz());

		for (int64_t x = 0; x < Lx; x++) {
			for (int64_t y = 0; y < Ly; y++) {
				for (int64_t z = 0; z < Lz; z++) {
					if (x > 0)      result.neighbors.push_back(ld.get_index(x - 1, y, z));
					if (x < Lx - 1) result.neighbors.push_back(ld.get_index(x + 1, y, z));
					if (y > 0)      result.neighbors.push_back(ld.get_index(x, y - 1, z));
					if (y < Ly - 1) result.neighbors.push_back(ld.get_index(x, y + 1, z));
					if (z > 0)      result.neighbors.push_back(ld.get_index(x, y, z - 1));
					if (z < Lz - 1) result.neighbors.push_back(ld.get_index(x, y, z + 1));
					result.offsets.push_back(result.neighbors.size());
				}
			}
		}
		return result;
	}

	inline std::pair<const uint64_t*, size_t> get_neighbors(const NeighborListCSR& nl, size_t i) {
		size_t begin = nl.offsets[i];
		size_t count = nl.offsets[i + 1] - begin;
		return { nl.neighbors.data() + begin, count };
	}
}


inline std::vector<uint64_t> get_neighbors_periodic_BC_3D(const Lattice3D::Lattice3D_Data& ld) {
	std::vector<uint64_t> neighbors;
	neighbors.resize(6 * ld.get_N());
	uint64_t lx = ld.get_Lx();
	uint64_t ly = ld.get_Ly();
	uint64_t lz = ld.get_Lz();
	size_t idx = 0;
	for (uint64_t x = 0; x < lx; x++) {
		for (uint64_t y = 0; y < ly; y++) {
			for (uint64_t z = 0; z < lz; z++) {
				idx = 6*(x * ly * lz + y * lz + z);
				if (x+1 == lx) {
					neighbors[idx] = ld.get_index(0, y, z);
				}
				else {
					neighbors[idx] = ld.get_index(x + 1, y, z);
				}
				if (static_cast<int64_t>(x) - 1 < 0) {
					neighbors[idx+1] = ld.get_index(lx - 1, y, z);
				}
				else {
					neighbors[idx+1] = ld.get_index(x - 1, y, z);
				}

				if (y+1 == ly) {
					neighbors[idx+2] = ld.get_index(x, 0, z);
				}
				else {
					neighbors[idx+2] = ld.get_index(x, y+1, z);
				}
				if (static_cast<int64_t>(y) - 1 < 0) {
					neighbors[idx+3] = ld.get_index(x, ly-1, z);
				}
				else {
					neighbors[idx+3] = ld.get_index(x, y-1, z);
				}

				if (z+1 == lz) {
					neighbors[idx+4] = ld.get_index(x, y, 0);
				}
				else {
					neighbors[idx+4] = ld.get_index(x, y, z+1);
				}
				if (static_cast<int64_t>(z) - 1 < 0) {
					neighbors[idx+5] = ld.get_index(x, y, lz-1);
				}
				else {
					neighbors[idx+5] = ld.get_index(x, y, z-1);
				}
			}
		}
	}
	return neighbors;
}