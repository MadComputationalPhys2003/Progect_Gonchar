#pragma once
#include "Lattice2D.h"
namespace Lattice2D_Neighbors_Open_BC {

	struct NeighborListCSR {
		std::vector<uint64_t> neighbors; // все соседи всех узлов подряд, без вложенности
		std::vector<size_t> offsets;     // offsets[i]..offsets[i+1] — диапазон для узла i, размер N+1
	};

	inline NeighborListCSR build_open_BC(const Lattice2D::Lattice2D_Data& ld) {
		NeighborListCSR result;
		size_t N = ld.get_N();
		result.offsets.reserve(N + 1);
		result.neighbors.reserve(N * 4); // верхняя оценка: не больше 4 соседей на узел в 2D
		result.offsets.push_back(0);

		int64_t Lx = static_cast<int64_t>(ld.get_Lx());
		int64_t Ly = static_cast<int64_t>(ld.get_Ly());

		for (int64_t x = 0; x < Lx; x++) {
			for (int64_t y = 0; y < Ly; y++) {
				if (x > 0)      result.neighbors.push_back(ld.get_index(x - 1, y));
				if (x < Lx - 1) result.neighbors.push_back(ld.get_index(x + 1, y));
				if (y > 0)      result.neighbors.push_back(ld.get_index(x, y - 1));
				if (y < Ly - 1) result.neighbors.push_back(ld.get_index(x, y + 1));
				result.offsets.push_back(result.neighbors.size());
			}
		}
		return result;
	}
	// доступ к соседям узла i без копирования: pair<указатель на первого соседа, число соседей>
	inline std::pair<const uint64_t*, size_t> get_neighbors(const NeighborListCSR& nl, size_t i) {
		size_t begin = nl.offsets[i];
		size_t count = nl.offsets[i + 1] - begin;
		return { nl.neighbors.data() + begin, count };
	}
}

inline std::vector<uint64_t> get_neighbors_periodic_BC(const Lattice2D::Lattice2D_Data& ld) {
	std::vector<uint64_t> neighbors;
	neighbors.resize(4 * ld.get_N());
	uint64_t lx = ld.get_Lx();
	uint64_t ly = ld.get_Ly();
	//int64_t x_new, y_new;
	size_t idx = 0;
	for (uint64_t x = 0; x < lx; x++) {

		for (uint64_t y = 0; y < ly; y++) {
			idx = 4 * (ly * x + y);
			if (x + 1 == lx) {
				neighbors[idx] = ld.get_index(0, y);
			}
			else{
				neighbors[idx] = ld.get_index(x + 1, y);
			}
			if (static_cast<int64_t> (x) - 1 < 0) {
				neighbors[idx + 1] = ld.get_index(lx - 1, y);
			}
			else{
				neighbors[idx + 1] = ld.get_index(x - 1, y);
			}
			if (y+1==ly){
				neighbors[idx + 2] = ld.get_index(x, 0);
			}
			else{
				neighbors[idx + 2] = ld.get_index(x, y + 1);
			}
			if (static_cast<int64_t> (y) - 1 < 0) {
				neighbors[idx + 3] = ld.get_index(x, ly - 1);
			}
			else{
				neighbors[idx + 3] = ld.get_index(x, y - 1);
			}
		}
	}
	return neighbors;
}