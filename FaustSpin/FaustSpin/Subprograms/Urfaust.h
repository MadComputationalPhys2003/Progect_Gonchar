#pragma once
#include"Vector3D_Decart.h"
#include"Lattice_Headers.h"
#include"Neighbor_Headers.h"
#include"Initialisation Headers/IonFaust.h"
#include"Initialisation Headers/Faust_Exchange_Integral.h"
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
	class SpinSystem {
	private:
		InitLattice il;
		InitIons::Ions ions;
		ExchangeIntegral exchangeIntegral;
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
		static InitIons::Ions makeInitIons(size_t n, InitIons::Ions::SpinLengthInit sli, InitIons::Ions::SpinDirectionInit sdi,
			std::string df) {
			if (sdi == InitIons::Ions::SpinDirectionInit::random ||
				sdi == InitIons::Ions::SpinDirectionInit::parallel) {
				return InitIons::Ions(n, sli, sdi);
			}
			else {
				return InitIons::Ions(n, sli, sdi, df);
			}
		}
	public:
		SpinSystem(uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0,
			std::variant<double, std::vector<double>> spin,
			std::variant<double, std::vector<FM::Matrix>> j,
			InitIons::Ions::SpinLengthInit sli,InitIons::Ions::SpinDirectionInit sdi,
			ExchangeIntegral::ExchangeIntegralInit eii,
			Dimension d,double ee,
			std::string df = " "):
			il(makeInitLattice(d,lx,ly,lz,x0,y0,z0)),
			ions(makeInitIons(il.getN(),sli,sdi,df)),exchangeIntegral(il.getNeighborCount(),il.getN(),eii), eps(ee)
		{
			ions.set_spin(std::move(spin), sli);
			exchangeIntegral.set_eexchange_integral(std::move(j), eii);
		}
		std::vector<Vector3D_Decart::Vector3D_Dec> getSpinDirections() const {
			return ions.get_Directions();
		};
		double energy() const {
			double sum = 0.0;
			double res = 0.0;
			for (size_t i = 0; i < il.getN(); ++i) {
				for (size_t a = 0; a < il.getNeighborCount(); ++a) {
					const size_t slot = il.getNeighborCount() * i + a;
					const size_t j =
						static_cast<size_t>(il.getNeighborIndices()[slot]);
					ions.
				}
			}
			
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
