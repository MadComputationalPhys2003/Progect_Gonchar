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


class Zeeman_Term {
private:
	std::variant<double, std::vector<FM::Matrix>> zt;
	std::size_t ion_count;
public:
	enum class Zeeman_Term_Init{constant, matrix};
	Zeeman_Term_Init ZTI;
	Zeeman_Term(std::size_t c, Zeeman_Term_Init zti = Zeeman_Term_Init::constant) :
		ion_count(c), ZTI(zti)
	{
		if (ZTI == Zeeman_Term_Init::constant) {
			zt = 2.0;
		}
		else if (ZTI == Zeeman_Term_Init::matrix) {
			std::vector<FM::Matrix> res(ion_count, FM::Matrix(3, 3));
			zt = res;
		}
	}
    std::variant<double, std::vector<FM::Matrix>> set_zeeman_term(
        std::variant<double, std::vector<FM::Matrix>> values,
        Zeeman_Term_Init mode)
    {
        if (mode == Zeeman_Term_Init::constant) {
            const double* value = std::get_if<double>(&values);

            if (value == nullptr) {
                throw std::invalid_argument(
                    "Constant mode requires one g-factor"
                );
            }

            if (!std::isfinite(*value)) {
                throw std::invalid_argument(
                    "g-factor must be finite"
                );
            }
        }
        else if (mode == Zeeman_Term_Init::matrix) {
            const std::vector<FM::Matrix>* matrices =
                std::get_if<std::vector<FM::Matrix>>(&values);

            if (matrices == nullptr || matrices->size() != ion_count) {
                throw std::invalid_argument(
                    "Matrix mode requires one g-tensor per ion"
                );
            }

            for (const FM::Matrix& matrix : *matrices) {
                if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                    throw std::invalid_argument(
                        "g-tensor matrix must be 3x3"
                    );
                }

                for (std::size_t i = 0; i < 9; ++i) {
                    if (!std::isfinite(matrix[i])) {
                        throw std::invalid_argument(
                            "g-tensor matrix elements must be finite"
                        );
                    }
                }
            }
        }
        else {
            throw std::invalid_argument(
                "Unknown Zeeman term mode"
            );
        }

        // Проверки и копирование выполняем до изменения состояния.
        std::variant<double, std::vector<FM::Matrix>> result = values;

        zt = std::move(values);
        ZTI = mode;

        return result;
    }
	const std::variant<double, std::vector<FM::Matrix>>&
		get_Zeeman_Term() const {
		return zt;
	}
};
