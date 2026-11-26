#pragma once
#include"../FaustSpin.h"
#include"../FaustSpinSimulation.h"
#include"../Faust math/Vector3D_Decart.h"
#include"../Additional calculus/IonMagnetMoment.h"
#include"../Additional calculus/Angles.h"
#include<cmath>
#include <cstddef>
#include <vector>
#include <algorithm>

namespace Curves1 {
	inline std::vector<std::pair<double,double>> spin_direction_vs_B(const FaustSpinSim::SimulationResult& res,
		std::size_t ion_index,std::size_t axis, const FaustSpinSim::Vector3& field_direction,
		 double B_max,double B_min,std::uint64_t N_points) {
		if (axis > 2) {
			throw std::invalid_argument("Error! Wrong direction number. 0->x,1->y,2->z");
		}
		double step = std::abs(B_min - B_max) * 1.0 / (N_points - 1.0);
		double B = B_min;
		std::vector<std::pair<double,double>> svb;
		svb.resize(N_points + 1);
		auto config = res.config;
		config.zeemanEnabled = true;
		const double norm = Vector3D_Decart::norm(field_direction);
		if (!std::isfinite(norm) || norm == 0.0) {
			throw std::invalid_argument("Field direction must be finite and nonzero");
		}

		const FaustSpinSim::Vector3 unit_direction{
			field_direction[0] / norm,
			field_direction[1] / norm,
			field_direction[2] / norm
		};
		for (std::size_t i = 0; i < N_points + 1; i++) {

			config.magneticField = B * unit_direction;
			FaustSpinSim::Simulation simulation(config);
			const auto point_result = simulation.run();
			if (!point_result.converged) {
				throw std::runtime_error("Spin minimization did not converge");
			}

			svb[i].first = B;
			svb[i].second = point_result.frames.back().spinDirections.at(ion_index)[axis];
			B = B + i * step;
			svb[i].first = B;
			svb[i].second = res.frames.back().spinDirections.at(ion_index)[axis];
		}
		return svb;
	}

}