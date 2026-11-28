#pragma once
#include"../FaustSpin.h"
#include"../FaustSpinSimulation.h"
#include"../Faust math/Vector3D_Decart.h"
#include"../Additional calculus/IonMagnetMoment.h"
#include"../Additional calculus/Angles.h"
#include<cmath>
#include <numbers>
#include <cstddef>
#include <vector>
#include <algorithm>

namespace Curves1 {
	inline std::vector<std::pair<double,double>> spin_direction_vs_B_direction(const FaustSpinSim::SimulationResult& res,
		std::size_t ion_index, std::size_t axis,std::size_t angle_flag,
		double angle_max,double angle_min,double other_angle,std::uint64_t N_points) {
		if (axis > 2) {
			throw std::invalid_argument("Error! Wrong direction number. 0->x,1->y,2->z");
		}
		if (angle_flag > 1) {
			throw std::invalid_argument("Error! angle_flag has to be 0 or 1");
		}
		auto config = res.config;
		double B = Vector3D_Decart::norm(config.magneticField);
		double step = std::abs(angle_max-angle_min) * 1.0 / (N_points - 1.0);
		double angle = angle_min;
		double& theta = angle_flag == 0 ? angle : other_angle;
		double& phi = angle_flag == 0 ? other_angle : angle;
		std::vector<std::pair<double,double>> svb_dir;
		svb_dir.resize(N_points + 1);
		config.zeemanEnabled = true;
		for (std::size_t i = 0; i < N_points; i++) {
			angle = angle_min + (1.0*i) * step;
			double x = std::sin(theta)*std::cos(phi) ;
			double y = std::sin(theta)*std::sin(phi);
			double z = std::cos(theta);
			Vector3D_Decart::Vector3D_Dec  unit_direction(x, y, z);
			config.magneticField = B * unit_direction;
			FaustSpinSim::Simulation simulation(config);
			const auto point_result = simulation.run();
			if (!point_result.converged) {
				throw std::runtime_error("Spin minimization did not converge");
			}
			svb_dir[i].first = angle;
			svb_dir[i].second = point_result.frames.back().spinDirections.at(ion_index)[axis];
		}
		return svb_dir;
	}
	inline std::vector<std::pair<double, double>> spin_direction_vs_B_module(const FaustSpinSim::SimulationResult& res,
		std::size_t ion_index, std::size_t axis, const FaustSpinSim::Vector3& field_direction,
		double B_max, double B_min, std::uint64_t N_points)
	{
		if (axis > 2) {
			throw std::invalid_argument("Error! Wrong direction number. 0->x,1->y,2->z");
		}
		double step = std::abs(B_min - B_max) * 1.0 / (N_points - 1.0);
		double B = B_min;
		std::vector<std::pair<double, double>> svb_abs;
		svb_abs.resize(N_points + 1);
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
		for (std::size_t i = 0; i < N_points; i++) {
			B = B_min + i * step;
			config.magneticField = B * unit_direction;
			FaustSpinSim::Simulation simulation(config);
			const auto point_result = simulation.run();
			if (!point_result.converged) {
				throw std::runtime_error("Spin minimization did not converge");
			}
			svb_abs[i].first = B;
			svb_abs[i].second = point_result.frames.back().spinDirections.at(ion_index)[axis];
		}
		return svb_abs;
	}

	inline std::vector<std::pair<double, double>>  total_magnetic_moment_direction_vs_B_direction(const FaustSpinSim::SimulationResult& res, std::size_t axis,
		std::size_t angle_flag,
		double angle_max, double angle_min, double other_angle, std::uint64_t N_points)
		{
		if (axis > 2) {
			throw std::invalid_argument("Error! Wrong direction number. 0->x,1->y,2->z");
		}
		if (angle_flag > 1) {
			throw std::invalid_argument("Error! angle_flag has to be 0 or 1");
		}
		auto config = res.config;
		double B = Vector3D_Decart::norm(config.magneticField);
		double step = angle_max - angle_min * 1.0 / (N_points - 1.0);
		double angle = angle_min;
		double& theta = angle_flag == 0 ? angle : other_angle;
		double& phi = angle_flag == 0 ? other_angle : angle;
		std::vector<std::pair<double, double>> tmmvbd;
		tmmvbd.resize(N_points + 1);
		config.zeemanEnabled = true;
		for (std::size_t i = 0; i < N_points; i++) {
			angle = angle_min + (1.0 * i) * step;
			double x = std::sin(theta) * std::cos(phi);
			double y = std::sin(theta) * std::sin(phi);
			double z = std::cos(theta);
			Vector3D_Decart::Vector3D_Dec  unit_direction(x, y, z);
			config.magneticField = B * unit_direction;
			FaustSpinSim::Simulation simulation(config);
			const auto point_result = simulation.run();
			if (!point_result.converged) {
				throw std::runtime_error("Spin minimization did not converge");
			}
			const auto& last_frame = point_result.frames.size() - 1;
			tmmvbd[i].first = angle;
			tmmvbd[i].second = IonMagneticMoment::total_magnetic_moment(point_result, last_frame)[axis];
		}
		return tmmvbd;
	}
	inline std::vector<std::pair<double, double>>total_magnetic_moment_direction_vs_B_module(const FaustSpinSim::SimulationResult& res, std::size_t axis,
		double B_max, double B_min, const FaustSpinSim::Vector3& field_direction, std::uint64_t N_points)
		{
		if (axis > 2) {
			throw std::invalid_argument("Error! Wrong direction number. 0->x,1->y,2->z");
		}
		double step = (B_max - B_min) * 1.0 / (N_points - 1.0);
		double B = B_min;
		std::vector<std::pair<double, double>> tmmdvb_abs;
		tmmdvb_abs.resize(N_points + 1);
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
			B = B_min + i * step;
			config.magneticField = B * unit_direction;
			FaustSpinSim::Simulation simulation(config);
			const auto point_result = simulation.run();
			if (!point_result.converged) {
				throw std::runtime_error("Spin minimization did not converge");
			}
			const auto& last_frame = point_result.frames.size() - 1;
			tmmdvb_abs[i].first = B;
			tmmdvb_abs[i].second =IonMagneticMoment::total_magnetic_moment(point_result,last_frame)[axis];
		}
		return tmmdvb_abs;
	}
	inline std::vector<std::pair<double, double>> total_magnetic_moment_module_vs_B_module(const FaustSpinSim::SimulationResult& res,
		double B_max, double B_min, std::uint64_t N_points, const FaustSpinSim::Vector3& field_direction) {
		double step = (B_max - B_min) * 1.0 / (N_points - 1.0);
		double B = B_min;
		std::vector<std::pair<double, double>> tmmdvb_abs_abs;
		tmmdvb_abs_abs.resize(N_points + 1);
		auto config = res.config;
		config.zeemanEnabled = true;
		const double norm_magnet = Vector3D_Decart::norm(field_direction);
		if (!std::isfinite(norm_magnet) || norm_magnet == 0.0) {
			throw std::invalid_argument("Field direction must be finite and nonzero");
		}
		const FaustSpinSim::Vector3 unit_direction_magnet{
			field_direction[0] / norm_magnet,
			field_direction[1] / norm_magnet,
			field_direction[2] / norm_magnet
		};
		for (std::size_t i = 0; i < N_points; i++) {
			B = B_min + i * step;
			config.magneticField = B * unit_direction_magnet;
			FaustSpinSim::Simulation simulation(config);
			const auto point_result = simulation.run();
			if (!point_result.converged) {
				throw std::runtime_error("Spin minimization did not converge");
			}
			const auto& last_frame = point_result.frames.size() - 1;
			tmmdvb_abs_abs[i].first = B;
			tmmdvb_abs_abs[i].second =Vector3D_Decart::norm(IonMagneticMoment::total_magnetic_moment(point_result, last_frame));
		}
		return tmmdvb_abs_abs;
	}
}