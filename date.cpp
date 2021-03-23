#include "date.h"

namespace lyz {
const double JD_OF_GPS_START = 2444244.5;
const double SEC_PER_DAY = 86400;

// ----------------------------------------------

//return difference(seconds)
double date_sec_diff(const Date& date1, const Date& date2) {
    double g1 = date2JD(date1) * 86400;
    double g2 = date2JD(date2) * 86400;
    return g1 - g2;
}


double date2JD(const Date& date){
    int Y = date.year;
    int M = date.month;
    int D = date.day;
    double H = date.hour + date.minute / 60.0 + date.second / 3600.0;
    int m = M <= 2 ? M + 12 : M;
    int y = M > 2 ? Y : Y - 1;
    return int(365.25 * y) + int(30.6001 * (m + 1))
            + D + H / 24.0 + 1720981.5;
}


double date2MJD(const Date& date){
    return date2JD(date) - 2400000.5;
}



GPST JD2GPST(double jd) {
    double week = int((jd - JD_OF_GPS_START) / 7);
    double sec = (jd - JD_OF_GPS_START - 7 * week) * SEC_PER_DAY;
    return GPST(week, sec);
}


GPST date2GPST(const Date& date) {
    return JD2GPST(date2JD(date));
}


double GPST2JD(const GPST& gpst) {
    return gpst.week * 7 +
            gpst.second / 86400 +
            2444244.5;
}


// ----------------------------------------------

/*
class Date
*/
Date::Date(int y, int m, int d, int h, int minute, double second) :
    year(y), month(m), day(d), hour(h), minute(minute), second(second) {

    hour += int(minute / 60);
    day += int(hour / 24);

    hour = hour - 24 * (hour / 24);
    this->minute = minute - 60 * (minute / 60);

    // compute JD
//    double H = hour + minute / 60.0 + second / 3600.0;
//    int mm = m <= 2 ? m + 12 : m;
//    int yy = m > 2 ? y : y - 1;
//    JD = int(365.25 * yy) + int(30.6001 * (mm + 1)) +
//            day + hour / 24.0 + 1720981.5;
    JD = date2JD(*this);
}


Date::Date(int y, int m, int d, double h){ *this = Date(y,m,d,h,0,0); }


Date::Date(int y, int m, int d, int h,int minute){ *this = Date(y,m,d,h,minute,0); }


// copy constructor
Date::Date(const Date& date) {
    *this = Date(date.year, date.month, date.day,
         date.hour, date.minute, date.second);

}


double Date::diff(const Date& date)const {
    return date_sec_diff(*this, date);
}


double Date::toJD(){ return date2JD(*this);}


double Date::toMJD(){ return date2MJD(*this);}


/*
class GPST
*/
GPST::GPST(int week, double sec):week(week), second(sec){}



}
