#pragma once
#include "../FaustSpinSimulation.h"
#include"../FaustSpin.h"
#include "../Faust math/Vector3D_Decart.h"
#include<cmath>
#include <cstddef>
#include <vector>
#include <algorithm>


namespace Angless {
	inline std::vector<double> angles_between_ions_single_frame(
		const FaustSpinSim::SimulationResult& res, std::size_t frame_index) {
		std::vector<double> angles; //= res.exchangePairs;
		const auto& pairs = res.exchangePairs;
		const auto& directions = res.frames.at(frame_index).spinDirections;
		double cos_theta = 0.0;
		angles.reserve(pairs.size());
		for (std::size_t k = 0; k < pairs.size();k++) {
			const std::size_t i = pairs[k].first;
			const std::size_t j = pairs[k].second;
			cos_theta =Vector3D_Decart::scalar_product(directions[i],directions[j]);
			angles.push_back(std::acos(std::clamp(cos_theta, -1.0, 1.0)));
		}
		return angles;
	}
}
