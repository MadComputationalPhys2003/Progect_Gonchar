#include <iostream>
#include<functional>
#include <vector>
#include <cmath>
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
		init(parity par, double h,double xmin, double xmax, std::function<double(double)> coefficient_k, std::function<double(double)> coefficient_k_diff)
			: wavefunction_parity(par),  h(h), xmin(xmin), xmax(xmax), coefficient_k(coefficient_k), coefficient_k_diff(coefficient_k_diff)
		{
			N =  static_cast<size_t>(std::round(std::abs(xmax) * 1.0 / (h) + 1));
			x.resize(N);
			for(size_t i=0; i<N; i++)
			{
				x[i] = xmin + i * h;
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
				psi1 = 1 - (h * h / 2.0) * coefficient_k(x[0]) - (h * h * h / 6.0) * coefficient_k_diff(x[0]);
			}
			return psi1;
		}
	};


}

int main()
{
	std::cout << "Hello World!\n";
}
