#pragma once
#include<iostream>
#include <stddef.h>
#include <vector>

namespace FM {
	class Matrix {
	private:
		size_t rows;
		size_t cols;
		std::vector<double> matrix;
	public:
		Matrix(size_t r, size_t c);
		size_t get_rows() const;
		size_t get_cols() const;
		const double& operator[](size_t idx) const;
		double& operator [](size_t);
		Matrix operator-( )const ;
		Matrix operator*=(const Matrix& rhs);
		Matrix operator*=(double scalar);
		Matrix operator +=(const Matrix& rhs);
		Matrix operator -=(const Matrix& rhs);
	};
	Matrix operator+(const Matrix& mat1, const Matrix& mat2);
	Matrix operator-(const Matrix& mat1, const Matrix& mat2);
	Matrix operator*(const Matrix& mat1, const Matrix& mat2);

	Matrix operator*(const Matrix& mat, double scalar);
	Matrix operator*(double scalar, const Matrix& mat);

	std::ostream& operator<<(std::ostream& os,const Matrix& mat);
	std::istream& operator>>(std::istream& is, Matrix& mat);
	Matrix transpose (const Matrix& mat);
}
