#include "date.h"

namespace lyz {
const double JD_OF_GPS_START = 2444244.5;
const double SEC_PER_DAY = 86400;

// ----------------------------------------------

//return difference(seconds)
double Date::date_sec_diff(const Date& date1, const Date& date2) {
    double g1 = date2MJD(date1) * 86400;
    double g2 = date2MJD(date2) * 86400;
    return g1 - g2;
}


double Date::date2JD(const Date& date){
    int Y = date.year;
    int M = date.month;
    int D = date.day;
    double H = date.hour + date.minute / 60.0 + date.second / 3600.0;
    int m = M <= 2 ? M + 12 : M;
    int y = M > 2 ? Y : Y - 1;
    return int(365.25 * y) + int(30.6001 * (m + 1))
            + D + H / 24.0 + 1720981.5;
}


double Date::date2MJD(const Date& date){
    return date2JD(date) - 2400000.5;
}






WeekSecond Date::JD2GPST(double jd) {
    double week = int((jd - JD_OF_GPS_START) / 7);
    double sec = (jd - JD_OF_GPS_START - 7 * week) * SEC_PER_DAY;
    return WeekSecond(week, sec);
}


WeekSecond Date::JD2BDST(double jd){
    return(GPST2BDST(JD2GPST(jd)));
}


WeekSecond Date::date2GPST(const Date& date) {
    return JD2GPST(date2JD(date));
}


WeekSecond Date::GPST2BDST(const WeekSecond& gpst){
    WeekSecond t(gpst.week-1356, gpst.second-14);
    if (t.second < 0 && (t.second += 604800))
        t.week -= 1;
    return t;
}


WeekSecond Date::BDST2GPST(const WeekSecond& bdst){
    WeekSecond gpst(bdst.week+1356, bdst.second+14);
    if (gpst.second > 604800 && (gpst.second -= 604800))
        gpst.week -= 1;
    return gpst;
}


WeekSecond Date::date2BDST(const Date& date){
    WeekSecond gpst = date2GPST(date);
    return GPST2BDST(gpst);
}


double Date::GPST2JD(const WeekSecond& WeekSecond) {
    return WeekSecond.week * 7 +
            WeekSecond.second / 86400 +
            2444244.5;
}


double Date::BDST2JD(const WeekSecond& bdst){
    return  Date::GPST2JD(Date::BDST2GPST(bdst));
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
    JD = Date::date2JD(*this);
}


Date::Date(int y, int m, int d, double h){ *this = Date(y,m,d,h,0,0); }


Date::Date(int y, int m, int d, int h,int minute){ *this = Date(y,m,d,h,minute,0); }


// copy constructor
Date::Date(const Date& date) {
    *this = Date(date.year, date.month, date.day,
         date.hour, date.minute, date.second);

}


double Date::diff(const Date& date)const {
    return Date::date_sec_diff(*this, date);
}


double Date::toJD(){ return  Date::date2JD(*this);}


double Date::toMJD(){ return  Date::date2MJD(*this);}


bool Date::operator < (const Date& rhs){
    return Date::date2JD(*this) < Date::date2JD(rhs);
}


bool Date::operator > (const Date& rhs){
    return Date::date2JD(*this) > Date::date2JD(rhs);
}


bool Date::operator == (const Date& rhs){
    return Date::date2JD(*this) == Date::date2JD(rhs);
}



}
