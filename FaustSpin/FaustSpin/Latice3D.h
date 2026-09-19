#pragma once
#include<array>
#include<cstdint>
#include<vector>

struct Cell
{
	int64_t x, y, z;
	Cell(int64_t x=0, int64_t y=0, int64_t z=0) : x(x), y(y), z(z) {}
};
class Latice3D{
private:
	uint64_t Lx, Ly, Lz; // Dimensions of the lattice
	size_t N; // Total number of cells
	int64_t x0, y0, z0; // Coordinates of the reference cell
	Cell cell0; // Reference cell
	std::vector<Cell> cells;
public:
	explicit Latice3D(uint64_t Lx, uint64_t Ly, uint64_t Lz,
		int64_t x0=0, int64_t y0=0, int64_t z0=0) :
		Lx(Lx), Ly(Ly), Lz(Lz), x0(x0), y0(y0), z0(z0), cell0(x0, y0, z0) {
		N = static_cast<size_t>(Lx *Ly *Lz);
		cells.resize(N);
	}
};