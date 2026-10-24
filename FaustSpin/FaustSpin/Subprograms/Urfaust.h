#pragma once
#include "../Vector3D_Decart.h"
#include "../Lattice_Headers.h"
#include "../Neighbor_Headers.h"
#include "../Initialisation Headers/IonFaust.h"
#include "../Initialisation Headers/Faust_Exchange_Integral.h"
#include<random>
#include <variant>
#include<numbers>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <utility>
#include <vector>
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
		const std::vector<std::pair<std::size_t, std::size_t>>&
			getUniquePairs() const
		{
			if (dim == Dimension::one) {
				return ND1D.unique_pairs;
			}
			if (dim == Dimension::two) {
				return ND2D.unique_pairs;
			}
			if (dim == Dimension::three) {
				return ND3D.unique_pairs;
			}

			throw std::logic_error("Unknown dimension");
		}

		const std::vector<std::size_t>& getMatrixIndexBySlot() const
		{
			if (dim == Dimension::one) {
				return ND1D.matrixIndexBySlot;
			}
			if (dim == Dimension::two) {
				return ND2D.matrixIndexBySlot;
			}
			if (dim == Dimension::three) {
				return ND3D.matrixIndexBySlot;
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
		const std::vector<std::pair<std::size_t, std::size_t>>&
			getUniquePairs() const
		{
			return neighbors.getUniquePairs();
		}

		const std::vector<std::size_t>& getMatrixIndexBySlot() const
		{
			return neighbors.getMatrixIndexBySlot();
		}
	};
	class SpinSystem {
	private:
		InitLattice il;
		InitIons::Ions ions;
		ExchangeIntegral exchangeIntegral;
		ExchangeIntegral::ExchangeIntegralInit EII;
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
		static InitIons::Ions makeInitIons(
			std::size_t n,
			InitIons::Ions::SpinLengthInit sli,
			InitIons::Ions::SpinDirectionInit sdi,
			const std::string& df,
			std::optional<int> directionSeed)
		{
			return InitIons::Ions(n, sli, sdi, df, directionSeed);
		}
	public:
		SpinSystem(
			uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0,
			std::variant<double, std::vector<double>> spin,
			std::variant<double, std::vector<FM::Matrix>> j,
			InitIons::Ions::SpinLengthInit sli,
			InitIons::Ions::SpinDirectionInit sdi,
			ExchangeIntegral::ExchangeIntegralInit eii,
			Dimension d,
			double ee,
			std::string df = " ",
			std::optional<int> directionSeed = std::nullopt)
			: il(makeInitLattice(d, lx, ly, lz, x0, y0, z0))
			, ions(makeInitIons(
				il.getN(), sli, sdi, df, directionSeed))
			,exchangeIntegral(il.getUniquePairs().size(), eii)
			, EII(eii)
			, eps(ee)
		{
			ions.set_spin(std::move(spin), sli);
			exchangeIntegral.set_eexchange_integral(std::move(j), eii);
		}
		void setParallelDirection(
			const Vector3D_Decart::Vector3D_Dec& direction)
		{
			const double length = Vector3D_Decart::norm(direction);

			if (!std::isfinite(length) ||
				std::abs(length - 1.0) > 1.0e-12) {
				throw std::invalid_argument(
					"Parallel direction must be a finite unit vector");
			}

			for (InitIons::Ion& ion : ions.ions) {
				ion.set_direction(direction);
			}
		}
		std::vector<Vector3D_Decart::Vector3D_Dec> getSpinDirections() const {
			return ions.get_Directions();
		};
		double energy() const
		{
			double sum = 0.0;

			const auto& pairs = il.getUniquePairs();
			const auto& exchange =
				exchangeIntegral.get_exchange_integral();

			if (EII == ExchangeIntegral::ExchangeIntegralInit::constant) {
				const double J = std::get<double>(exchange);

				for (const auto& pair : pairs) {
					const std::size_t i = pair.first;
					const std::size_t j = pair.second;

					if (i == j) {
						continue;
					}

					sum += J
						* ions.get_spin_length(i)
						* ions.get_spin_length(j)
						* Vector3D_Decart::scalar_product(
							ions.get_direction(i),
							ions.get_direction(j)
						);
				}
			}
			else {
				const auto& J =
					std::get<std::vector<FM::Matrix>>(exchange);

				for (std::size_t k = 0; k < pairs.size(); ++k) {
					const std::size_t i = pairs[k].first;
					const std::size_t j = pairs[k].second;

					if (i == j) {
						continue;
					}

					sum += ions.get_spin_length(i)
						* ions.get_spin_length(j)
						* Vector3D_Decart::scalar_product(
							ions.get_direction(i),
							J[k] * ions.get_direction(j)
						);
				}
			}

			return sum;
		}
		void relaxation_sweep()
		{
			const auto& indices = il.getNeighborIndices();
			const auto& mapping = il.getMatrixIndexBySlot();
			const auto& exchange =
				exchangeIntegral.get_exchange_integral();

			const std::size_t neighborCount =
				il.getNeighborCount();

			std::vector<std::size_t> processedPairs;
			processedPairs.reserve(neighborCount);

			for (std::size_t i = 0; i < il.getN(); ++i) {
				Vector3D_Decart::Vector3D_Dec h(0.0, 0.0, 0.0);
				processedPairs.clear();

				for (std::size_t a = 0; a < neighborCount; ++a) {
					const std::size_t slot =
						neighborCount * i + a;

					const std::size_t j =
						static_cast<std::size_t>(indices[slot]);

					if (i == j) {
						continue;
					}

					const std::size_t k = mapping[slot];

					// Coincident periodic neighbors contribute only once.
					if (std::find(
						processedPairs.begin(),
						processedPairs.end(),
						k
					) != processedPairs.end()) {
						continue;
					}

					processedPairs.push_back(k);

					const double Sj = ions.get_spin_length(j);

					if (EII ==
						ExchangeIntegral::ExchangeIntegralInit::constant) {
						const double J = std::get<double>(exchange);

						h -= ions.get_direction(j) * (J * Sj);
					}
					else {
						const auto& J =
							std::get<std::vector<FM::Matrix>>(exchange);

						if (i < j) {
							h -= (J[k] * ions.get_direction(j)) * Sj;
						}
						else {
							h -= (
								FM::transpose(J[k])
								* ions.get_direction(j)
								) * Sj;
						}
					}
				}

				const double hNorm = Vector3D_Decart::norm(h);

				if (hNorm > 1.0e-15) {
					ions.ions[i].set_direction(
						h * (1.0 / hNorm)
					);
				}
			}
		}
	};
}
