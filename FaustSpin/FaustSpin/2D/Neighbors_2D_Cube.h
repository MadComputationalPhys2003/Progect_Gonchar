#pragma once
#include "Lattice2D.h"
std::vector<uint64_t> open_BC(const Lattice2D::Lattice2D_Data& ld, int64_t x, int64_t y) {
	uint64_t neighbor;
	std::vector<uint64_t> neighbors;
	neighbors.reserve(2);
	for (int64_t x = 0; x < ld.get_Lx(); x++) {
		for (int64_t y = 0; y < ld.get_Ly(); y++){
			size_t idx = ld.get_index(x,y);
			Cell2D cell = ld.get_cell(idx);
			if((x==0&&y==0)||(x==0&&y==ld.get_Ly()-1)||(x==ld.get_Lx()-1&&y==0)||(x==ld.get_Lx()-1&&y==ld.get_Ly()-1)){
				neighbors.push_back(2);
			}
			if(x==0&&y!=0&&y!=ld.get_Ly()-1){
				neighbors.push_back(3);
			}
			if(x==ld.get_Lx()-1&&y!=0&&y!=ld.get_Ly()-1){
				neighbors.push_back(3);
			}
			if(y==0&&x!=0&&x!=ld.get_Lx()-1){
				neighbors.push_back(3);
			}
			if(y==ld.get_Ly()-1&&x!=0&&x!=ld.get_Lx()-1){
				neighbors.push_back(3);
			}
			else {
				neighbors.push_back(4);
			}
		}
	}
	return neighbors;
}

