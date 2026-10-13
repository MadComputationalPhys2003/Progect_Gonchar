#pragma once
#include "../Faust math/Faust_Matrix.h"
#include "../Vector3D_Decart.h"
#include<random>
#include<vector>
#include<algorithm>
#include <variant>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include<numbers>

namespace InitIons {
	const double pi = std::numbers::pi;
	class Ion
	{
	private:
		Vector3D_Decart::Vector3D_Dec spin_direction;
		double S; // Spin length for one ion
	public:
		Ion(
			double x = 1.0,
			double y = 0.0,
			double z = 0.0,
			double s = 1.0
		)
			: spin_direction(x, y, z), S(s)
		{
			set_direction(spin_direction);
			set_ion_spin_length(s);
		}

		double get_ion_spin_length() const
		{
			return S;
		}

		void set_ion_spin_length(double s)
		{
			if (!std::isfinite(s) || s <= 0.0) {
				throw std::invalid_argument(
					"Spin length must be finite and greater than zero"
				);
			}

			S = s;
		}
		const Vector3D_Decart::Vector3D_Dec& get_direction() const {
			return spin_direction;
		};
		void set_direction(const Vector3D_Decart::Vector3D_Dec& new_direction) {
			const double newNorm = Vector3D_Decart::norm(new_direction);

			if (!std::isfinite(newNorm) ||
				std::abs(newNorm - 1.0) > 1.0e-12) {
				throw std::invalid_argument(
					"Ion direction must be a finite unit vector"
				);
			}

			spin_direction = new_direction;
		};
		double getTheta() const {
			return std::acos(std::clamp(spin_direction[2], -1.0, 1.0));
		};
		double getPhi() const {
			double phi = std::atan2(spin_direction[1], spin_direction[0]);
			if (phi < 0.0) {
				phi += 2.0 * pi;
			}
			return phi;
		};

	};

	class Ions
	{

	private:
		void random_direction_generator() {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dist_angle(0, 2 * pi);
			std::uniform_real_distribution<> dist_z(-1, 1);
			double x, y, z, theta;
			double rho;
			for (size_t i = 0; i < ions.size(); i++) {
				z = dist_z(gen);
				theta = dist_angle(gen);
				rho = std::sqrt(1 - z * z);
				x = rho * std::cos(theta);
				y = rho * std::sin(theta);
				ions[i].set_direction(
					Vector3D_Decart::Vector3D_Dec(x, y, z)
				);
			}
		};
		void import_direction_from_file(const std::string& filename)
		{
			std::ifstream input(filename);

			if (!input) {
				throw std::runtime_error(
					"Cannot open spin directions file"
				);
			}

			input.imbue(std::locale::classic());

			std::vector<Vector3D_Decart::Vector3D_Dec> directions;
			directions.reserve(ions.size());

			for (std::size_t i = 0; i < ions.size(); ++i) {
				double x, y, z;

				if (!(input >> x >> y >> z)) {
					throw std::runtime_error(
						"Cannot read direction for ion " +
						std::to_string(i)
					);
				}

				Vector3D_Decart::Vector3D_Dec direction(x, y, z);
				const double n = Vector3D_Decart::norm(direction);

				if (!std::isfinite(n) ||
					std::abs(n - 1.0) > 1.0e-12) {
					throw std::invalid_argument(
						"Direction must be a finite unit vector; ion " +
						std::to_string(i)
					);
				}

				directions.push_back(direction);
			}

			input >> std::ws;

			if (input.bad()) {
				throw std::runtime_error(
					"Cannot read spin directions file"
				);
			}

			if (!input.eof()) {
				throw std::runtime_error(
					"Spin directions file contains extra data"
				);
			}

			// Применяем направления после проверки всего файла.
			for (std::size_t i = 0; i < ions.size(); ++i) {
				ions[i].set_direction(directions[i]);
			}

			SI = SpinDirectionInit::file;
		}
	public:
		enum class SpinLengthInit { constant, different };
		enum class SpinDirectionInit { parallel, random, file };
		std::vector<Ion> ions;
		SpinDirectionInit SI;
		SpinLengthInit SL;
		std::string spinfile;
		std::string directionfile;
		std::variant<double, std::vector<double>> spin;// Spin for every ion

		std::variant<double, std::vector<double>> set_spin(std::variant<double, std::vector<double>> values, SpinLengthInit mode)
		{
			if (mode == SpinLengthInit::constant) {
				const double* value = std::get_if<double>(&values);

				if (value == nullptr) {
					throw std::invalid_argument(
						"Constant mode requires one number"
					);
				}

				//check_spin_length(*value);
			}
			else if (mode == SpinLengthInit::different) {
				const std::vector<double>* lengths =
					std::get_if<std::vector<double>>(&values);

				if (lengths == nullptr ||
					lengths->size() != ions.size()) {
					throw std::invalid_argument(
						"Different mode requires one length per ion"
					);
				}
			}
			else {
				throw std::invalid_argument(
					"Unknown spin length mode"
				);
			}

			// Копию для возврата готовим до изменения состояния.
			std::variant<double, std::vector<double>> result = values;

			spin = std::move(values);
			SL = mode;

			if (const double* value = std::get_if<double>(&spin)) {
				for (Ion& ion : ions) {
					ion.set_ion_spin_length(*value);
				}
			}
			else {
				const std::vector<double>& lengths =
					std::get<std::vector<double>>(spin);

				for (std::size_t i = 0; i < ions.size(); ++i) {
					ions[i].set_ion_spin_length(lengths[i]);
				}
			}

			return result;
		}

		Ions(size_t n,
			SpinLengthInit sl = SpinLengthInit::constant,
			SpinDirectionInit si = SpinDirectionInit::parallel,
			std::string df="") :
			ions(), SI(si), SL(sl), directionfile(df)
		{

			ions.resize(n, Ion());
			if (SL == SpinLengthInit::constant) {
				spin = 1.0;
			}
			else if (SL == SpinLengthInit::different) {
				std::vector<double> res;
				res.resize(n);
				for (size_t i = 0; i < n; i++) {
					res[i] = 1.0;
				}
				spin = res;
			}
			if (SI == SpinDirectionInit::random) {
				random_direction_generator();
			}
			if (SI == SpinDirectionInit::file) {
				import_direction_from_file(directionfile);
			}
			// SI==SpinDirectionInit::parallel uses default constructor for Ion
		};
		std::vector<Vector3D_Decart::Vector3D_Dec> get_Directions() const {
			std::vector<Vector3D_Decart::Vector3D_Dec> result;
			result.reserve(ions.size());
			for (const Ion& ion : ions) {
				result.push_back(ion.get_direction());
			}
			return result;
		};
		Vector3D_Decart::Vector3D_Dec get_direction(size_t index) const {
			if (index >= ions.size()) {
				throw std::out_of_range("Index out of range");
			}
			return ions[index].get_direction();
		};
		double get_spin_length(size_t index) const {
			if (index >= ions.size()) {
				throw std::out_of_range("Index out of range");
			}
			return ions[index].get_ion_spin_length();
		};
	};
}




