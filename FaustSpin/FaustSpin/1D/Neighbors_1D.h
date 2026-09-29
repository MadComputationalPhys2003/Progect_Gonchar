#pragma once
#include "Lattice1D.h"

namespace Neighbors1D {

	struct NeighborData {
		std::vector<uint64_t> indices;
		std::uint8_t count;
	};

	inline NeighborData get_neighbors_periodic_BC_1D(
		const Lattice1D::Lattice1D_Data& ld
	) {
		NeighborData nd;
		const uint64_t N = ld.get_N();
		nd.count = 2;
		if (N == 0) {
			return {};
		}
		nd.indices.resize(2 * N);

		for (uint64_t i = 0; i < N; ++i) {
			const uint64_t left =
				(i == 0) ? N - 1 : i - 1;

			const uint64_t right =
				(i == N - 1) ? 0 : i + 1;

			nd.indices[2 * i] = left;
			nd.indices[2 * i + 1] = right;
		}

		return nd;
	}
	inline std::vector<uint64_t> open_BC_1D(const Lattice1D::Lattice1D_Data& ld, uint64_t x) {
		uint64_t left;
		uint64_t right;
		std::vector<uint64_t> neighbors;
		neighbors.reserve(2);
		size_t idx = ld.get_index(x);
		if (idx != 0) {
			left = idx - 1;
			neighbors.push_back(left);
		}
		if (idx != ld.get_N() - 1) {
			right = idx + 1;
			neighbors.push_back(right);
		}
		return neighbors;
	}
}