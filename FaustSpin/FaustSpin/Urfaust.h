#pragma once
#include"Vector3D_Decart.h"
#include "1D/Lattice1D.h"
#include "2D/Lattice2D.h"
#include "3D/Latice3D.h"
#include"1D/Neighbors_1D.h"
#include"2D/Neighbors_2D_Cube.h"
#include"3D/Neighbors_3D_Cube.h"
#include<random>
#include <variant>
#include<numbers>
constexpr const double pi = std::numbers::pi;
namespace UrFaust3D {
    // Исправленный конструктор структуры InitLattice для UrFaust3D
    struct InitLattice {
		Lattice3D::Lattice3D_Data lattice;
		NeighborData neighbor_data;
        InitLattice(uint64_t lx, uint64_t ly, uint64_t lz,
            uint64_t x_0, uint64_t y_0, uint64_t z_0) :
            lattice(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(lx, ly, lz, x_0, y_0, z_0), Lattice3D_Setters::set_cube_cell)),
			neighbor_data(get_neighbors_periodic_BC_3D(lattice))
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
		void set_direction(
			const Vector3D_Decart::Vector3D_Dec& new_direction
		) {
			direction = new_direction;
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
		//Rework
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
	public:
		SpinSystem3D(uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0,
			double ss, double j,
			Ions::SpinInit mode) :
			il(lx, ly, lz, x0, y0, z0),ions(il.lattice.get_N(), mode), S(ss), J(j)
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
		void relaxation_sweep() {
			for (uint64_t i = 0; i < il.lattice.get_N(); i++) {
				Vector3D_Decart::Vector3D_Dec sum_vec(0, 0, 0);
				Vector3D_Decart::Vector3D_Dec h(0, 0, 0);
				double h_norm = 1.0;
				for (size_t a = 0; a < 6; ++a) {
					const size_t slot = 6 * i + a;
					const size_t j =
						static_cast<size_t>(il.neighbors[slot]);
					sum_vec += ions.ions[j].get_direction();
				}
				h = -S * S * J * sum_vec;
				h_norm = Vector3D_Decart::norm(h);
				if (h_norm > 1.0e-15) {
					ions.ions[i].set_direction(
						h * (1.0 / h_norm)
					);
				}
			}
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
			//neighbors(get_neighbors_periodic_BC_2D(lattice))
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

namespace UrFaust {
	enum class Dimention {
		one,
		two,
		three
	};
	struct InitLattice{
	private:
		Dimention dim;
		Lattice1D::Lattice1D_Data lattice1D;
		Lattice2D::Lattice2D_Data lattice2D;
		Lattice3D::Lattice3D_Data lattice3D;
		Neighbors1D::NeighborData ND1D;
		Neighbors2D::NeighborData ND2D;
		Neighbors3D::NeighborData ND3D;
	public:
		InitLattice(uint64_t lx,uint64_t x_0,Dimention d=Dimention::one) :
			dim(d),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(lx,x_0),Lattice1D_Setters::set_chain_cell)),
			ND1D(Neighbors1D::get_neighbors_periodic_BC_1D(lattice1D)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(1,1,1,1),Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(1,1,1,1,1,1),Lattice3D_Setters::set_cube_cell))
		{ 	
		}
		InitLattice(uint64_t lx, uint64_t ly, uint64_t x_0, uint64_t y_0, Dimention d = Dimention::two) :
			dim(d),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(1, 1), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(lx, ly, x_0, y_0), Lattice2D_Setters::set_square_cell)),
			ND2D(Neighbors2D::get_neighbors_periodic_BC_2D(lattice2D)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(1, 1, 1, 1,1,1), Lattice3D_Setters::set_cube_cell))
		{
		}
		InitLattice(uint64_t lx, uint64_t ly,uint64_t lz, uint64_t x_0, uint64_t y_0,uint64_t z_0, Dimention d=Dimention::three) :
			dim(d),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(1, 1), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(1, 1, 1, 1), Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(lx, ly, lz, x_0,y_0,z_0), Lattice3D_Setters::set_cube_cell))
		{
		}
	public:
		const Lattice1D::Lattice1D_Data& get_lattice1D( ) const {
			if (dim != Dimention::one) {
				throw std::invalid_argument("Wrong dimention!");
			}
			return lattice1D;
		}
		const Lattice2D::Lattice2D_Data& get_lattice2D() const {
			if (dim != Dimention::two) {
				throw std::invalid_argument("Wrong dimention!");
			}
			return lattice2D;
		}
		const Lattice3D::Lattice3D_Data& get_lattice3D() const{
			if (dim != Dimention::three) {
				throw std::invalid_argument("Wrong dimention!");
			}
			return lattice3D;
		}
		std::size_t getN() const {
			if (dim == Dimention::one) {
				return lattice1D.get_N();
			}
			if (dim == Dimention::two) {
				return lattice2D.get_N();
			}
			if (dim == Dimention::three) {
				return lattice3D.get_N();
			}
			return 0;// plug
		};
		const NeighborData& getNeighborData(size_t idx) const {
			if (dim == Dimention::one) {
				return NeighborData();
			}
		};
	};
	struct Ion {
	private:
		Vector3D_Decart::Vector3D_Dec direction;
	public:
		Ion(double x, double y, double z) : direction(x, y, z)
		{
			if (std::abs(Vector3D_Decart::norm(direction) - 1) > 1e-12) {
				throw std::invalid_argument("Ion's norma doesn't equal one");
			}
			if (!std::isfinite(Vector3D_Decart::norm(direction))) {
				throw std::runtime_error("Ion's norma must be finite");
			}
		}
		const Vector3D_Decart::Vector3D_Dec& get_direction() const {
			return direction;
		}
		double getPhi() const
		{
			return std::acos(
				std::clamp(direction[2], -1.0, 1.0)
			);
		}

		double getTheta() const
		{
			double theta = std::atan2(direction[1], direction[0]);

			if (theta < 0.0) {
				theta += 2.0 * pi;
			}

			return theta;
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
			double x, y, z,theta;
			double rho;
			for (size_t i = 0; i < ions.size(); i++) {
				z = dist_z(gen);
				theta = dist_angle(gen);
				rho = std::sqrt(1 - z * z);
				x = rho * std::cos(theta);
				y = rho * std::sin(theta);
				ions[i] = Ion(x, y, z);
			}
		}
	};
	class SpinSystem {
	private:
		InitLattice il;
		Ions ions;
		double S;
		double J;
		double eps;
		Dimention dim;
		static InitLattice makeInitLattice(Dimention dim, uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0) {
			if (dim == Dimention::one) {
				
				
			}
			if (dim == Dimention::two) {
				return InitLattice(lx,ly,x0,y0);
			}
			if (dim == Dimention::three) {
				return InitLattice(lx,ly,lz,x0,y0,z0);
			}
		}
	public:
		SpinSystem(uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0,
			double ss, double j,
			Ions::SpinInit mode,Dimention d):
			il(makeInitLattice(d,lx,ly,lz,x0,y0,z0)),
			ions(il.get_lattice1D().get_N()), S(ss), J(j)
		{
		
		}







	};
}
