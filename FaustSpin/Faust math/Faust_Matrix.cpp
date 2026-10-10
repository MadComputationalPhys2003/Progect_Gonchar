#pragma once
#include "Faust_Matrix.h"
#include <stdexcept>
#include <exception>


namespace FM {
	class Matrix {
	private:
		size_t rows;
		size_t cols;
		std::vector<double> matrix;
	public:
		Matrix(size_t r, size_t c) :
			rows(r),cols(c)
		{
			if (rows < 2 && cols < 2) {
				throw std::invalid_argument("rows and cols both less 2");
			}
			matrix.resize(rows * cols);
		};
		size_t get_rows() const {
			return rows;
		};
		size_t get_cols() const {
			return cols;
		};
		size_t size() const {
			return rows * cols;
		};
		const double& operator[](size_t idx) const {
			return matrix[idx];
		};
		double& operator [](size_t idx) {
			return matrix[idx];
		};
		Matrix operator-( )const {
			Matrix res = *this;
			size_t k = 0;
			for (size_t i = 0; i < rows; i++) {
				for (size_t j = 0; j < cols; j++) {
					k = i *cols + j;
					res[k] = -1 * matrix[k];
				}
			}
			return *this;
		};
		Matrix operator*=(const Matrix& rhs) {
			Matrix res(rows, rhs.get_cols());
			double sum = 0.0;
			for (size_t i = 0; i < rows; i++) {
				for (size_t j = 0; j < rhs.get_cols(); j++) {
					sum = 0.0;
					for (size_t k = 0; k < cols; k++) {
						sum += matrix[i * cols + k] * rhs[k * rhs.get_cols() + j];
					}
					res[i * rhs.get_cols() + j] = sum;
				}
			}
			return res;
		};
		Matrix operator*=(double scalar) {
			Matrix res = *this;
			for(size_t i=0;i<res.get_rows();i++) {
				for(size_t j=0;j<res.get_cols();j++) {
					res[i * res.get_cols() + j] *= scalar;
				}
			}
			return res;
		};
		Matrix operator +=(const Matrix& rhs) {
			Matrix res = *this;
			size_t k = 0;
			for (size_t i = 0; i < res.get_rows(); i++) {
				for (size_t j = 0; j < res.get_cols(); j++) {
					k = i * res.get_cols() + j;
					res[k] += rhs[k];
				}
			}
			return res;
		};
		Matrix operator -=(const Matrix& rhs) {
			Matrix res = *this;
			size_t k = 0;
			for (size_t i = 0; i < res.get_rows(); i++) {
				for (size_t j = 0; j < res.get_cols(); j++) {
					k = i * res.get_cols() + j;
					res[k] -= rhs[k];
				}
			}
			return res;
		};
	};
	Matrix operator+(const Matrix& mat1, const Matrix& mat2) {
		if (mat1.get_cols()!= mat2.get_cols()&&mat1.get_rows()!=mat2.get_rows()) {
			throw std::runtime_error("Matrix sizes are not equal");
		} 
		Matrix res = mat1;
		return res += mat2;
		
	};
	Matrix operator-(const Matrix& mat1, const Matrix& mat2) {
		if (mat1.get_cols() != mat2.get_cols() && mat1.get_rows() != mat2.get_rows()) {
			throw std::runtime_error("Matrix sizes are not equal");
		}
		Matrix res = mat1;
		return res -= mat2;
	};
	Matrix operator*(const Matrix& mat1, const Matrix& mat2) {
		if (mat1.get_cols() != mat2.get_rows()) {
			throw std::runtime_error("Matrix sizes are not compatible for multiplication");
		}
		Matrix res = mat1;
		return res *= mat2;
	};

	Matrix operator*(const Matrix& mat, double scalar) {
		Matrix res = mat;
		return res *= scalar;
	};
	Matrix operator*(double scalar, const Matrix& mat) {
		Matrix res = mat;
		return res *= scalar;
	};

	std::ostream& operator<<(std::ostream& os, const Matrix& mat) {
		const size_t rows = mat.get_rows();
		const size_t cols = mat.get_cols();

		for (size_t i = 0; i < rows; ++i) {
			if (i != 0) os << '\n';

			for (size_t j = 0; j < cols; ++j) {
				if (j != 0) os << ' ';
				os << mat[i * cols + j];
			}
		}

		return os;
	}

	std::istream& operator>>(std::istream& is, Matrix& mat) {
		const size_t count = mat.get_rows() * mat.get_cols();

		for (size_t idx = 0; idx < count; ++idx) {
			double value;

			if (!(is >> value)) {
				return is;
			}

			mat[idx] = value;
		}

		return is;
	}
	Matrix transpose(const Matrix& mat) {
		Matrix res(mat.get_cols(), mat.get_rows());
		for (size_t i = 0; i < mat.get_rows(); i++) {
			for (size_t j = 0; j < mat.get_cols(); j++) {
				res[j * res.get_cols() + i] = mat[i * mat.get_cols() + j];
			}
		}
		return res;
	};
}
