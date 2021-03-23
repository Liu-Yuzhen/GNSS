#pragma once
#include <vector>
#include "mat.h"
#include "datatype.h"

namespace lyz {

class Interpolater
{
public:

static double Lagrange(const std::vector<double>& x, const std::vector<double>& y, double x0);


static std::vector<double> Lagrange(const std::vector<double>& x,
	const std::vector<double>& y, const std::vector<double>& x0);


static std::vector<double> Newton(const std::vector<double>& x,
	const std::vector<double>& y, const std::vector<double>& x0);


};

}

