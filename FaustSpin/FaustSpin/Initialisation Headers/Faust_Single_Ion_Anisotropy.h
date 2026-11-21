#pragma once
#include"../Faust math/Faust_Matrix.h"
#include<variant>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <locale>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


class Single_Ion_Anisotropy
{
private:
	std::variant<double, std::vector<FM::Matrix>> sia;
	std::size_t ion_count;
public:
	enum class Single_Ion_Anisotropy_Init { constant, matrix };
	Single_Ion_Anisotropy_Init SIAI;
	Single_Ion_Anisotropy(std::size_t c,
		Single_Ion_Anisotropy_Init si=Single_Ion_Anisotropy_Init::constant
	):ion_count(c), SIAI(si)
	{
		if (SIAI == Single_Ion_Anisotropy_Init::constant) {
			sia = -1.0;
		}
		else if (SIAI==Single_Ion_Anisotropy_Init::matrix){
			std::vector<FM::Matrix> res(ion_count, FM::Matrix(3, 3));
			sia = res;
		}
	}

    std::variant<double, std::vector<FM::Matrix>> set_single_ion_anisotropy(
        std::variant<double, std::vector<FM::Matrix>> values,
        Single_Ion_Anisotropy_Init mode)
    {
        if (mode == Single_Ion_Anisotropy_Init::constant) {
            const double* value = std::get_if<double>(&values);

            if (value == nullptr) {
                throw std::invalid_argument(
                    "Constant mode requires one anisotropy coefficient"
                );
            }

            if (!std::isfinite(*value)) {
                throw std::invalid_argument(
                    "Anisotropy coefficient must be finite"
                );
            }
        }
        else if (mode == Single_Ion_Anisotropy_Init::matrix) {
            const std::vector<FM::Matrix>* matrices =
                std::get_if<std::vector<FM::Matrix>>(&values);

            if (matrices == nullptr || matrices->size() != ion_count) {
                throw std::invalid_argument(
                    "Matrix mode requires one anisotropy matrix per ion"
                );
            }

            for (const FM::Matrix& matrix : *matrices) {
                if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                    throw std::invalid_argument(
                        "Anisotropy matrix must be 3x3"
                    );
                }

                for (std::size_t i = 0; i < 9; ++i) {
                    if (!std::isfinite(matrix[i])) {
                        throw std::invalid_argument(
                            "Anisotropy matrix elements must be finite"
                        );
                    }
                }
            }
        }
        else {
            throw std::invalid_argument(
                "Unknown single-ion anisotropy mode"
            );
        }

        // Проверки и копирование выполняем до изменения состояния.
        std::variant<double, std::vector<FM::Matrix>> result = values;

        sia = std::move(values);
        SIAI = mode;

        return result;
    }

	const std::variant<double, std::vector<FM::Matrix>>&
		get_Single_Ion_Anisotropy() const {
		return sia;
	}
};

