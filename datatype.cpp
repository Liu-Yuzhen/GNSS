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
const double c = 3e8;




/*
class Epoch(const Date&)
*/
//Epoch::Epoch(const Date& date):date_start(date){}



/*
class Position
*/
Position::Position(Pointd pt):pt(pt), clock_error(0.0) {}


Position::Position(Pointd pt, double clock_erorr, const Date& date) :
	pt(pt), clock_error(clock_erorr), date(date) {}
 





PositionPtr GPSEhpemeris::compute(const Date& date){

    WeekSecond toe_WeekSecond(this->GPSweek, this->toe);
    double date_jd = Date::date2JD(date);
    double toe_jd = toe2jd(toe_WeekSecond);
    double toc_jd = Date::date2JD(this->toc);


    double tk = (date_jd - toe_jd) * 86400;
    double a = this->sqrt_a;
    a = a * a;
    double n0 = sqrt(GM / pow(a, 3));
    double n = n0 + this->delta_n;
    double Mk = this->M0 + n * tk;

    double Ek = Mk, Ek0 = 0.0;
    int count = 0;
    while (abs(Ek - Ek0) > 1.0e-8 && ++count < 20) {
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

    double dt = (date_jd - toc_jd) * 86400;
    double delta_t = this->a0 + this->a1 * dt + this->a2 * dt * dt;
    // relative effect correction
    double F = - 2 * sqrt(GM) / pow(c, 2);
    delta_t += F * e * sqrt_a * sin(Ek);
    PositionPtr pos(new Position(pt*1e-3, delta_t, date));
    pos->prn = this->prn;
    return pos;

}



double GPSEhpemeris::toe2jd(const WeekSecond& wksec){
    return Date::GPST2JD(wksec);;
}



//bds
double GPSBEhpemeris::toe2jd(const WeekSecond& wksec){
    return Date::BDST2JD(wksec);;
}



PositionPtr SBASEhpemeris::compute(const Date& date){
    return nullptr;
}


PositionPtr GLONASSEphemeris::compute(const Date& date){
    return nullptr;
}






}
