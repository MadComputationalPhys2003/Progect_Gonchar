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
class ExchangeIntegral {
private:
	std::variant<double,std::vector<FM::Matrix>> exint;
	uint64_t count;
public:
	enum class ExchangeIntegralInit{constant, matrix};
	ExchangeIntegralInit EI;
	ExchangeIntegral(uint8_t neighbor_count,size_t n,
	ExchangeIntegralInit ei=ExchangeIntegralInit::constant):
		count(neighbor_count*n/2), EI(ei)
	{
		if (EI == ExchangeIntegralInit::constant) {
			exint = -1.0;
		}
		else if (EI == ExchangeIntegralInit::matrix) {
			std::vector<FM::Matrix> res(count, FM::Matrix(3, 3));
			exint = res;
		}
       
	}
    std::variant<double, std::vector<FM::Matrix>> set_eexchange_integral(
        std::variant<double, std::vector<FM::Matrix>> values,
        ExchangeIntegralInit mode)
    {
        if (mode == ExchangeIntegralInit::constant) {
            const double* value = std::get_if<double>(&values);

            if (value == nullptr) {
                throw std::invalid_argument(
                    "Constant mode requires one exchange integral"
                );
            }

            if (!std::isfinite(*value)) {
                throw std::invalid_argument(
                    "Exchange integral must be finite"
                );
            }
        }
        else if (mode == ExchangeIntegralInit::matrix) {
            const std::vector<FM::Matrix>* matrices =
                std::get_if<std::vector<FM::Matrix>>(&values);

            if (matrices == nullptr || matrices->size() != count) {
                throw std::invalid_argument(
                    "Different mode requires one matrix per neighbor-list entry"
                );
            }

            for (const FM::Matrix& matrix : *matrices) {
                if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                    throw std::invalid_argument(
                        "Exchange matrix must be 3x3"
                    );
                }

                for (std::size_t i = 0; i < 9; ++i) {
                    if (!std::isfinite(matrix[i])) {
                        throw std::invalid_argument(
                            "Exchange matrix elements must be finite"
                        );
                    }
                }
            }
        }
        else {
            throw std::invalid_argument(
                "Unknown exchange integral mode"
            );
        }

        // Проверки и копирование выполняем до изменения состояния.
        std::variant<double, std::vector<FM::Matrix>> result = values;

        exint = std::move(values);
        EI = mode;

        return result;
    }
    const std::variant<double, std::vector<FM::Matrix>>&
        get_exchange_integral() const
    {
        return exint;
    }
   
};

