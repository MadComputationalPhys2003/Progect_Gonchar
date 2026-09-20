#pragma once
#include "Lattice1D.h"
std::array<uint64_t, 2> periodic_BC_1D(const Lattice1D::Lattice1D_Data& ld, uint64_t x) {
	size_t idx = ld.get_index(x);
	uint64_t left;
	uint64_t right;
	if (idx == 0) {
		left = ld.get_N() - 1;
	}
	else {
		left = idx - 1;
	}
	if (idx == ld.get_N() - 1) {
		right = 0;
	}
	else {
		right = idx + 1;
	}
	std::array<uint64_t, 2> neighbors = { left,right };
	return neighbors;
}
std::vector<uint64_t> open_BC_1D(const Lattice1D::Lattice1D_Data& ld, uint64_t x) {
	uint64_t left;
	uint64_t right;
	std::vector<uint64_t> neighbors;
	neighbors.reserve(2);
	size_t idx = ld.get_index(x);
	if (idx != 0) {
		left = idx-1;
		neighbors.push_back(left);
	}
	if (idx != ld.get_N() - 1) {
		right = idx+1;
		neighbors.push_back(right);
	}
	return neighbors;
}
