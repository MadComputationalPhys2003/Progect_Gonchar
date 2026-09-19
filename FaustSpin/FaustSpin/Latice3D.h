#pragma once
#include<array>
#include<functional>
#include<stdexcept>
#include<tuple>
#include<cstdint>
#include<vector>



struct Cell3D
{
	int64_t x, y, z;
	Cell3D(int64_t x=0, int64_t y=0, int64_t z=0) : x(x), y(y), z(z) {}
};
namespace Lattice3D {
	class Latice3D_Data {
	private:
		uint64_t Lx, Ly, Lz; // Dimensions of the lattice
		size_t N; // Total number of cells
		std::vector<Cell3D> cells;// Vector to store the cells
		Cell3D cell0; // Reference cell
		bool lattice_set;//
	public:
		explicit Latice3D_Data(uint64_t Lx, uint64_t Ly, uint64_t Lz,
			int64_t x0 = 0, int64_t y0 = 0, int64_t z0 = 0) :
			Lx(Lx), Ly(Ly), Lz(Lz), cell0(x0, y0, z0) {
			N = static_cast<size_t>(Lx * Ly * Lz);
			cells.resize(N);
			lattice_set = false;
		}
		const Cell3D& get_cell(size_t index) const {
			if (!lattice_set) throw std::runtime_error("Lattice doesn't set");
			if (index >= N) throw std::out_of_range("Index out of range");
			return cells[index];
		}
		std::vector<Cell3D> get_cells() const {
			if (!lattice_set) throw std::runtime_error("Lattice doesn't set");
			return cells;
		}
		void set_cells(const std::vector<Cell3D>& new_cells) {
			if (new_cells.size() != N) throw std::invalid_argument("New cells vector size must match the total number of cells");
			lattice_set = true;
			cells = new_cells;
		}
		const Cell3D& get_cell0() const {
			return cell0;
		}
		const uint64_t& get_Lx() const { return Lx; }
		const uint64_t& get_Ly() const { return Ly; }
		const uint64_t& get_Lz() const { return Lz; }
		const size_t& get_N() const { return N; }
		std::array<int64_t, 3> get_coords(size_t idx) const {
			if (idx >= N) throw std::out_of_range("Index out of range");
			int64_t x=idx/(Ly*Lz);
			int64_t r = idx % (Ly * Lz);
			int64_t y = r / Lz;
			int64_t z = r % Lz;
			return {x, y, z};
		}
		size_t get_index(int64_t x, int64_t y, int64_t z) const {
			if (x < 0 || x >= static_cast<int64_t>(Lx) ||
				y < 0 || y >= static_cast<int64_t>(Ly) ||
				z < 0 || z >= static_cast<int64_t>(Lz)) {
				throw std::out_of_range("Coordinates out of range");
			}
			return static_cast<size_t>(x * Ly * Lz + y * Lz + z);
		}
	};
	inline Latice3D_Data create_lattice3D(const Latice3D_Data& ld, std::function<Cell3D(const Latice3D_Data&, int64_t, int64_t, int64_t)> func) {
		std::vector<Cell3D> new_cells(ld.get_N());
		size_t ion_count = 0;
		for (size_t x = 0; x < ld.get_Lx() && ion_count < ld.get_N(); x++) {
			for (size_t y = 0; y < ld.get_Ly() && ion_count < ld.get_N(); y++) {
				for (size_t z = 0; z < ld.get_Lz() && ion_count < ld.get_N(); z++) {
					new_cells[ion_count] = func(ld, x, y, z);
					ion_count++;
				}
			}
		}
		Latice3D_Data new_lattice(ld.get_Lx(), ld.get_Ly(), ld.get_Lz(), ld.get_cell0().x, ld.get_cell0().y, ld.get_cell0().z);
		new_lattice.set_cells(new_cells);
		return new_lattice;
	}
}
namespace Lattice3D_Setters {
	inline Cell3D set_cube_cell(const Lattice3D::Latice3D_Data& lattice,int64_t x, int64_t y, int64_t z) {
		Cell3D cell_0 = lattice.get_cell0();
		int64_t x0 = cell_0.x;
		int64_t y0 = cell_0.y;
		int64_t z0 = cell_0.z;
		if ((0>x||x >= lattice.get_Lx()) ||
			(0>y||y >= lattice.get_Ly() )||
			(0>z||z >= lattice.get_Lz())) {
			throw std::out_of_range("Coordinates out of range");
		}
		int64_t x_cell = (x0 + x);
		int64_t y_cell = (y0 + y);
		int64_t z_cell = (z0 + z);
		return Cell3D(x_cell, y_cell, z_cell);
	}
}