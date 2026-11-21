#pragma once
#include "Faust math/Vector3D_Decart.h"
#include "Lattice_Headers.h"
#include "Neighbor_Headers.h"
#include "Initialisation Headers/IonFaust.h"
#include "Initialisation Headers/Faust_Exchange_Integral.h"
#include"Initialisation Headers/Faust_Single_Ion_Anisotropy.h"
#include"Initialisation Headers/Faust_Zeeman_Term.h"
#include"Subprograms/Urfaust.h"
#include<random>
#include <variant>
#include<numbers>
#include <algorithm>
#include <optional>
#include <cstddef>
#include <utility>
#include <vector>
constexpr double mu_Bor = 9.2740100783e-24;  // Bohr magneton J/T
namespace FaustSpin {
	enum class Dimension {
		one,
		two,
		three
	};
	struct NeighborInit {
	private:
		Dimension dim;
		Neighbors1D::NeighborData ND1D;
		Neighbors2D::NeighborData ND2D;
		Neighbors3D::NeighborData ND3D;
	public:
		NeighborInit(const Lattice1D::Lattice1D_Data& lattice) :
			dim(Dimension::one), ND1D(Neighbors1D::get_neighbors_periodic_BC_1D(lattice))
		{
		}
		NeighborInit(const Lattice2D::Lattice2D_Data& lattice) :
			dim(Dimension::two), ND2D(Neighbors2D::get_neighbors_periodic_BC_2D(lattice))
		{
		}
		NeighborInit(const Lattice3D::Lattice3D_Data& lattice) :
			dim(Dimension::three), ND3D(Neighbors3D::get_neighbors_periodic_BC_3D(lattice))
		{
		}
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
	struct InitLattice {//попробовать с помощью интерфейса
	private:
		Dimension dim;
		Lattice1D::Lattice1D_Data lattice1D;
		Lattice2D::Lattice2D_Data lattice2D;
		Lattice3D::Lattice3D_Data lattice3D;
		NeighborInit neighbors;
	public:
		InitLattice(uint64_t lx, uint64_t x_0) :
			dim(Dimension::one),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(lx, x_0), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(1, 1, 1, 1), Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(1, 1, 1, 1, 1, 1), Lattice3D_Setters::set_cube_cell)),
			neighbors(lattice1D)
		{
		}
		InitLattice(uint64_t lx, uint64_t ly, uint64_t x_0, uint64_t y_0) :
			dim(Dimension::two),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(1, 1), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(lx, ly, x_0, y_0), Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(1, 1, 1, 1, 1, 1), Lattice3D_Setters::set_cube_cell)),
			neighbors(lattice2D)
		{
		}
		InitLattice(uint64_t lx, uint64_t ly, uint64_t lz, uint64_t x_0, uint64_t y_0, uint64_t z_0) :
			dim(Dimension::three),
			lattice1D(Lattice1D::create_lattice1D(Lattice1D::Lattice1D_Data(1, 1), Lattice1D_Setters::set_chain_cell)),
			lattice2D(Lattice2D::create_lattice2D(Lattice2D::Lattice2D_Data(1, 1, 1, 1), Lattice2D_Setters::set_square_cell)),
			lattice3D(Lattice3D::create_lattice3D(Lattice3D::Lattice3D_Data(lx, ly, lz, x_0, y_0, z_0), Lattice3D_Setters::set_cube_cell)),
			neighbors(lattice3D)
		{
		}
	public:
		const Lattice1D::Lattice1D_Data& get_lattice1D() const {
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
		const Lattice3D::Lattice3D_Data& get_lattice3D() const {
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
	class FaustSpinSystemInit {
	private:
		InitLattice il;
		InitIons::Ions ions;
		ExchangeIntegral exchangeIntegral;
		Single_Ion_Anisotropy single;
		Zeeman_Term zeenman;
		ExchangeIntegral::ExchangeIntegralInit EII;
		Single_Ion_Anisotropy::Single_Ion_Anisotropy_Init SIAI;
		Zeeman_Term::Zeeman_Term_Init ZTI;
		FM::Matrix B = FM::Matrix(3, 1);
		double eps;
		static InitLattice makeInitLattice(Dimension dim, uint64_t lx, uint64_t ly, uint64_t lz,
			uint64_t x0, uint64_t y0, uint64_t z0) {
			if (dim == Dimension::one) {
				return InitLattice(lx, x0);
			}
			if (dim == Dimension::two) {
				return InitLattice(lx, ly, x0, y0);
			}
			if (dim == Dimension::three) {
				return InitLattice(lx, ly, lz, x0, y0, z0);
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
		bool exchange;
		bool ion_anisotropy;
		bool Zeeman;
		FaustSpinSystemInit(
			const uint64_t lx, const uint64_t ly, const uint64_t lz,
			const uint64_t x0, const uint64_t y0, const uint64_t z0,
			std::variant<double, std::vector<double>> spin,
			std::variant<double, std::vector<FM::Matrix>> j,
			std::variant<double, std::vector<FM::Matrix>> dsin,
			std::variant<double, std::vector<FM::Matrix>> zennman1,
			InitIons::Ions::SpinLengthInit sli,
			InitIons::Ions::SpinDirectionInit sdi,
			ExchangeIntegral::ExchangeIntegralInit eii,
			Single_Ion_Anisotropy::Single_Ion_Anisotropy_Init siai,
			Zeeman_Term::Zeeman_Term_Init zti,
			FM::Matrix b,
			Dimension d,
			double ee,
			bool ex = true,
			bool ia = false,
			bool zee = false,
			std::string df = " ",
			std::optional<int> directionSeed = std::nullopt)
			: il(makeInitLattice(d, lx, ly, lz, x0, y0, z0))
			, ions(makeInitIons(
				il.getN(), sli, sdi, df, directionSeed))
			, exchangeIntegral(il.getUniquePairs().size(), eii)
			, single(il.getN(), siai)
			, zeenman(il.getN(), zti)
			, EII(eii)
			, SIAI(siai)
			, ZTI(zti)
			, B(b)
			, eps(ee)
			, exchange(ex)
			, ion_anisotropy(ia)
			, Zeeman(zee)
		{
			ions.set_spin(std::move(spin), sli);
			exchangeIntegral.set_eexchange_integral(std::move(j), eii);
			single.set_single_ion_anisotropy(std::move(dsin), siai);
			zeenman.set_zeeman_term(std::move(zennman1), zti);
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
		InitLattice getLattice()const {
			return il;
		}
		InitIons::Ions getIons() const {
			return ions;
		}
		ExchangeIntegral getExchangeIntegral() const {
			return exchangeIntegral;
		}
		Single_Ion_Anisotropy getSingle_Ion_Anisotropy()const {
			return single;
		}
		Zeeman_Term getZeenmanTerm()const {
			return zeenman;
		}
		const FM::Matrix& getMagneticField() const {
			return B;
		}
		ExchangeIntegral::ExchangeIntegralInit getExchangeIntegralInit()const {
			return EII;
		}
		Single_Ion_Anisotropy::Single_Ion_Anisotropy_Init getSingle_Ion_Anisotropy_Init()const {
			return SIAI;
		}
		Zeeman_Term::Zeeman_Term_Init getZeeman_Term_Init()const {
			return ZTI;
		}
	};
	namespace HamiltonianTermsAndMinAlgorithms {
		inline double exchange_interaction_term(FaustSpinSystemInit& sps)
		{
			double sum = 0.0;
			const auto& il = sps.getLattice();
			const auto& ions = sps.getIons();
			const auto& exchangeIntegral = sps.getExchangeIntegral();
			const auto& EII = sps.getExchangeIntegralInit();
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
		inline double single_ion_anisotropy_term(FaustSpinSystemInit& sps) {
			double sum = 0.0;
			const auto& il = sps.getLattice();
			const auto& ions = sps.getIons();
			const auto& single_ion_anisotropy_term = sps.getSingle_Ion_Anisotropy();
			const auto& SIAI = sps.getSingle_Ion_Anisotropy_Init();
			const auto& sia =
				single_ion_anisotropy_term.get_Single_Ion_Anisotropy();
			if (SIAI == Single_Ion_Anisotropy::Single_Ion_Anisotropy_Init::constant) {
				const double D = std::get<double>(sia);
				for (std::size_t i = 0; i < il.getN(); ++i) {
					const double Sz = ions.get_spin_length(i) * ions.get_direction(i)[2];
					sum += D * Sz * Sz;
				}
			}
			else if (SIAI == Single_Ion_Anisotropy::Single_Ion_Anisotropy_Init::matrix) {
				const auto& D = std::get<std::vector<FM::Matrix>>(sia);
				for (std::size_t i = 0; i < il.getN(); i++) {
					const auto& D_i = D.at(i);
					for (std::size_t j = 0; j < 3; j++) {
						for (std::size_t k = 0; k < 3; k++) {
							sum += ions.get_spin_length(i) * ions.get_spin_length(i) *
								ions.get_direction(i)[j] * D_i[3 * j + k] * ions.get_direction(i)[k];
						}
					}
				}
			}
			return sum;
		}
		inline double Zeenman_Term(FaustSpinSystemInit& sps) {
			double sum = 0.0;
			const auto& il = sps.getLattice();
			const auto& ions = sps.getIons();
			const auto& zeeman = sps.getZeenmanTerm();
			const auto ZTI = sps.getZeeman_Term_Init();
			const auto& zt = zeeman.get_Zeeman_Term();
			const auto& B = sps.getMagneticField();

			if (B.get_rows() != 3 || B.get_cols() != 1) {
				throw std::invalid_argument("Magnetic field must be a 3x1 column");
			}

			const Vector3D_Decart::Vector3D_Dec field(B[0], B[1], B[2]);
			if (ZTI == Zeeman_Term::Zeeman_Term_Init::constant) {
				const double g = std::get<double>(zt);
				for (std::size_t i = 0; i < il.getN(); ++i) {
					const auto spin = ions.get_spin_length(i) * ions.get_direction(i);
					sum += g * Vector3D_Decart::scalar_product(field, spin);
				}
			}
			else if (ZTI == Zeeman_Term::Zeeman_Term_Init::matrix) {
				const auto& g = std::get<std::vector<FM::Matrix>>(zt);
				for (std::size_t i = 0; i < il.getN(); ++i) {
					const auto spin = ions.get_spin_length(i) * ions.get_direction(i);
					sum += Vector3D_Decart::scalar_product(field, g.at(i) * spin);
				}
			}
			else {
				throw std::invalid_argument("Unknown Zeeman term mode");
			}
			return -mu_Bor * sum;
		}
	}

	inline double Hamiltonan(FaustSpinSystemInit& sps) {
		return(sps.exchange ? HamiltonianTermsAndMinAlgorithms::exchange_interaction_term(sps) : 0.0)
			+ (sps.ion_anisotropy ? HamiltonianTermsAndMinAlgorithms::single_ion_anisotropy_term(sps) : 0.0)
			+ (sps.Zeeman ? HamiltonianTermsAndMinAlgorithms::Zeenman_Term(sps) : 0.0);
	}
}

	
