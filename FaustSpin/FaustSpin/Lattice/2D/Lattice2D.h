#pragma once
#include<array>
#include<functional>
#include<stdexcept>
#include<cstdint>
#include<vector>

struct Cell2D
{
	uint64_t x, y;
	Cell2D(uint64_t x = 0, uint64_t y = 0) : x(x), y(y) {}
};
namespace Lattice2D {
	class Lattice2D_Data {
	private:
		uint64_t Lx, Ly;
		size_t N;
		std::vector<Cell2D> cells;
		Cell2D cell0;
		bool lattice_set;
	public:
		explicit Lattice2D_Data(uint64_t Lx, uint64_t Ly,
			uint64_t x0 = 0, uint64_t y0 = 0) :
			Lx(Lx), Ly(Ly), cell0(x0, y0) {
			N = static_cast<size_t>(Lx * Ly);
			cells.resize(N);
			lattice_set = false;
		}
		const Cell2D& get_cell(size_t index) const {
			if (!lattice_set) throw std::runtime_error("Lattice doesn't set");
			if (index >= N) throw std::out_of_range("Index out of range");
			return cells[index];
		}
		std::vector<Cell2D> get_cells() const {
			if (!lattice_set) throw std::runtime_error("Lattice doesn't set");
			return cells;
		}
		void set_cells(const std::vector<Cell2D>& new_cells) {
			if (new_cells.size() != N) throw std::invalid_argument("New cells vector size must match the total number of cells");
			cells = new_cells;
			lattice_set = true;
		}
		const Cell2D& get_cell0() const {
			return cell0;
		}
		const uint64_t& get_Lx() const { return Lx; }
		const uint64_t& get_Ly() const { return Ly; }
		const size_t& get_N() const { return N; }
		std::array<uint64_t, 2> get_coords(size_t idx) const {
			if (idx >= N) throw std::out_of_range("Index out of range");
			uint64_t x = idx / Ly;
			uint64_t y = idx % Ly;
			return { x, y };
		}
		size_t get_index(uint64_t x, uint64_t y) const {
			if (x >= Lx || y >= Ly) {
				throw std::out_of_range("Coordinates out of range");
			}
			return static_cast<size_t>(x * Ly + y);
		}
	};
	inline Lattice2D_Data create_lattice2D(const Lattice2D_Data& ld, std::function<Cell2D(const Lattice2D_Data&, uint64_t, uint64_t)> func) {
		std::vector<Cell2D> new_cells(ld.get_N());
		size_t ion_count = 0;
		for (size_t x = 0; x < ld.get_Lx() && ion_count < ld.get_N(); x++) {
			for (size_t y = 0; y < ld.get_Ly() && ion_count < ld.get_N(); y++) {
				new_cells[ion_count] = func(ld, x, y);
				ion_count++;
			}
		}
		Lattice2D_Data new_lattice(ld.get_Lx(), ld.get_Ly(), ld.get_cell0().x, ld.get_cell0().y);
		new_lattice.set_cells(new_cells);
		return new_lattice;
	}
}
namespace Lattice2D_Setters {
	inline Cell2D set_square_cell(const Lattice2D::Lattice2D_Data& lattice, uint64_t x, uint64_t y) {
		const Cell2D& cell_0 = lattice.get_cell0();
		uint64_t x0 = cell_0.x;
		uint64_t y0 = cell_0.y;
		if ((x >= lattice.get_Lx()) ||
			(y >= lattice.get_Ly())) {
			throw std::out_of_range("Coordinates out of range");
		}
		uint64_t x_cell = (x0 + x);
		uint64_t y_cell = (y0 + y);
		return Cell2D(x_cell, y_cell);
	}
}