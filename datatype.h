#ifndef DATATYPE_H
#define DATATYPE_H
#include <map>
#include <vector>
#include <string>
#include <QSharedPointer>
#include "mat.h"
#include "date.h"


namespace lyz {
	
/**************************************************/
class Position {
public:
    Position(Pointd pt);
    Position(Pointd pt, double clock_erorr,
             const Date& date);


    Pointd pt;
    std::string prn;
    double clock_error;
    Date date;
};
typedef QSharedPointer<Position> PositionPtr;


/*
several positions(satellite) per Epoch
*/
class Epoch
{
public:
    Epoch(const Date&);
    Date date_start;
    std::vector<PositionPtr> positions;
};
typedef QSharedPointer<Epoch> EpochPtr;



class Ephemeris
{
public:
    virtual PositionPtr compute(const Date&) = 0;
    std::string prn;
    Date toc;
};
typedef QSharedPointer<Ephemeris> EhpemerisPtr;
typedef std::map<std::string, std::vector<EhpemerisPtr>> EhpemerisMap;
typedef EhpemerisMap::const_iterator GPSephs_const_iterator;


class GPSEhpemeris: public Ephemeris
{
public:
    virtual PositionPtr compute(const Date&);


	double a0, a1, a2;
	double IODE;
	double Crc, Crs;
	double Cuc, Cus;
	double Cic, Cis;
	double delta_n;
	double M0;
	double e;
	double sqrt_a;
	double toe;
	double Omg0;
	double I0;
	double w;//perigee
	double I0dot;
	double Omg0dot;
	double GPSweek;
	double TGD;
	double IODC;
	double codeL2;
	double L2flag;
	double accuracy;
	double health;
	double trans_time_of_message;
	double fit_interval;
};
typedef QSharedPointer<GPSEhpemeris> GPSEhpemerisPtr;



class GLONASSEphemeris: public Ephemeris
{
    virtual PositionPtr compute(const Date&);
};
typedef QSharedPointer<GLONASSEphemeris> GLONASSEhpemerisPtr;
/**************************************************/






// .sp3 file format
class SP3
{
public:
	std::string version;
	char mode;// P or V
    int year_start, month_start, day_start,
    hour_start, minute_start;

	double second_start;
	int number_of_epoch;

	std::string cs;
	std::string agency;

	long GPS_WEEK;
	double sec_of_week;
	double epoch_interval;
	long MJD;
	double frac_day;
	int number_of_satellite;
    std::vector<EpochPtr> epochs;
};
typedef QSharedPointer<SP3> SP3_Ptr;


class CSConvertor
{
public:

    /*
     *
     * input: GPS ehpemeris, date(one day only)
     * return: position

    */
    static PositionPtr ehp2position(
            const GPSEhpemerisPtr& ehp,
            const Date& date);


	// find the nearest to compute coordinate
    static PositionPtr ehp2position(
            const std::vector<GPSEhpemerisPtr>& ehps,
                  const Date& date);


    static PositionPtr ehp2position(
            const SP3_Ptr& sp3file,
            std::string prn,
           const Date& date);



    static std::vector<PositionPtr> ehp2position(
            const GPSEhpemerisPtr& ehp,
            const std::vector<Date>& dates);

};



/*
transfer to GNSSFile
*/

EhpemerisMap readBRDC(const char* path);
SP3_Ptr readSP3(const char* path);


}
#endif // !DATALOADER_H
