#pragma once
#include "UrFaustSimulation_1.h"
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace WriteFileUrFaust {
	inline void write_spin_txt(UrFaust::Dimension dim, size_t idx, uint64_t iter,const Vector3D_Decart::Vector3D_Dec& dec);
	inline void write_spin_tex(UrFaust::Dimension dim, size_t idx, uint64_t iter, const Vector3D_Decart::Vector3D_Dec& dec);
	inline void write_energy_txt(UrFaust::Dimension dim, size_t idx, uint64_t iter,double energy);
	inline void write_energy_tex(UrFaust::Dimension dim, size_t idx, uint64_t iter,double energy);
}

