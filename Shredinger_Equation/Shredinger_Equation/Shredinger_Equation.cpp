#include <iostream>
#include<functional>
#include <vector>
#include <cmath>
#include<complex>
#include<cstdint>
#include <fstream>
#include<iomanip>
#include<numbers>
#include <utility>
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
		double coeff_1 = ini.coefficient_k(ini.x[0]);
		double coeff_2 = ini.coefficient_k(ini.x[1]);
		double coeff_3 = ini.coefficient_k(ini.x[2]);
		double alpha_1 = (1 + (ini.h * ini.h) / 12.0 * coeff_1);
		double alpha_2 = 2*(1-(5*ini.h*ini.h/12.0*coeff_2));
		double alpha_3 = (1 + (ini.h * ini.h) / 12.0 * coeff_3);
		for (size_t i = 2; i < ini.N; i++) {
			ini.psi[i] = (alpha_2 * ini.psi[i - 1] - alpha_1 * ini.psi[i - 2]) * 1.0 / (alpha_3);
			if (std::abs(ini.psi[i]) > psi_max) {
				std::cout << "Warning: Wavefunction exceeded maximum value at x = " << ini.x[i] << std::endl;
				ini.psi.resize(i);
				break;
			}
			if (i + 1 < ini.N) {
				coeff_1 = coeff_2;
				coeff_2 = coeff_3;
				coeff_3 = ini.coefficient_k(ini.x[i + 1]);
				alpha_1 = (1 + (ini.h * ini.h) / 12.0 * coeff_1);
				alpha_2 = 2 * (1 - (5 * ini.h * ini.h / 12.0 * coeff_2));
				alpha_3 = (1 + (ini.h * ini.h) / 12.0 * coeff_3);
			}
		}
	}
	void export_to_csv(const init& ini, const std::string& filename) {
		std::ofstream out(filename);
		if (!out.is_open()) {
			std::cerr << "Error: could not open file " << filename << std::endl;
			return;
		}

		out << "x,psi\n";
		out << std::setprecision(15);

		for (size_t i = 0; i < ini.psi.size(); ++i) {
			out << ini.x[i] << "," << ini.psi[i] << "\n";
		}
	}


}
double potential_1(double x, double a, double V0) {
	if (std::abs(x) <= a) {
		return 0;
	}
	else {
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
namespace Test_Eigen_func {

	std::pair<std::function<double(double)>, std::function<double(double)>>
		make_infinite_well(double E) {
		return {
			[E](double) { return 2.0 * E; },
			[](double) { return 0.0; }
		};
	}

	std::pair<std::function<double(double)>, std::function<double(double)>>
		make_finite_well(double E, double a, double V0) {
		return {
			[E, a, V0](double x) { return 2.0 * (E - potential_1(x, a, V0)); },
			[](double) { return 0.0; } // V кусочно-постоянна; про x=a — см. ниже
		};
	}

	std::pair<std::function<double(double)>, std::function<double(double)>>
		make_harmonic(double E) {
		return {
			[E](double x) { return 2.0 * E - x * x; },
			[](double x) { return -2.0 * x; }
		};
	}

	std::pair<std::function<double(double)>, std::function<double(double)>>
		make_anharmonic(double E, double lambda) {
		return {
			[E, lambda](double x) {
				double x2 = x * x;
				return 2.0 * E - x2 - 2.0 * lambda * x2 * x2;
			},
			[lambda](double x) {
				double x2 = x * x;
				return -2.0 * x - 8.0 * lambda * x2 * x;
			}
		};
	}
}
namespace Out_Eigen {
	int count_nodes(const std::vector<double>& psi) {
		int nodes = 0;
		for (size_t i = 1; i < psi.size(); ++i) {
			if (psi[i - 1] * psi[i] < 0.0) ++nodes;
		}
		return nodes;
	}

	void test_infinite_well() {
		const double a = 1.0;
		const double h = 0.001;
		const double psi_max = 1e6;
		const int n = 1;
		const double E_exact = n * n * pi * pi / (8.0 * a * a);

		auto [k, kd] = Test_Eigen_func::make_infinite_well(E_exact);
		Eigen::init ini(Eigen::init::parity::Even, 0.0, a, h, k, kd);
		Eigen::numerow_method(ini, psi_max);
		Eigen::export_to_csv(ini, "infinite_well.csv");

		double psi_end = ini.psi.back();
		int nodes = count_nodes(ini.psi);
		bool pass = std::abs(psi_end) < 1e-3 && nodes == 0;

		std::cout << "[infinite well] n=" << n
			<< " E_exact=" << E_exact
			<< " psi(a)=" << psi_end
			<< " nodes=" << nodes
			<< " (" << ini.psi.size() << "/" << ini.N << " points)"
			<< (pass ? "  PASS" : "  FAIL") << "\n";
	}

	void test_harmonic() {
		const double xmax = 8.0;
		const double h = 0.001;
		const double psi_max = 1e6;
		const int n = 0;
		const double E_exact = n + 0.5;

		auto [k, kd] = Test_Eigen_func::make_harmonic(E_exact);
		Eigen::init ini(Eigen::init::parity::Even, 0.0, xmax, h, k, kd);
		Eigen::numerow_method(ini, psi_max);
		Eigen::export_to_csv(ini, "harmonic.csv");

		double psi_end = ini.psi.back();
		int nodes = count_nodes(ini.psi);
		bool pass = std::abs(psi_end) < 1e-6 && nodes == 0;

		std::cout << "[harmonic] n=" << n
			<< " E_exact=" << E_exact
			<< " psi(xmax)=" << psi_end
			<< " nodes=" << nodes
			<< " (" << ini.psi.size() << "/" << ini.N << " points)"
			<< (pass ? "  PASS" : "  FAIL") << "\n";
	}

	void test_finite_well() {
		const double a = 1.0, V0 = 150.0, h = 0.001, psi_max = 1e6;
		const double E_guess = 1.0 * pi * pi / (8.0 * a * a);

		auto [k, kd] = Test_Eigen_func::make_finite_well(E_guess, a, V0);
		Eigen::init ini(Eigen::init::parity::Even, 0.0, a * 3.0, h, k, kd);
		Eigen::numerow_method(ini, psi_max);
		Eigen::export_to_csv(ini, "finite_well.csv");

		std::cout << "[finite well] E_guess=" << E_guess
			<< " psi(edge)=" << ini.psi.back()
			<< " nodes=" << count_nodes(ini.psi)
			<< " (" << ini.psi.size() << "/" << ini.N << " points)"
			<< "  -- sanity-check, не PASS/FAIL\n";
	}

	void test_anharmonic() {
		const double lambda = 0.1;
		const double xmax = 8.0, h = 0.0001, psi_max = 1e4;
		const double E_guess = 0.175;

		auto [k, kd] = Test_Eigen_func::make_anharmonic(E_guess, lambda);
		Eigen::init ini(Eigen::init::parity::Even, 0.0, xmax, h, k, kd);
		Eigen::numerow_method(ini, psi_max);
		Eigen::export_to_csv(ini, "anharmonic.csv");

		std::cout << "[anharmonic] lambda=" << lambda
			<< " E_guess=" << E_guess
			<< " psi(xmax)=" << ini.psi.back()
			<< " nodes=" << count_nodes(ini.psi)
			<< " (" << ini.psi.size() << "/" << ini.N << " points)"
			<< "  -- sanity-check, не PASS/FAIL\n";
	}
}
namespace Krank_Nicolson {
	namespace detail {
		std::vector<std::complex<double>> thomas_algorithm(const std::complex<double> A,
			const std::vector<std::complex<double>>& B,
			const std::vector<std::complex<double>>& D) {
			if(D.empty()||B.empty()||B.size()!=D.size()){
				throw std::runtime_error("Error: D or B vector is empty or their sizes do not match.");
			};
			size_t n = D.size();
			std::vector<std::complex<double>> psi_state;
			psi_state.resize(n);
			std::vector<std::complex<double>> alpha;
			std::vector<std::complex<double>> beta;
			alpha.resize(n);
			beta.resize(n);
			alpha[0] = -A / B[0];
			beta[0] = D[0] / B[0];
			for(size_t i =1;i<n;i++){
				alpha[i] = -A*1.0 / (B[i] + A * alpha[i - 1]);
				beta[i] = (D[i] - A * beta[i - 1]) * 1.0 / (B[i] + A * alpha[i - 1]);
			}
			psi_state[n - 1] = beta[n - 1];
			for(size_t i = n - 2; i < n; --i) {
				psi_state[i] = alpha[i] * psi_state[i + 1] + beta[i];
			}
			return psi_state;
		}

	}
	class init {
	public:
		double dx, dt; //Spatial and temporal step sizes
		std::function<double(double, double)> V;//Potential function
		std::vector<std::complex<double>> psi; //Wavefunction vector at time dt
		std::vector<double> x; //Spatial grid vector
		double xmin, xmax; //left and right boundary
		size_t N; //Number of spatial grid points
		double t; //Current time
		init(double ddx, double ddt, std::function<double(double, double)> V_func,
			double x_min, double x_max) :
			dx(ddx), dt(ddt), V(V_func), xmin(x_min), xmax(x_max), t(0.0)
		{
			N = static_cast<size_t>(std::round((std::abs(xmax - xmin)) / dx)) + 1;
			dx = std::abs(xmax - xmin) / (N - 1);
			x.resize(N);
			for (size_t i = 0; i < N; ++i) {
				x[i] = xmin + i * dx;
			}
			psi.resize(N, 0.0);
		}

	};

	void step(init& ini) {
		std::vector<std::complex<double>> B(ini.N-2);
		std::vector<std::complex<double>> D(ini.N-2);
		const double dx2 = ini.dx * ini.dx;
		const std::complex<double> A = std::complex<double>(0, -ini.dt / (4.0 * dx2));
		size_t n = ini.N - 2;//Number of interior points
		double t_mid =ini.t+ini.dt*0.5;
		std::vector<std::complex<double>> psi_new;
		for (size_t i = 0; i < n; i++) {
			double V_mid=ini.V(ini.x[i + 1], t_mid);
			B[i]=std::complex<double>(1.0, ini.dt*0.5*(1.0/dx2 + V_mid));
			D[i] = (2.0 - B[i]) * ini.psi[i+1] - A * (ini.psi[i + 2] + ini.psi[i]);
		}
		psi_new = detail::thomas_algorithm(A, B, D);
		for(size_t i=0;i<n;i++){
			ini.psi[i+1] = psi_new[i];
		}
		ini.psi[0] = 0.0; //Boundary condition at the left end
		ini.psi[ini.N - 1] = 0.0; //Boundary condition at the right end
		ini.t += ini.dt;
	}
	void evolve(init& ini, size_t n_steps) {
		for (size_t k = 0; k < n_steps; ++k)
			step(ini);
	}
	void write_psi_csv(const std::string& filename,
		const std::vector<double>& x,
		const std::vector<std::complex<double>>& psi)
	{
		std::ofstream out(filename);
		if (!out.is_open())
			throw std::runtime_error("write_psi_csv: cannot open " + filename);

		out << std::setprecision(15);
		out << "x,Re_psi,Im_psi\n";

		for (size_t j = 0; j < psi.size(); ++j) {
			out << x[j] << ","
				<< psi[j].real() << ","
				<< psi[j].imag() << "\n";
		}
	}





}







int main() {
	Out_Eigen::test_infinite_well();
	Out_Eigen::test_harmonic();
	Out_Eigen::test_finite_well();
	Out_Eigen::test_anharmonic();
}