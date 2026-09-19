#pragma once
#include<array>
#include<functional>
#include<stdexcept>
#include<cstdint>
#include<vector>

struct Cell1D
{
	int64_t x;
	Cell1D(int64_t x = 0) : x(x) {}
};
namespace Lattice1D {
	class Lattice1D_Data {
	private:
		uint64_t Lx;
		size_t N;
		std::vector<Cell1D> cells;
		Cell1D cell0;
		bool lattice_set;
	public:
		explicit Lattice1D_Data(uint64_t Lx, int64_t x0 = 0) :
			Lx(Lx), cell0(x0) {
			N = static_cast<size_t>(Lx);
			cells.resize(N);
			lattice_set = false;
		}
		const Cell1D& get_cell(size_t index) const {
			if (!lattice_set) throw std::runtime_error("Lattice doesn't set");
			if (index >= N) throw std::out_of_range("Index out of range");
			return cells[index];
		}
		std::vector<Cell1D> get_cells() const {
			if (!lattice_set) throw std::runtime_error("Lattice doesn't set");
			return cells;
		}
		void set_cells(const std::vector<Cell1D>& new_cells) {
			if (new_cells.size() != N) throw std::invalid_argument("New cells vector size must match the total number of cells");
			cells = new_cells;
			lattice_set = true;
		}
		const Cell1D& get_cell0() const {
			return cell0;
		}
		const uint64_t& get_Lx() const { return Lx; }
		const size_t& get_N() const { return N; }
		int64_t get_coords(size_t idx) const {
			if (idx >= N) throw std::out_of_range("Index out of range");
			return static_cast<int64_t>(idx);
		}
		size_t get_index(int64_t x) const {
			if (x < 0 || x >= static_cast<int64_t>(Lx)) {
				throw std::out_of_range("Coordinates out of range");
			}
			return static_cast<size_t>(x);
		}
	};
	inline Lattice1D_Data create_lattice1D(const Lattice1D_Data& ld, std::function<Cell1D(const Lattice1D_Data&, int64_t)> func) {
		std::vector<Cell1D> new_cells(ld.get_N());
		size_t ion_count = 0;
		for (size_t x = 0; x < ld.get_Lx() && ion_count < ld.get_N(); x++) {
			new_cells[ion_count] = func(ld, x);
			ion_count++;
		}
		Lattice1D_Data new_lattice(ld.get_Lx(), ld.get_cell0().x);
		new_lattice.set_cells(new_cells);
		return new_lattice;
	}
}
namespace Lattice1D_Setters {
	inline Cell1D set_chain_cell(const Lattice1D::Lattice1D_Data& lattice, int64_t x) {
		const Cell1D& cell_0 = lattice.get_cell0();
		int64_t x0 = cell_0.x;
		if (0 > x || x >= lattice.get_Lx()) {
			throw std::out_of_range("Coordinates out of range");
		}
		int64_t x_cell = (x0 + x);
		return Cell1D(x_cell);
	}
}
