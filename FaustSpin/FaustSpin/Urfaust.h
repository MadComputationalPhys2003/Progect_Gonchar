#pragma once
#include"Vector3D_Decart.h"
#include "1D/Lattice1D.h"
#include "2D/Lattice2D.h"
#include "3D/Latice3D.h"
#include"1D/Neighbors_1D.h"
#include"2D/Neighbors_2D_Cube.h"
#include"3D/Neighbors_3D_Cube.h"
#include<random>
#include<numbers>
constexpr const double pi = std::numbers::pi;
namespace UrFaust3D {
	struct InitLattice {
		Lattice3D::Lattice3D_Data lattice;
		std::vector<uint64_t> neighbors;
		InitLattice(uint64_t lx,uint64_t ly,uint64_t lz,
			uint64_t x_0,uint64_t y_0,uint64_t z_0): 
			lattice(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(lx,ly,lz,x_0,y_0,z_0),Lattice3D_Setters::set_cube_cell)),
			neighbors(get_neighbors_periodic_BC_3D(lattice))
		{
		}
	};
	struct Ion{
	private:
		Vector3D_Decart::Vector3D_Dec direction;
	public:
		Ion(double x,double y, double z): direction(x,y,z)
		{ }
		const Vector3D_Decart::Vector3D_Dec& get_direction() const {
			return direction;
		}
	};
	class Ions{
	public:
		enum class SpinInit{
			parallel,
			random
		};
		std::vector<Ion> ions;
		SpinInit SI;
		Ions(size_t n, SpinInit si = SpinInit::parallel)
			:ions(),SI(si)
		{
			ions.resize(n,
				Ion(1.0, 0.0, 0.0)
			);
			if(SI==SpinInit::random) {
				direction_generator();
			}
		}
	private:
		void direction_generator() {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dist_angle(0, 2 * pi);
			std::uniform_real_distribution<> dist_z(-1, 1);
			double x, y, z,phi;
			double rho;
			for (size_t i = 0; i < ions.size(); i++) {
				z = dist_z(gen);
				phi = dist_angle(gen);
				rho = std::sqrt(1 - z * z);
				x = rho * std::cos(phi);
				y = rho * std::sin(phi);
				ions[i] = Ion(x, y, z);
			}
		}
	};
	class SpinSystem3D {
	private:
		InitLattice il;
		Ions ions;
		double S;
		double J;
		double eps;
	public:
		SpinSystem3D(uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0,
			double ss, double j,double e,
			Ions::SpinInit mode) :
			il(lx, ly, lz, x0, y0, z0),ions(il.lattice.get_N(), mode), S(ss), J(j),eps(e)
		{
		
		}
		double energy() const {
			double sum = 0.0;

			for (size_t i = 0; i < il.lattice.get_N(); ++i) {
				for (size_t a = 0; a < 6; ++a) {
					const size_t slot = 6 * i + a;
					const size_t j =
						static_cast<size_t>(il.neighbors[slot]);

					sum += Vector3D_Decart::scalar_product(
						ions.ions[i].get_direction(),
						ions.ions[j].get_direction()
					);
				}
			}

			return 0.5 * J * S * S * sum;
		}
	};
}
namespace UrFaust2D {
	struct InitLattice {
		Lattice2D::Lattice2D_Data lattice;
		std::vector<uint64_t> neighbors;
		InitLattice(uint64_t lx, uint64_t ly,
			uint64_t x_0, uint64_t y_0) :
			lattice(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(lx,ly,x_0,y_0),Lattice2D_Setters::set_square_cell)),
			neighbors(get_neighbors_periodic_BC_2D(lattice))
		{
		}
	};
	struct Ion {
	private:
		Vector3D_Decart::Vector3D_Dec direction;
	public:
		Ion(double x, double y, double z) : direction(x, y, z)
		{
		}
		const Vector3D_Decart::Vector3D_Dec& get_direction() const {
			return direction;
		}
	};
	class Ions {
	public:
		enum class SpinInit {
			parallel,
			random
		};
		std::vector<Ion> ions;
		SpinInit SI;
		Ions(size_t n, SpinInit si = SpinInit::parallel)
			:ions(), SI(si)
		{
			ions.resize(n,
				Ion(1.0, 0.0, 0.0)
			);
			if (SI == SpinInit::random) {
				direction_generator();
			}
		}
	private:
		void direction_generator() {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dist_angle(0, 2 * pi);
			std::uniform_real_distribution<> dist_z(-1, 1);
			double x, y, z, phi;
			double rho;
			for (size_t i = 0; i < ions.size(); i++) {
				z = dist_z(gen);
				phi = dist_angle(gen);
				rho = std::sqrt(1 - z * z);
				x = rho * std::cos(phi);
				y = rho * std::sin(phi);
				ions[i] = Ion(x, y, z);
			}
		}
	};
	class SpinSystem2D {
	private:
		InitLattice il;
		Ions ions;
		double S;
		double J;
		double eps;
	public:
		SpinSystem2D(uint64_t lx, uint64_t ly,
					 uint64_t x0, uint64_t y0,
			double ss, double j, double e,
			Ions::SpinInit mode) :
			il(lx, ly, x0, y0), ions(il.lattice.get_N(), mode), S(ss), J(j), eps(e)
		{

		}
		double energy() const {
			double sum = 0.0;

			for (size_t i = 0; i < il.lattice.get_N(); ++i) {
				for (size_t a = 0; a < 4; ++a) {
					const size_t slot = 4 * i + a;
					const size_t j =
						static_cast<size_t>(il.neighbors[slot]);

					sum += Vector3D_Decart::scalar_product(
						ions.ions[i].get_direction(),
						ions.ions[j].get_direction()
					);
				}
			}

			return 0.5 * J * S * S * sum;
		}
	};

}
namespace UrFaust1D {
	struct InitLattice {
		Lattice1D::Lattice1D_Data lattice;
		std::vector<uint64_t> neighbors;
		InitLattice(uint64_t lx,
					uint64_t x_0) :
			lattice(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(lx, x_0), Lattice1D_Setters::set_chain_cell)),
			neighbors(get_neighbors_periodic_BC_1D(lattice))
		{
		}
	};
	struct Ion {
	private:
		Vector3D_Decart::Vector3D_Dec direction;
	public:
		Ion(double x, double y, double z) : direction(x, y, z)
		{
		}
		const Vector3D_Decart::Vector3D_Dec& get_direction() const {
			return direction;
		}
	};
	class Ions {
	public:
		enum class SpinInit {
			parallel,
			random
		};
		std::vector<Ion> ions;
		SpinInit SI;
		Ions(size_t n, SpinInit si = SpinInit::parallel)
			:ions(), SI(si)
		{
			ions.resize(n,
				Ion(1.0, 0.0, 0.0)
			);
			if (SI == SpinInit::random) {
				direction_generator();
			}
		}
	private:
		void direction_generator() {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dist_angle(0, 2 * pi);
			std::uniform_real_distribution<> dist_z(-1, 1);
			double x, y, z, phi;
			double rho;
			for (size_t i = 0; i < ions.size(); i++) {
				z = dist_z(gen);
				phi = dist_angle(gen);
				rho = std::sqrt(1 - z * z);
				x = rho * std::cos(phi);
				y = rho * std::sin(phi);
				ions[i] = Ion(x, y, z);
			}
		}
	};
	class SpinSystem1D {
	private:
		InitLattice il;
		Ions ions;
		double S;
		double J;
		double eps;
	public:
		SpinSystem1D(uint64_t lx,
					 uint64_t x0,
			double ss, double j, double e,
			Ions::SpinInit mode) :
			il(lx, x0), ions(il.lattice.get_N(), mode), S(ss), J(j), eps(e)
		{

		}
		double energy() const {
			double sum = 0.0;

			for (size_t i = 0; i < il.lattice.get_N(); ++i) {
				for (size_t a = 0; a < 2; ++a) {
					const size_t slot = 2 * i + a;
					const size_t j =
						static_cast<size_t>(il.neighbors[slot]);

					sum += Vector3D_Decart::scalar_product(
						ions.ions[i].get_direction(),
						ions.ions[j].get_direction()
					);
				}
			}

			return 0.5 * J * S * S * sum;
		}
	};

}
