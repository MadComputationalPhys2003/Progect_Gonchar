#pragma once
#include <cmath>
#include<stdexcept>
#include<array>
namespace Vector3D_Decart
{
	class Vector3D_Dec
	{
	private:
		std::array<double, 3> Vec3D;
	public:
		explicit Vector3D_Dec(double x, double y, double z) : Vec3D({ x, y, z }) {
		}
		Vector3D_Dec operator-()const {
			return Vector3D_Dec(-Vec3D[0], -Vec3D[1], -Vec3D[2]);
		}
		Vector3D_Dec& operator+=(const Vector3D_Dec& other) {
			Vec3D[0] += other.Vec3D[0];
			Vec3D[1] += other.Vec3D[1];
			Vec3D[2] += other.Vec3D[2];
			return *this;
		}
		Vector3D_Dec& operator-=(const Vector3D_Dec& other) {
			Vec3D[0] -= other.Vec3D[0];
			Vec3D[1] -= other.Vec3D[1];
			Vec3D[2] -= other.Vec3D[2];
			return *this;
		}
		Vector3D_Dec operator*=(double scalar) {
			Vec3D[0] *= scalar;
			Vec3D[1] *= scalar;
			Vec3D[2] *= scalar;
			return *this;
		}
		const double& operator[](size_t i)const {
			if(i>=3) throw std::out_of_range("Index out of range");
			return Vec3D[i];
		}
		double& operator[](size_t i){
			if (i >= 3) throw std::out_of_range("Index out of range");
			return Vec3D[i];
		}
	};
	inline Vector3D_Dec operator+(Vector3D_Dec vec1,Vector3D_Dec vec2) {
		return vec1 += vec2;
	}
	inline Vector3D_Dec operator-(Vector3D_Dec vec1,Vector3D_Dec vec2) {
		return vec1 -= vec2;
	}
	inline Vector3D_Dec operator*(Vector3D_Dec vec, double scalar) {
		return vec *= scalar;
	}
	inline double scalar_product(const Vector3D_Dec& vec1, const Vector3D_Dec& vec2) {
		return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
	}
	inline double norm(const Vector3D_Dec& vec) { return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec	[2]); }
	inline Vector3D_Dec normalize(const Vector3D_Dec& vec) {
		double n = norm(vec);
		if (n == 0) throw std::runtime_error("Cannot normalize zero vector");
		return vec * (1.0 / n);
	}
}
