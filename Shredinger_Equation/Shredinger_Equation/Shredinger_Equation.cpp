#include <iostream>
#include<functional>
#include <vector>
#include <cmath>
#include<cstdint>
#include<numbers>
constexpr double pi=std::numbers::pi;
//Like program "Eigen" from Guld part 2 but used Numerov Method(Problems 17.1-17.4)
namespace Eigen {
	class init {
	public:
		enum class parity { Even, Odd };
		parity wavefunction_parity; //Parity of the wavefunction (Even or Odd)
		double xmin; //Value of x at the left boundary
		double xmax; //Value of x at the right boundary
		double h; //Step size for the numerical method
		std::function<double(double)> coefficient_k; //2*(energy-potetial)
		std::function<double(double)> coefficient_k_diff; //Differential of 2*(energy- potential)
		std::vector<double> x; //Vector of x values
		std::vector<double> psi; //Vector of wavefunction values
		std::vector<double> psi_diff; //Vector of wavefunction derivative values
		size_t N; //Number of steps in the numerical method
		init(parity par,double xmin, double xmax, double h, std::function<double(double)> coefficient_k, std::function<double(double)> coefficient_k_diff)
			: wavefunction_parity(par),  h(h), xmin(xmin), xmax(xmax), coefficient_k(coefficient_k), coefficient_k_diff(coefficient_k_diff)
		{
			N =  static_cast<size_t>(std::round(std::abs(xmax) * 1.0 / (h) + 1));
			x.resize(N);
			for(size_t i=0; i<N; i++)
			{
				x[i] =i * h;
			}
			psi.resize(N);
			psi_diff.resize(N);
			if (wavefunction_parity == parity::Even) {
				psi0 = 1.0; //Initial value of the wavefunction at the left boundary in Even case
				psi0_diff = 0.0; //Initial value of the derivative of the wavefunction at the left boundary in Even Case
				psi[0] = psi0;
				psi_diff[0] = psi0_diff;
				psi[1] = bootstrap(wavefunction_parity);
			}
			else if (wavefunction_parity == parity::Odd) {
				
				psi0 = 0; //Initial value of the wavefunction at the left boundary in Even case
				psi0_diff = 1.0; //Initial value of the derivative of the wavefunction at the left boundary in Even Case
				psi[0] = psi0;
				psi_diff[0] = psi0_diff;
				psi[1] = bootstrap(wavefunction_parity);
			}


		}
	private:
		double psi0; //Initial value of the wavefunction at the left boundary
		double psi0_diff; //Initial value of the derivative of the wavefunction at the left boundary
		double bootstrap(parity par) {
			double psi1;
			if (par == parity::Even) {
				psi1 = 1 - (h * h / 2.0) * coefficient_k(x[0]) - (h * h * h / 6.0) * coefficient_k_diff(x[0]);
			}
			else
			{
				psi1 = h-(h * h * h / 6.0) * coefficient_k(x[0]);
			}
			return psi1;
		}
	};
	void numerow_method(init& ini,const double& psi_max) {
		double alpha_1 = (1 + (ini.h * ini.h) / 12.0 * ini.coefficient_k(ini.x[0]));
		double alpha_2 = 2*(1-(5*ini.h*ini.h/12.0*ini.coefficient_k(ini.x[1])));
		double alpha_3 = (1 + (ini.h * ini.h) / 12.0 * ini.coefficient_k(ini.x[2]));
		for (size_t i = 2; i < ini.N; i++) {
			ini.psi[i] = (alpha_2 * ini.psi[i - 1] - alpha_1 * ini.psi[i - 2]) * 1.0 / (alpha_3);
			if (std::abs(ini.psi[i]) > psi_max) {
				std::cout << "Warning: Wavefunction exceeded maximum value at x = " << ini.x[i] << std::endl;
				break;
			}
			if (i + 1 < ini.N) {
				alpha_1 = alpha_2;
				alpha_2 = alpha_3;
				alpha_3 = (1 + (ini.h * ini.h) / 12.0 * ini.coefficient_k(ini.x[i + 1]));
			}
		}
	}



}
double potential_1(double x,double a,double V0) {
	if (std::abs(x) <= a) {
		return 0;
	}
	else{
		return V0;
	}
}
double func(double x) {
	uint8_t n = 1;
	double a = 1;
	double V0 = 20;
	double E = n * n * pi * pi / (8 * a * a);
	return 2 * (E - potential_1(x, a, V0));
}



int main()
{
	std::cout << "Hello World!\n";
}
