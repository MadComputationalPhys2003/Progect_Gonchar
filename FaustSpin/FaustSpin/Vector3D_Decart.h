#pragma once
#include <cmath>
#include<array>
namespace Vector3D_Decart
{
	class Vector3D_Dec
	{
	public:
		double norm() const { return std::sqrt(Vx * Vx + Vy * Vy + Vz * Vz); }
		double Vx, Vy, Vz;//Projections
		std::array<double, 3> Vec3D;
		double norma;
		explicit Vector3D_Dec(double x, double y, double z) : Vx(x), Vy(y), Vz(z), Vec3D({ x, y, z }) {
			norma = norm();
		}
		Vector3D_Dec operator-()const {
			return Vector3D_Dec(-Vx, -Vy, -Vz);
		}
		Vector3D_Dec& operator+=(const Vector3D_Dec& other) {
			Vx += other.Vx;
			Vy += other.Vy;
			Vz += other.Vz;
			Vec3D = { Vx, Vy, Vz };
			norma = norm();
			return *this;
		}
		Vector3D_Dec& operator-=(const Vector3D_Dec& other) {
			Vx -= other.Vx;
			Vy -= other.Vy;
			Vz -= other.Vz;
			Vec3D = { Vx, Vy, Vz };
			norma = norm();
			return *this;
		}
		Vector3D_Dec operator*=(double scalar) {
			Vx *= scalar;
			Vy *= scalar;
			Vz *= scalar;
			Vec3D = { Vx, Vy, Vz };
			norma = norm();
			return *this;
		}

	};
	Vector3D_Dec operator+(Vector3D_Dec vec1,Vector3D_Dec vec2) {
		return vec1 += vec2;
	}
	Vector3D_Dec operator-(Vector3D_Dec vec1,Vector3D_Dec vec2) {
		return vec1 -= vec2;
	}
	Vector3D_Dec operator*(Vector3D_Dec vec, double scalar) {
		return vec *= scalar;
	}
	double scalar_product(const Vector3D_Dec& vec1, const Vector3D_Dec& vec2) {
		return vec1.Vx * vec2.Vx + vec1.Vy * vec2.Vy + vec1.Vz * vec2.Vz;
	}
}
