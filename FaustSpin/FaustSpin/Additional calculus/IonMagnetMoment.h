#pragma once
#include "../FaustSpinSimulation.h"
#include "../Faust math/Vector3D_Decart.h"
#include <cstddef>
#include <variant>
#include <vector>


namespace IonMagneticMoment {
	inline Vector3D_Decart::Vector3D_Dec ion_magnetic_moment(const FaustSpinSim::SimulationResult& res, std::size_t frame_index,
		std::size_t ion_index) { 
		Vector3D_Decart::Vector3D_Dec mag_moment(0.0, 0.0, 0.0);
		const double& bor = res.bohrMagneton;
		//auto sv=res.spinVector(frame_index, i)
		Vector3D_Decart::Vector3D_Dec spin = res.spinVector(frame_index, ion_index);
		if (std::get_if<double>(&res.config.gFactor) != nullptr) {
			double gFactor = std::get<double>(res.config.gFactor);
			mag_moment = bor * gFactor * spin;
		}
		else {
			const std::vector<FM::Matrix>& gFactor = std::get<std::vector<FM::Matrix>>(res.config.gFactor);
			mag_moment = bor*(gFactor.at(ion_index) * spin);
		}
		return mag_moment;
	}
	inline Vector3D_Decart::Vector3D_Dec total_magnetic_moment(const FaustSpinSim::SimulationResult& res,std::size_t frame_index) {
		std::size_t N = res.spinLengths.size();
		Vector3D_Decart::Vector3D_Dec tot_mag_moment(0.0, 0.0, 0.0);
		for (std::size_t i = 0; i < N; i++) {
			tot_mag_moment += ion_magnetic_moment(res, frame_index, i);
		}
		return tot_mag_moment;
	}
}