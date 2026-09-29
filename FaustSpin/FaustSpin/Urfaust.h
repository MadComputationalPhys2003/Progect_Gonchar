#pragma once
#include"Vector3D_Decart.h"
#include"Lattice_Headers.h"
#include"Neighbor_Headers.h"
#include<random>
#include <variant>
#include<numbers>

constexpr const double pi = std::numbers::pi;

namespace UrFaust {
	enum class Dimension {
		one,
		two,
		three
	};
	struct NeighborInit{
	private:
		Dimension dim;
		Neighbors1D::NeighborData ND1D;
		Neighbors2D::NeighborData ND2D;
		Neighbors3D::NeighborData ND3D;
	public:
		NeighborInit(const Lattice1D::Lattice1D_Data& lattice):
			dim(Dimension::one), ND1D(Neighbors1D::get_neighbors_periodic_BC_1D(lattice))
		{}
		NeighborInit(const Lattice2D::Lattice2D_Data& lattice ) :
			dim(Dimension::two), ND2D(Neighbors2D::get_neighbors_periodic_BC_2D(lattice))
		{}
		NeighborInit(const Lattice3D::Lattice3D_Data& lattice):
			dim(Dimension::three), ND3D(Neighbors3D::get_neighbors_periodic_BC_3D(lattice))
		{}
		const std::vector<uint64_t>& getIndices() const {
			if (dim == Dimension::one) {
				return ND1D.indices;
			}
			if (dim == Dimension::two) {
				return ND2D.indices;
			}
			if (dim == Dimension::three) {
				return ND3D.indices;
			}
			throw std::logic_error("Unknown dimension");
		}
		const uint8_t& getCount() const {
			if (dim == Dimension::one) {
				return ND1D.count;
			}
			if (dim == Dimension::two) {
				return ND2D.count;
			}
			if (dim == Dimension::three) {
				return ND3D.count;
			}
			throw std::logic_error("Unknown dimension");
		}
	};
	struct InitLattice{
	private:
		Dimension dim;
		Lattice1D::Lattice1D_Data lattice1D;
		Lattice2D::Lattice2D_Data lattice2D;
		Lattice3D::Lattice3D_Data lattice3D;
		NeighborInit neighbors;
	public:
		InitLattice(uint64_t lx,uint64_t x_0) :
			dim(Dimension::one),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(lx,x_0),Lattice1D_Setters::set_chain_cell)),
			neighbors(lattice1D),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(1,1,1,1),Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(1,1,1,1,1,1),Lattice3D_Setters::set_cube_cell))
		{ 	
		}
		InitLattice(uint64_t lx, uint64_t ly, uint64_t x_0, uint64_t y_0) :
			dim(Dimension::two),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(1, 1), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(lx, ly, x_0, y_0), Lattice2D_Setters::set_square_cell)),
			neighbors(lattice2D),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(1, 1, 1, 1,1,1), Lattice3D_Setters::set_cube_cell))
		{
		}
		InitLattice(uint64_t lx, uint64_t ly,uint64_t lz, uint64_t x_0, uint64_t y_0,uint64_t z_0) :
			dim(Dimension::three),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(1, 1), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(1, 1, 1, 1), Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(lx, ly, lz, x_0,y_0,z_0), Lattice3D_Setters::set_cube_cell)),
			neighbors(lattice3D)
		{
		}
	public:
		const Lattice1D::Lattice1D_Data& get_lattice1D( ) const {
			if (dim != Dimension::one) {
				throw std::invalid_argument("Wrong dimension!");
			}
			return lattice1D;
		}
		const Lattice2D::Lattice2D_Data& get_lattice2D() const {
			if (dim != Dimension::two) {
				throw std::invalid_argument("Wrong dimension!");
			}
			return lattice2D;
		}
		const Lattice3D::Lattice3D_Data& get_lattice3D() const{
			if (dim != Dimension::three) {
				throw std::invalid_argument("Wrong dimension!");
			}
			return lattice3D;
		}
		const std::size_t& getN() const {
			if (dim == Dimension::one) {
				return lattice1D.get_N();
			}
			if (dim == Dimension::two) {
				return lattice2D.get_N();
			}
			if (dim == Dimension::three) {
				return lattice3D.get_N();
			}
			throw std::logic_error("Unknown dimension");
		};
		const std::vector<uint64_t>& getNeighborIndices() const
		{
			return neighbors.getIndices();
		}
		uint8_t getNeighborCount() const
		{
			return neighbors.getCount();
		}
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
		void set_direction(
			const Vector3D_Decart::Vector3D_Dec& new_direction
		) {
			const double newNorm = Vector3D_Decart::norm(new_direction);

			if (!std::isfinite(newNorm) ||
				std::abs(newNorm - 1.0) > 1.0e-12) {
				throw std::invalid_argument(
					"Ion direction must be a finite unit vector"
				);
			}

			direction = new_direction;
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
		std::vector<Vector3D_Decart::Vector3D_Dec> get_Directions() const {
			std::vector<Vector3D_Decart::Vector3D_Dec> res;
			for (size_t i = 0; i < ions.size(); i++) {
				res[i] = ions[i].get_direction();
			}
			return res;
		};
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
		static InitLattice makeInitLattice(Dimension dim, uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0) {
			if (dim == Dimension::one) {
				return InitLattice(lx, x0);
			}
			if (dim == Dimension::two) {
				return InitLattice(lx,ly,x0,y0);
			}
			if (dim == Dimension::three) {
				return InitLattice(lx,ly,lz,x0,y0,z0);
			}
			throw std::logic_error("Unknown dimension");
		}
	public:
		SpinSystem(uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0,
			double ss, double j, double ee,
			Ions::SpinInit mode,Dimension d):
			il(makeInitLattice(d,lx,ly,lz,x0,y0,z0)),
			ions(il.getN(),mode), S(ss), J(j),eps(ee)
		{
		}
		std::vector<Vector3D_Decart::Vector3D_Dec> getSpinDirections() const {
			std::vector<Vector3D_Decart::Vector3D_Dec> res;
			res = ions.get_Directions();
			return res;
		};
		double energy() const {
			double sum = 0.0;
			for (size_t i = 0; i < il.getN(); ++i) {
				for (size_t a = 0; a < il.getNeighborCount(); ++a) {
					const size_t slot = il.getNeighborCount() * i + a;
					const size_t j =
						static_cast<size_t>(il.getNeighborIndices()[slot]);
					sum += Vector3D_Decart::scalar_product(ions.ions[i].get_direction(),ions.ions[j].get_direction());
				}
			}
			return 0.5 * J * S * S * sum;
		}
		void relaxation_sweep() {
			for (uint64_t i = 0; i < il.getN(); i++) {
				Vector3D_Decart::Vector3D_Dec sum_vec(0, 0, 0);
				Vector3D_Decart::Vector3D_Dec h(0, 0, 0);
				double h_norm = 1.0;
				for (size_t a = 0; a < il.getNeighborCount(); ++a) {
					const size_t slot = il.getNeighborCount() * i + a;
					const size_t j =
						static_cast<size_t>(il.getNeighborIndices()[slot]);
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
