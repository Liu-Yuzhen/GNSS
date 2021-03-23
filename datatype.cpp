#include "datatype.h"
#include "date.h"
#include "interpolater.h"
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>
#include <map>

namespace lyz {

const double GM = 3.986005e14;
const double w_earth = 7.2921151467e-5;
const double a_earth = 6378137.0;
const double f = 1 / 298.257223563;


    // use qstring instead
inline double _str2f(const std::string& str) {
	int len = str.length();
	double d = atof(str.substr(0, len - 4).c_str());
	int expo = atof(str.substr(len - 3).c_str());
	return d * pow(10, expo);
}


// use qstring instead
inline std::string trim(const std::string str) {
	size_t start = str.find_first_not_of(" ");
	size_t end = str.find_last_not_of(" ") + 1;
	return str.substr(start, end - start);
}




/*
class Epoch(const Date&)
*/
Epoch::Epoch(const Date& date):date_start(date){}



/*
class Position
*/
Position::Position(Pointd pt):pt(pt), clock_error(0.0) {}


Position::Position(Pointd pt, double clock_erorr, const Date& date) :
	pt(pt), clock_error(clock_erorr), date(date) {}
 


/*
class CSConvertor
*/
//-------------

/*
 *
 * we'd better to implement a abstract class for
 * Ephemeris to handle GLONASS and SBAS
 * The abstract class should have:
 *

*/


// TimeConvertor:: -> Data::
PositionPtr CSConvertor::ehp2position(
        const GPSEhpemerisPtr& brdc, const Date& date) {

	// gps second going with 1980
	GPST toe_gpst(brdc->GPSweek, brdc->toe);
    double date_jd = date2JD(date);
    double toe_jd = GPST2JD(toe_gpst);
    double toc_jd = date2JD(brdc->toc);


	double tk = (date_jd - toe_jd) * 86400;
	double a = brdc->sqrt_a;
	a = a * a;
    double n0 = sqrt(GM / pow(a, 3));
	double n = n0 + brdc->delta_n;
	double Mk = brdc->M0 + n * tk;

	double Ek = Mk, Ek0 = 0.0;
	while (abs(Ek - Ek0) > 1.0e-12) {
		Ek0 = Ek;
		Ek = Mk + brdc->e * sin(Ek0);
	}
	//sqrt(1 - pow(brdc.e, 2))*sin(E) , (cos(E) - brdc.e)
	double Vk = atan2(sqrt(1 - pow(brdc->e, 2)) * sin(Ek), (cos(Ek) - brdc->e));
	double uk = Vk + brdc->w;
	double cos_uk = cos(2 * uk), sin_uk = sin(2 * uk);
	uk += brdc->Cuc * cos_uk + brdc->Cus * sin_uk;
	double r = a * (1 - brdc->e * cos(Ek)) +
		brdc->Crc * cos_uk + brdc->Crs * sin_uk;

	double i = brdc->I0 + brdc->I0dot * tk + brdc->Cic * cos_uk + brdc->Cis * sin_uk;

	double x = r * cos(uk), y = r * sin(uk);
	double lambda = brdc->Omg0 + (brdc->Omg0dot - w_earth) * tk - w_earth * brdc->toe;
	Matd mx = Matd::creat(-i, Axis::xAxis);
	Matd mz = Matd::creat(-lambda, Axis::zAxis);
	Pointd xyz(x, y, 0);

	Pointd pt = mz * mx * xyz;

	double dt = (toc_jd - toe_jd) * 86400;
	double delta_t = brdc->a0 + brdc->a1 * dt + brdc->a2 * dt * dt;
    PositionPtr pos(new Position(pt, delta_t, date));
	pos->prn = brdc->prn;
	return pos;
}



PositionPtr CSConvertor::ehp2position(const std::vector<GPSEhpemerisPtr>& ehps, const Date& date) {
	double mindif = 7200;
	size_t minindex = -1;
	for (int i = 0; i < ehps.size(); i++) {
		double dif = Date(ehps[i]->toc).diff(date);
		if (abs(dif) <= abs(mindif)) {
			mindif = dif;
			minindex = i;
		}
	}

	if (abs(mindif) <= 7200) {
		return ehp2position(ehps[minindex], date);
	}
	else {
		printf("7200 error\n");
		return NULL;
	}
}



PositionPtr CSConvertor::ehp2position(const SP3_Ptr& sp3file, std::string prn, const Date& date) {
    std::vector<EpochPtr> EpochPtrs = sp3file->epochs;
    std::vector<PositionPtr> pos_ptrs;

	// generate (prn, positions) map
	// ith ephemeris 
    for (int epi = 0; epi < EpochPtrs.size(); epi++) {
        std::vector<PositionPtr> tmp_pos_ptrs = EpochPtrs[epi]->positions;
		// jth position/satellite
        for (size_t posj = 0; posj < tmp_pos_ptrs.size(); posj++) {
			if (tmp_pos_ptrs[posj]->prn == prn)
				pos_ptrs.push_back(tmp_pos_ptrs[posj]);
		}
	}

	// make date at the middle
	double mindif = 86400;
    int index = -1;
    for (int i = 0; i < pos_ptrs.size(); i++) {
		double dif = date.diff(pos_ptrs[i]->date);
		if (abs(dif) < abs(mindif)) {
			index = i;
			mindif = dif;
		}
	}

	if (index < 0)
		return NULL;
	if (index >= 0 && index < 5)
		index = 4;
	else if (index > 20)
		index = 20;
	
	std::vector<double> x, y, z, clock;
	std::vector<double> times;

    for (int i = index - 4; i <= index + 4; i++) {
		auto ptr = pos_ptrs[i];
		x.push_back(ptr->pt._x);
		y.push_back(ptr->pt._y);
		z.push_back(ptr->pt._z);
		clock.push_back(ptr->clock_error);

		// use difference to avoid overflow
		times.push_back(ptr->date.diff(date));
	}

	double sec = 0.0;
	double x_ = Interpolater::Lagrange(times, x, sec);
	double y_ = Interpolater::Lagrange(times, y, sec);
	double z_ = Interpolater::Lagrange(times, z, sec);
	double clc = Interpolater::Lagrange(times, clock, sec);

    return PositionPtr(new Position(Pointd(x_,y_,z_), clc, date));
}



std::vector<PositionPtr> CSConvertor::ehp2position(
    const GPSEhpemerisPtr& ephs, const std::vector<Date>& dates) {

    std::vector<PositionPtr> points;
	for (size_t k = 0; k < dates.size(); k++) {
		points.push_back(ehp2position(ephs, dates[k]));
	}
	return points;
}





PositionPtr GPSEhpemeris::compute(const Date& date){
    GPST toe_gpst(this->GPSweek, this->toe);
    double date_jd = date2JD(date);
    double toe_jd = GPST2JD(toe_gpst);
    double toc_jd = date2JD(this->toc);


    double tk = (date_jd - toe_jd) * 86400;
    double a = this->sqrt_a;
    a = a * a;
    double n0 = sqrt(GM / pow(a, 3));
    double n = n0 + this->delta_n;
    double Mk = this->M0 + n * tk;

    double Ek = Mk, Ek0 = 0.0;
    while (abs(Ek - Ek0) > 1.0e-12) {
        Ek0 = Ek;
        Ek = Mk + this->e * sin(Ek0);
    }
    //sqrt(1 - pow(this.e, 2))*sin(E) , (cos(E) - this.e)
    double Vk = atan2(sqrt(1 - pow(this->e, 2)) * sin(Ek), (cos(Ek) - this->e));
    double uk = Vk + this->w;
    double cos_uk = cos(2 * uk), sin_uk = sin(2 * uk);
    uk += this->Cuc * cos_uk + this->Cus * sin_uk;
    double r = a * (1 - this->e * cos(Ek)) +
        this->Crc * cos_uk + this->Crs * sin_uk;

    double i = this->I0 + this->I0dot * tk + this->Cic * cos_uk + this->Cis * sin_uk;

    double x = r * cos(uk), y = r * sin(uk);
    double lambda = this->Omg0 + (this->Omg0dot - w_earth) * tk - w_earth * this->toe;
    Matd mx = Matd::creat(-i, Axis::xAxis);
    Matd mz = Matd::creat(-lambda, Axis::zAxis);
    Pointd xyz(x, y, 0);

    Pointd pt = mz * mx * xyz;

    double dt = (toc_jd - toe_jd) * 86400;
    double delta_t = this->a0 + this->a1 * dt + this->a2 * dt * dt;
    PositionPtr pos(new Position(pt, delta_t, date));
    pos->prn = this->prn;
    return pos;
}






// ------------------------------------ split ---------------------------------------------------------------------------

void f2str(std::stringstream& ss, const double& val1,
           const double& val2, const double& val3, const double& val4) {
	ss.str("");
	std::stringstream tmp;
	tmp.precision(6);
	tmp.setf(std::ios::fixed);

	if (val1 >= 0)
	{
		tmp << " ";
		ss << " ";
	}
	ss << " ";
	tmp << val1;

	std::string t = tmp.str();
	int len = tmp.str().length();
	for (int i = 0; i < 13 - len; i++) {
		ss << " ";
	}
	ss << val1;
	
	tmp.str("");
	if (val2 >= 0)
	{
		tmp << " ";
		ss << " ";
	}
	ss << " ";
	tmp << val2;
	len = tmp.str().length();
	for (int i = 0; i < 13 - len; i++) {
		ss << " ";
	}
	ss << val2;

	tmp.str("");
	if (val3 >= 0)
	{
		tmp << " ";
		ss << " ";
	}
	ss << " ";
	tmp << val3;
	len = tmp.str().length();
	for (int i = 0; i < 13 - len; i++) {
		ss << " ";
	}
	ss << val3;


	tmp.str("");
	if (val4 >= 0)
	{
		tmp << " ";
		ss << " ";
	}
	ss << " ";
	tmp << val4;
	len = tmp.str().length();
	for (int i = 0; i < 13 - len; i++) {
		ss << " ";
	}
	ss << val4;
}



// can only read gps-like brdc file
EhpemerisMap readBRDC(const char* str) {
    EhpemerisMap m;
	std::ifstream ifs(str);
	if (!ifs.is_open()) {
        return m;
	}

	std::string line;
	while (true) {
		std::getline(ifs, line);
		if (trim(line) == "END OF HEADER")
			break;
	}

	while (std::getline(ifs, line)) {
		
        GPSEhpemerisPtr brdc(new GPSEhpemeris);

		brdc->toc.year = atoi(line.substr(3, 2).c_str()) + 2000;//20xx
		brdc->toc.month = atoi(line.substr(6, 2).c_str());
		brdc->toc.day = atoi(line.substr(9, 2).c_str());
		brdc->toc.hour = atoi(line.substr(12, 2).c_str());
		brdc->toc.minute = atoi(line.substr(15, 2).c_str());
		brdc->toc.second = atof(line.substr(18, 4).c_str());

		brdc->prn = line.substr(0, 2);
        brdc->a0 = _str2f(line.substr(22, 19));
        brdc->a1 = _str2f(line.substr(41, 19));
        brdc->a2 = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 2
        brdc->IODE = _str2f(line.substr(3, 19));
        brdc->Crs = _str2f(line.substr(22, 19));
        brdc->delta_n = _str2f(line.substr(41, 19));
        brdc->M0 = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 3
        brdc->Cuc = _str2f(line.substr(3, 19));
        brdc->e = _str2f(line.substr(22, 19));
        brdc->Cus = _str2f(line.substr(41, 19));
        brdc->sqrt_a = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 4
        brdc->toe = _str2f(line.substr(3, 19));
        brdc->Cic = _str2f(line.substr(22, 19));
        brdc->Omg0 = _str2f(line.substr(41, 19));
        brdc->Cis = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 5
        brdc->I0 = _str2f(line.substr(3, 19));
        brdc->Crc = _str2f(line.substr(22, 19));
        brdc->w = _str2f(line.substr(41, 19));
        brdc->Omg0dot = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 6
        brdc->I0dot = _str2f(line.substr(3, 19));
        brdc->codeL2 = _str2f(line.substr(22, 19));
        brdc->GPSweek = _str2f(line.substr(41, 19));
        brdc->L2flag = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 7
        brdc->accuracy = _str2f(line.substr(3, 19));
        brdc->health = _str2f(line.substr(22, 19));
        brdc->TGD = _str2f(line.substr(41, 19));
        brdc->IODC = _str2f(line.substr(60, 19));

		std::getline(ifs, line);// line 8
        brdc->trans_time_of_message = _str2f(line.substr(3, 19));
        brdc->fit_interval = _str2f(line.substr(22, 19));

		m[brdc->prn].push_back(brdc);
	}

	ifs.close();
	return m;
}




SP3_Ptr readSP3(const char* str) {
	SP3_Ptr sp3(new SP3);
	std::ifstream ifs(str);
	if (!ifs.is_open()) {
		return sp3;
	}

	std::string line;
	// 1-th
	std::getline(ifs, line);
	sp3->mode = line[2];
	sp3->year_start = std::stoi(line.substr(3, 4));
	sp3->month_start = std::stoi(line.substr(8, 2));
	sp3->day_start = std::stoi(line.substr(11, 2));
	sp3->hour_start = std::stoi(line.substr(14, 2));
	sp3->minute_start = std::stoi(line.substr(17, 2));
	sp3->second_start = std::stod(line.substr(20, 11));

	// 2-th
	std::getline(ifs, line);
	sp3->GPS_WEEK = std::stol(line.substr(3, 4));
	sp3->sec_of_week = std::stod(line.substr(8, 15));
	sp3->epoch_interval = std::stod(line.substr(24, 14));
	sp3->MJD = std::stol(line.substr(39, 5));
	sp3->frac_day = std::stod(line.substr(45, 15));

	// 3-7 th
	std::getline(ifs, line);
	sp3->number_of_satellite = std::stol(line.substr(4, 2));
	while (line[0] == '+'){
		std::getline(ifs, line);
	}
	
	// 8-22 th
	while (line[0] != '*') {
		std::getline(ifs, line);
	} 

	
	// 23-END th
	size_t n = -1;
	while (trim(line) != "EOF") {

		if (line[0] != '*') {
			double x = std::stod(line.substr(4, 14));
			double y = std::stod(line.substr(18, 14));
			double z = std::stod(line.substr(32, 14));
			double clock = std::stod(line.substr(46, 14));


            PositionPtr pos(new Position(Pointd(x, y, z), clock, sp3->epochs[n]->date_start));
			pos->prn = line.substr(1, 3);
			sp3->epochs[n]->positions.push_back(pos);
		}
		// next epoch
		else {
			// 23-th
			Date date;
			date.year = std::stoi(line.substr(3, 4));
			date.month = std::stoi(line.substr(8, 2));
			date.day = std::stoi(line.substr(11, 2));
			date.hour = std::stoi(line.substr(14, 2));
			date.minute = std::stoi(line.substr(17, 2));
			date.second = 0.0;
            EpochPtr epoch(new Epoch(date));
			sp3->epochs.push_back(epoch);
			n++;
		}
		std::getline(ifs, line);
	}
	
	return sp3;
}



}
