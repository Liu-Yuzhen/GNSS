#include "interpolater.h"

namespace lyz {

double* getDiffTable(const std::vector<double>& xs,
	const std::vector<double>& ys) {
	const int n = xs.size();
	double* f = new double[n];
	for (int i = 0; i < n; i++) {
		for (int j = n - 1; j >= i; j--) {
			// first loop
			if (i == 0) { f[j] = ys[j]; }
			else {
				f[j] = (f[j] - f[j - 1]) / (xs[j] - xs[j - i]);
			}
		}
	}
	return f;
}


std::vector<double> Interpolater::Newton(const std::vector<double>& xs,
	const std::vector<double>& ys, const std::vector<double>& x0) {
	double* f = getDiffTable(xs, ys);
	std::vector<double> res;
	for (int k = 0; k < x0.size(); k++) {
		double sum = 0;
		for (int i = 0; i < xs.size(); i++) {
			double mul = f[i];
			for (int j = 0; j < i; j++) {
				mul *= x0[k] - xs[j];
			}
			sum += mul;
		}
		res.push_back(sum);
	}
	delete[] f;
	return res;
}


double Interpolater::Lagrange(const std::vector<double>& xs, 
	const std::vector<double>& ys, double x) {

	int n = xs.size();
	double res = 0;
	for (size_t i = 0; i < n; i++) {
		double tmp = 1.0;
		for (size_t j = 0; j < n; j++) {
			if (j == i)
				continue;
			tmp *= (x - xs[j]) / (xs[i] - xs[j]);

		}
		res += ys[i] * tmp;
	}

	return res;
}


std::vector<double> Interpolater::Lagrange(const std::vector<double>& xs,
	const std::vector<double>& ys, const std::vector<double>& x0) {
	std::vector<double> res;
	for (int i = 0; i < x0.size(); i++) {
		res.push_back(Interpolater::Lagrange(xs, ys, x0[i]));
	}
	return res;
}


}
