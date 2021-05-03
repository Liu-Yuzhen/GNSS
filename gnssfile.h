#ifndef GNSSFILE_H
#define GNSSFILE_H
#include "datatype.h"
#include <string>
#include <vector>

namespace lyz{


struct SP3Head{
    // line 1
    char mode; // 'P' or 'V'
    Date date_start;
    int epoch_number;


    // line 2
    WeekSecond weeksec;
    double epoch_interval;
    double time_start_mjd;
    double fraction_day;

    // line 3
    double satellite_number;
    std::vector<std::string> satellite_names;
    std::vector<int> satellite_accuracy;

    int read(std::ifstream& ifs);
    int write(std::ofstream& ofs);
};


struct SP3Record{
    Date date;
    std::vector<PositionPtr> poses;
    std::vector<char> modes;

    const PositionPtr getPos(const std::string& prn);

    int read(std::ifstream& ifs);
    int write(std::ofstream& ofs);
};


struct ObsHead{
    std::string version;
    char mode;
    std::vector<std::string> types_of_obs;
    Date time_first_obs;
    double interval;

    int read(std::ifstream& ifs);
};


struct ObsRecord{
    Date date;

    std::map<std::string, std::vector<double>> map_;

    int read(std::ifstream& ifs);
};


class ObsFile
{
public:
    ObsFile(){}
    ObsFile(const std::string& path){ open(path);}
    ObsFile(const char* path){ open(std::string(path)); }
    int open(const std::string& path);
    bool isopen() { return !_records.empty(); }

    Date getDateStart(){ return _head.time_first_obs; }
    Date getClosetDate(const Date& date);
    void getSeudoRanges(
            const Date& date,
            std::vector<double>& ranges,
            std::vector<std::string>& prns);
    void getValues(const Date& date, std::string code,
                         std::vector<double>& phases,
                         std::vector<std::string>& prns);
    std::string path(){ return _path; }
    int codeIndex(const std::string& code);
    std::vector<std::string> getObsType(){
        return _head.types_of_obs;
    }
    Date getDate(){ return _head.time_first_obs; }


private:
    std::string _path;
    ObsHead _head;
    std::vector<ObsRecord> _records;
};


class GNSSFile
{
public:
    GNSSFile(const std::string& path);
    GNSSFile(const char* path);
    GNSSFile();
    virtual ~GNSSFile(){}

    virtual int open(const std::string& path) = 0;
    virtual bool isopen() = 0;

    virtual PositionPtr compute(const
            std::string& prn, Date date) = 0;


    virtual int writeSP3(const std::string& path,
                          int minute) = 0;
    virtual int writeSP3(const char* path,
                          int minute) = 0;
    virtual std::vector<std::string> getPrn() = 0;
    virtual Date getDateStart() = 0;
    std::string path(){ return _path; }


protected:
    std::string _path;
};



class BRDCFile: public GNSSFile
{
public:
    BRDCFile(const std::string& path):GNSSFile(path){ open(path);}
    BRDCFile(const char* path):GNSSFile(path){ open(std::string(path)); }
    BRDCFile(){}
    virtual ~BRDCFile(){}
    virtual int open(const std::string& path);
    virtual bool isopen(){ return !_map.empty(); }
    virtual PositionPtr compute(
            const std::string& prn,
            Date date);

    virtual std::vector<std::string> getPrn();
    virtual Date getDateStart();
    virtual int writeSP3(const char* path,
                          int minute = 5);
    virtual int writeSP3(const std::string& path,
                          int minute = 5);

private:
    std::string _version;
    char _satellie_system;
    EhpemerisMap _map;
};


class SP3File: public GNSSFile
{
public:
    SP3File(const std::string& path):GNSSFile(path){ open(path);}
    SP3File(const char* path):GNSSFile(path){ open(std::string(path)); }
    SP3File(){}
    virtual ~SP3File(){}
    virtual int open(const std::string& path);

    virtual bool isopen(){ return opened; }

    virtual PositionPtr compute(
            const std::string& prn,
            Date date);


    virtual std::vector<std::string> getPrn();
    virtual Date getDateStart(){ return head.date_start; }
    virtual int writeSP3(const char* path, int minute = 5);
    virtual int writeSP3(const std::string& path,int minute = 5);

private:
    bool opened;

    SP3Head head;
    std::vector<SP3Record> records;
};


}
#endif // GNSSFILE_H
