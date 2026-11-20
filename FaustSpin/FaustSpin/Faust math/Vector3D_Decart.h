#pragma once
#include <cmath>
#include<stdexcept>
#include<array>
#include"Faust math/Faust_Matrix.h"
namespace Vector3D_Decart
{
	class Vector3D_Dec
	{
	private:
		std::array<double, 3> Vec3D;//Исправить
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
		Vector3D_Dec& operator*=(double scalar) {
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
	inline Vector3D_Dec operator*(double scalar,Vector3D_Dec vec) {
		return vec *= scalar;
	}
	inline Vector3D_Dec operator*(const FM::Matrix& mat, const Vector3D_Dec& vec) {
		if (mat.get_cols() != 3 || mat.get_rows() != 3) {
			throw std::invalid_argument("Matrix must be 3x3 for multiplication with Vector3D_Dec");
		};
		double row1 = 0.0;
		double row2 = 0.0;
		double row3 = 0.0;
		row1 = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2];
		row2 = mat[3] * vec[0] + mat[4] * vec[1] + mat[5] * vec[2];
		row3 = mat[6] * vec[0] + mat[7] * vec[1] + mat[8] * vec[2];
		Vector3D_Dec result(row1, row2, row3);
		return result;
	};
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
