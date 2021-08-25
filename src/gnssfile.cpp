#include "gnssfile.h"
#include "interpolater.h"
#include "Eigen/Dense"
#include <math.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <QDebug>
#include <QString>
#include <map>


namespace lyz{
std::string _line;
size_t row_per_entry;
size_t obs_total;

inline std::string trim(const std::string str) {
    size_t start = str.find_first_not_of(" ");
    size_t end = str.find_last_not_of(" ") + 1;
    return str.substr(start, end - start);
}


// from "1.231234D-02"-like string to float
inline double str2f(const std::string& str) {
    int len = str.length();
    double d = atof(str.substr(0, len - 4).c_str());
    int expo = atof(str.substr(len - 3).c_str());
    return d * pow(10, expo);
}




void read_brdc_gps_like_entry_v2(
        std::ifstream& ifs,EhpemerisMap& m,
        std::string& line){

    GPSEhpemerisPtr ehp(new GPSEhpemeris);

    ehp->prn = 'G';
    // pad zero
    if (line[0] == ' ')
        ehp->prn += "0";
    ehp->prn += trim(line.substr(0, 2));

    ehp->toc.year = atoi(line.substr(3, 2).c_str()) + 2000;//20xx
    ehp->toc.month = atoi(line.substr(6, 2).c_str());
    ehp->toc.day = atoi(line.substr(9, 2).c_str());
    ehp->toc.hour = atoi(line.substr(12, 2).c_str());
    ehp->toc.minute = atoi(line.substr(15, 2).c_str());
    ehp->toc.second = atof(line.substr(18, 4).c_str());


    ehp->a0 = str2f(line.substr(22, 19));
    ehp->a1 = str2f(line.substr(41, 19));
    ehp->a2 = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 2
    ehp->IODE = str2f(line.substr(3, 19));
    ehp->Crs = str2f(line.substr(22, 19));
    ehp->delta_n = str2f(line.substr(41, 19));
    ehp->M0 = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 3
    ehp->Cuc = str2f(line.substr(3, 19));
    ehp->e = str2f(line.substr(22, 19));
    ehp->Cus = str2f(line.substr(41, 19));
    ehp->sqrt_a = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 4
    ehp->toe = str2f(line.substr(3, 19));
    ehp->Cic = str2f(line.substr(22, 19));
    ehp->Omg0 = str2f(line.substr(41, 19));
    ehp->Cis = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 5
    ehp->I0 = str2f(line.substr(3, 19));
    ehp->Crc = str2f(line.substr(22, 19));
    ehp->w = str2f(line.substr(41, 19));
    ehp->Omg0dot = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 6
    ehp->I0dot = str2f(line.substr(3, 19));
    ehp->codeL2 = str2f(line.substr(22, 19));
    ehp->GPSweek = str2f(line.substr(41, 19));
    ehp->L2flag = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 7
    ehp->accuracy = str2f(line.substr(3, 19));
    ehp->health = str2f(line.substr(22, 19));
    ehp->TGD = str2f(line.substr(41, 19));
    ehp->IODC = str2f(line.substr(60, 19));

    std::getline(ifs, line);// _line 8
    ehp->trans_time_of_message = str2f(line.substr(3, 19));
    ehp->fit_interval = str2f(line.substr(22, 19));

    m[ehp->prn].push_back(ehp);
}


void read_brdc_gps_like_entry_v3(
        std::ifstream& ifs,EhpemerisMap& m,
        std::string& line, bool isgps = true){

    GPSEhpemeris* ehppstd;
    if (isgps)
        ehppstd = new GPSEhpemeris;
    else
        ehppstd = new GPSBEhpemeris;

    GPSEhpemerisPtr ehp(ehppstd);
    ehp->prn += trim(line.substr(0, 3));

    int year = atoi(line.substr(4, 4).c_str());//20xx
    int month = atoi(line.substr(9, 2).c_str());
    int day = atoi(line.substr(12, 2).c_str());
    int hour = atoi(line.substr(15, 2).c_str());
    int minute = atoi(line.substr(18, 2).c_str());
    double second = atof(line.substr(21, 4).c_str());
    ehp->toc = Date(year, month, day, hour, minute, second);

    ehp->a0 = str2f(line.substr(23, 19));
    ehp->a1 = str2f(line.substr(42, 19));
    ehp->a2 = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 2
    ehp->IODE = str2f(line.substr(4, 19));
    ehp->Crs = str2f(line.substr(23, 19));
    ehp->delta_n = str2f(line.substr(42, 19));
    ehp->M0 = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 3
    ehp->Cuc = str2f(line.substr(4, 19));
    ehp->e = str2f(line.substr(23, 19));
    ehp->Cus = str2f(line.substr(42, 19));
    ehp->sqrt_a = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 4
    ehp->toe = str2f(line.substr(4, 19));
    ehp->Cic = str2f(line.substr(23, 19));
    ehp->Omg0 = str2f(line.substr(42, 19));
    ehp->Cis = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 5
    ehp->I0 = str2f(line.substr(4, 19));
    ehp->Crc = str2f(line.substr(23, 19));
    ehp->w = str2f(line.substr(42, 19));
    ehp->Omg0dot = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 6
    ehp->I0dot = str2f(line.substr(4, 19));
    ehp->codeL2 = str2f(line.substr(23, 19));
    ehp->GPSweek = str2f(line.substr(42, 19));
    ehp->L2flag = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 7
    ehp->accuracy = str2f(line.substr(4, 19));
    ehp->health = str2f(line.substr(23, 19));
    ehp->TGD = str2f(line.substr(42, 19));
    ehp->IODC = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 8
    ehp->trans_time_of_message = str2f(line.substr(4, 19));
    ehp->fit_interval = str2f(line.substr(23, 19));

    m[ehp->prn].push_back(ehp);
}



void read_brdc_sbas_entry_v3(
        std::ifstream& ifs, EhpemerisMap& m,
        std::string& line){

    SBASEhpemerisPtr ehp(new SBASEhpemeris);

    // line 1
    ehp->prn = line.substr(0, 3);
    int year = atoi(line.substr(4, 4).c_str());
    int month = atoi(line.substr(9, 2).c_str());
    int day = atoi(line.substr(12, 2).c_str());
    int hour = atoi(line.substr(15, 2).c_str());
    int minute = atoi(line.substr(18, 2).c_str());
    double second = atof(line.substr(21, 4).c_str());
    ehp->toc = Date(year, month, day, hour, minute, second);

    ehp->clock_bias = str2f(line.substr(23, 19));
    ehp->frequency_bias = str2f(line.substr(42, 19));
    ehp->trans_time_of_message = str2f(line.substr(61, 19));

    // _line 2
    std::getline(ifs, line);
    ehp->x = str2f(line.substr(4, 19));
    ehp->vx = str2f(line.substr(23, 19));
    ehp->ax = str2f(line.substr(42, 19));
    ehp->health = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 3
    ehp->y = str2f(line.substr(4, 19));
    ehp->vy = str2f(line.substr(23, 19));
    ehp->ay = str2f(line.substr(42, 19));
    ehp->accuracy = str2f(line.substr(61, 19));

    std::getline(ifs, line);// _line 4
    ehp->z = str2f(line.substr(4, 19));
    ehp->vz = str2f(line.substr(23, 19));
    ehp->az = str2f(line.substr(42, 19));
    ehp->IODN = str2f(line.substr(61, 19));

    m[ehp->prn].push_back(ehp);
}


inline void jump_entry(std::ifstream& ifs, int n){
    for (int i = 0; i < n; i++){
        std::getline(ifs, _line);
    }
}


inline std::string float_align_end(
        double val,
        const int& length,
        const int& decimal){

    std::stringstream ss;
    ss << std::setiosflags(std::ios::fixed|std::ios::showpoint)
       << std::setprecision(decimal) << val;

    int speclen = length - ss.str().length();
    if (speclen > 0){
        std::string space(speclen, ' ');
        return space + ss.str();
    }
    else
        return ss.str();

}


inline std::string int_align_end(int val, int length){
    std::stringstream ss;
    ss << val;
    int speclen = length - ss.str().length();
    if (speclen > 0){
        std::string space(speclen, ' ');
        return space + ss.str();
    }
    else
        return ss.str();

}


inline std::vector<double> read_entry(
        std::ifstream& ifs){
    std::vector<double> values;
    size_t count = 0;

    for (size_t row = 0; row < row_per_entry; row++){
        for (size_t i = 0; i < 5; i++){
            size_t start = i*16;
            if (start >= _line.length())
                values.push_back(0);
            else
                values.push_back(atof(_line.substr(start,14).c_str()));
            ++count;
            if (count == obs_total)
                break;
        }
        std::getline(ifs, _line);
    }
    return values;
}


/*
 * return 0: success
 * return -1: not open
 * return -2: unknown file format
*/
int ObsHead::read(std::ifstream &ifs){
    if (!ifs.is_open())
        return -1;

    std::getline(ifs, _line);

    version = trim(_line.substr(0, 10));
    if (version != "2.11")
        return -2;
    mode = _line[40];

    while(_line.substr(60) !=
          "# / TYPES OF OBSERV"){
        std::getline(ifs, _line);
    }

    int total = atoi(_line.substr(0, 6).c_str());
    int rows = ceil(atof(_line.substr(0, 6).c_str())/9.0);
    int count = 0;
    for (size_t i = 0; i < rows; i++){
        for (size_t j = 0; j < 9; j++){
            types_of_obs.push_back(_line.substr(10+j*6, 2));
            if (++count >= total)
                break;

        }
        std::getline(ifs, _line);
    }

    while(_line.substr(60) !=
          "INTERVAL"){
        std::getline(ifs, _line);
    }

    interval = atof(_line.substr(0,11).c_str());

    while (std::getline(ifs, _line)){
        if (trim(_line) == "END OF HEADER")
            break;
    }

    row_per_entry = ceil(types_of_obs.size() / 5.0);
    obs_total = types_of_obs.size();

    std::getline(ifs, _line);
    return 0;
}


int ObsRecord::read(std::ifstream &ifs){
    // eof
    if (_line=="")
        return 0;
    if (_line == "                            4  1"){
        std::getline(ifs, _line);
        std::getline(ifs, _line);
    }

    int year = atoi(_line.substr(1, 2).c_str()) + 2000;
    int month = atoi(_line.substr(4, 2).c_str());
    int day = atoi(_line.substr(7, 2).c_str());
    int hour = atoi(_line.substr(10, 2).c_str());
    int min = atoi(_line.substr(13, 2).c_str());
    double sec = str2f(_line.substr(15, 10));
    date = Date(year, month, day, hour, min, sec);

    size_t total = atoi(_line.substr(29, 3).c_str());
    size_t rows = ceil(total / 12.0);
    size_t count = 0;

    std::vector<std::string> satellite_names;
    for (size_t row = 0; row < rows; row++){
        for (size_t col = 0; col < 12; col++){
            satellite_names.push_back(
                        _line.substr(32+col*3, 3));
            ++count;
            if (count == total)
                break;
        }
        std::getline(ifs, _line);
    }

    for (size_t i = 0; i < satellite_names.size(); i++){
        map_[satellite_names[i]] = read_entry(ifs);
    }


    return 1;
}



/*
 * return 0: success
 * return -1: not open
 * return -2: unknown file format
*/
int ObsFile::open(const std::string& path){
    _path = path;

    std::ifstream ifs(path);
    if (!ifs.is_open())
        return -1;

    if (_head.read(ifs) == -2){
        return -2;
    }


    while(true){
        ObsRecord record;
        if (record.read(ifs))
            _records.push_back(record);
        else
            break;

    }

    _head.time_first_obs = _records[0].date;
    return 0;
}



void ObsFile::getSeudoRanges(
        const Date& date,
        std::vector<double>& ranges,
        std::vector<std::string>& prns){

//    if (date < _head.time_first_obs ||
//            date - _head.time_first_obs > 86400)
//        return;

    for (size_t i = 0; i < _records.size(); i++){
        ObsRecord record = _records[i];
        if (record.date == date){

            // find seudo range index
            std::vector<size_t> indecs;

            for (size_t j = 0; j < _head.types_of_obs.size(); j++){
                char mode = _head.types_of_obs[j][0];
                if (mode == 'C' || mode == 'P'){
                    indecs.push_back(j);
                }
            }

            // get ranges and prns
            for (std::map<std::string,
                 std::vector<double>>::iterator it = record.map_.begin();
                 it != record.map_.end(); it++){


                std::vector<double> values = it->second;

                // maybe blank
                size_t index = 0;
                while (values[indecs[index]] == 0.0)
                    index++;



                ranges.push_back(values[indecs[index]]);
                prns.push_back(it->first);
            }


            break;
        }
    }



}


void ObsFile::getValues(
        const Date& date,
        std::string code,
        std::vector<double>& phases,
        std::vector<std::string>& prns)
{
    for (size_t i = 0; i < _records.size(); i++){
        ObsRecord record = _records[i];
        if (record.date == date){

            // find seudo range index
            int index = -1;

            for (size_t j = 0; j < _head.types_of_obs.size(); j++){
                std::string mode = _head.types_of_obs[j];
                if (mode == code){
                    index = j;
                }
            }

            if (index < 0)
                return;

            // get phases and prns
            for (std::map<std::string,
                 std::vector<double>>::iterator it = record.map_.begin();
                 it != record.map_.end(); it++){


                std::vector<double> values = it->second;
                double phase = values[index];
                if (phase == 0.0)
                    continue;
                phases.push_back(phase);
                prns.push_back(it->first);
            }


            break;
        }
    }
}


int ObsFile::codeIndex(const std::string& code){
    for (size_t i = 0; i < _head.types_of_obs.size();i++){
        if (_head.types_of_obs[i] == code)
            return i;
    }
    return -1;
}


Date ObsFile::getClosetDate(const Date& date){
    Date mindate = _records[0].date;
    double mindif = mindate - date;
    for (size_t i = 1; i < _records.size(); i++){
        double dif = _records[i].date - date;
        if (dif >= 0){
            if (abs(dif) < abs(mindif)){
                mindate = _records[i].date;
            }
            break;
        }
        else{
            mindate = _records[i].date;
            mindif = dif;
        }

    }

    return mindate;
}

/*
 * return 0: success
 * return -1: not open
 * return -2: unknown file format
*/
int SP3Head::read(std::ifstream& ifs){
    if (!ifs.is_open())
        return -1;

    // line 1
    std::getline(ifs, _line);
    if (_line[0] != '#')
        return -2;
    mode = _line[2];

    int year = atoi(_line.substr(3, 4).c_str());
    int month = atoi(_line.substr(8, 2).c_str());
    int day = atoi(_line.substr(11, 2).c_str());
    int hour = atoi(_line.substr(14, 2).c_str());
    int minute = atoi(_line.substr(17, 2).c_str());
    double second = std::stod(_line.substr(20, 11).c_str());

    date_start = Date(year, month, day, hour, minute, second);
    epoch_number = atoi(_line.substr(32, 7).c_str());

    // line 2
    std::getline(ifs, _line);
    int wk = atoi(_line.substr(3, 7).c_str());
    double sec = atof(_line.substr(8, 15).c_str());
    weeksec = WeekSecond(wk, sec);
    epoch_interval = atof(_line.substr(24,14).c_str());
    time_start_mjd = atof(_line.substr(39, 5).c_str());
    fraction_day = atof(_line.substr(45, 15).c_str());

    // line 3
    std::getline(ifs, _line);
    satellite_number = atoi(_line.substr(4, 2).c_str());

    int count = 0;
    // names
    for (int ln = 0; ln < 5; ln++){

        for (int i = 0; i < 17; i++){
            if (count == satellite_number)
                break;

            ++count;
            int start = i * 3 + 9;
            satellite_names.push_back(_line.substr(start,3));
        }
        std::getline(ifs, _line);
    }

    count = 0;
    // accuracy
    for (int ln = 0; ln < 5; ln++){

        for (int i = 0; i < 17; i++){
            if (count == satellite_number)
                break;

            ++count;
            int start = i * 3 + 9;
            int acc = atoi(_line.substr(start, 3).c_str());
            satellite_accuracy.push_back(acc);
        }
        std::getline(ifs, _line);
    }

    jump_entry(ifs, 10);
    // end of Head

    return 0;
}


// requied test
int SP3Head::write(std::ofstream& ofs){
    if (!ofs.is_open())
        return -1;

    std::stringstream ss;

    // line 1
    ss << "#c" << mode << date_start.year << " "
       << int_align_end(date_start.month, 2) << " "
      << int_align_end(date_start.day, 2) << " "
     << int_align_end(date_start.hour, 2) << " "
        << int_align_end(date_start.minute, 2)
    << "  0.00000000   "
    << int_align_end(epoch_number, 5) << " ORBIT IGb14 HLM  IGS\n";



    // line 2
    ss << "## " << int_align_end(weeksec.week, 4) << " "
       << float_align_end(weeksec.second, 15, 8) << " "
       << float_align_end(epoch_interval, 14, 8) << " "
       << int_align_end(time_start_mjd, 5) << " "
       << float_align_end(fraction_day, 15, 13) << "\n";



    // line 3 - 7
    ss << "+" << int_align_end(satellite_number, 5) << "   ";


    int count = 0;
    for (int i = 0; i < 17; i++){
        if (count < satellite_number)
            ss << satellite_names[count++];
        else
            ss << "  0";
    }

    ss << "\n";

    for (int i = 0; i < 4; i++){
        ss << "+        ";
        for (int j = 0; j < 17; j++){
            if (count < satellite_number)
                ss << satellite_names[count++];
            else
                ss << "  0";
        }
        ss << "\n";
    }


    // line 8-12
    count = 0;
    for (int i = 0; i < 5; i++){
        ss << "++       ";
        for (int j = 0; j < 17; j++){
            if (count < satellite_number)
                ss << int_align_end(satellite_accuracy[count++], 3);
            else
                ss << "  0";
        }
        ss << "\n";
    }

    // line 13-18
    // don't know what it means....
    ss << "%c cc cc ccc ccc cccc cccc cccc cccc ccccc ccccc ccccc ccccc\n";
    ss << "%c cc cc ccc ccc cccc cccc cccc cccc ccccc ccccc ccccc ccccc\n";
    ss << "%f  0.0000000  0.000000000  0.00000000000  0.000000000000000\n";
    ss << "%f  0.0000000  0.000000000  0.00000000000  0.000000000000000\n";
    ss << "%i    0    0    0    0      0      0      0      0         0\n";
    ss << "%i    0    0    0    0      0      0      0      0         0\n";

    // line 19-22
    ss << "/*\n";
    ss << "/*\n";
    ss << "/*\n";
    ss << "/*\n";


    ofs << ss.str();
    return 0;
}



/*
 * return 0: not eof
 * return -1: eof
 * return -2: error
*/
int SP3Record::read(std::ifstream& ifs){
    if (_line[0] != '*')
        return -2;

    // date
    int y = atoi(_line.substr(3, 4).c_str());
    int m = atoi(_line.substr(8, 2).c_str());
    int d = atoi(_line.substr(11, 2).c_str());
    int h = atoi(_line.substr(14, 2).c_str());
    int min = atoi(_line.substr(17, 2).c_str());
    double sec = atof(_line.substr(20, 11).c_str());
    date = Date(y,m,d,h,min,sec);


    int count = 0;
    while(std::getline(ifs, _line)){
        if (_line[0] == '*')
            return 0;
        else if (trim(_line) == "EOF")
            return -1;

        // to avoid dead loop
        if (count++ >= 2e4)
            break;

        double x = atof(_line.substr(4, 14).c_str());
        double y = atof(_line.substr(18, 14).c_str());
        double z = atof(_line.substr(32, 14).c_str());
        PositionPtr pos(new Position(Pointd(x,y,z)));
        pos->prn = _line.substr(1, 3);
        pos->clock_error = atof(_line.substr(46, 14).c_str());
        pos->date = date;
        poses.push_back(pos);
        modes.push_back(_line[0]);
    }



    return -2;
}



// requied test
int SP3Record::write(std::ofstream& ofs){
    // for one record/ehpemeris
    std::stringstream ss;
    ss << "*  " << date.year << " "
       << int_align_end(date.month, 2) << " "
       << int_align_end(date.day, 2) << " "
       << int_align_end(date.hour, 2) << " "
       << int_align_end(date.minute, 2) << " "
       << float_align_end(date.second, 11, 8) << "\n";

    for (size_t i = 0; i < poses.size(); i++){
        PositionPtr pos = poses[i];
        ss << modes[i] << pos->prn
           << float_align_end(pos->pt._x, 14, 6)
           << float_align_end(pos->pt._y, 14, 6)
           << float_align_end(pos->pt._z, 14, 6)
           << float_align_end(pos->clock_error*1e6, 14, 6) << "\n";
    }


    ofs << ss.str();
    return 0;
}



const PositionPtr SP3Record::getPos(
        const std::string& prn){

    for (size_t i = 0; i < poses.size(); i++){
        if (poses[i]->prn == prn){
            return poses[i];
        }
    }

    return nullptr;

}






GNSSFile::GNSSFile():_path(NULL)
{
}


GNSSFile::GNSSFile(const std::string& path):_path(path){}


GNSSFile::GNSSFile(const char* path):_path(std::string(path)){}



/*
 * return 0: success
 * return -1: not open
 * return -2: unknown file format
 * return -3: not supported version
*/
int BRDCFile::open(const std::string& path){

    this->_path = path;
    std::ifstream ifs(path.c_str());
    if (!ifs.is_open()) {
        return -1;
    }


    std::getline(ifs, _line);
    this->_version = trim(_line.substr(0, 10));
    if (this->_version != "2" && this->_version != "3.04"){
        qDebug() << QString::fromStdString(this->_version) << "\n";
        return -3;
    }



    this->_satellie_system = _line[40] == ' ' ? 'G': _line[40];

    // Head
    while(trim(_line) != "END OF HEADER")
        std::getline(ifs, _line);




    // for each entry
    while(std::getline(ifs, _line)){
        if (this->_version == "2")
        {
            if (this->_satellie_system == 'G')
                read_brdc_gps_like_entry_v2(ifs, this->_map, _line);
            else
                return -2;// unknown ehpemeris
        }
        else if (this->_version == "3.04")
        {
            switch (_line[0])
            {
            case('C'):
                read_brdc_gps_like_entry_v3(ifs, this->_map, _line, false);
                break;
            case('G'):
            case('E'):
            case('I'):
            case('J'):
                read_brdc_gps_like_entry_v3(ifs, this->_map, _line);
                break;
            case('S'):
            case('R'):
                read_brdc_sbas_entry_v3(ifs, this->_map, _line);
                break;
            default:
                assert(false);
            }
        }
        else // unknown version
        {
            return -3;
        }

    }

    return 0;
}



PositionPtr BRDCFile::compute(
        const std::string& prn,
         Date date){

    if (_map.count(prn) == 0)
        return nullptr;


    std::vector<EhpemerisPtr> ptrs = _map[prn];

    size_t index = -1;
    double mindiff = 1e300;

    for (size_t i = 0; i < ptrs.size(); i++){
        EhpemerisPtr eph_ptr = ptrs[i];

        double diff = eph_ptr->toc.diff(date);
        if (abs(diff) < abs(mindiff)){
            mindiff = diff;
            index = i;
        }
    }

    if (abs(mindiff) > 7200)
        return nullptr;

    return ptrs[index]->compute(date);

}



int BRDCFile::writeSP3(
        const std::string& path,
        int minute){
    return writeSP3(path.c_str(), minute);
}


std::vector<std::string> BRDCFile::getPrn(){
    std::vector<std::string> prns;
    for (EhpemerisMap::const_iterator it = _map.begin();
         it != _map.end(); it++){
        prns.push_back(it->first);
    }
    return prns;
}


/*
 * return 0: success
 * return -1: not open
 * return -2: not supported version
*/
int BRDCFile::writeSP3(
        const char* path,
        int minute){


    std::ofstream ofs(path);
    if (!ofs.is_open())
        return -1;

    SP3Head head;
    
    EhpemerisMap::const_iterator it = _map.begin();

    // write head
    //line 1
    head.date_start = it->second[0]->toc;
    head.mode = 'P';
    head.epoch_number = 1440 / minute;

    //line 2
    head.epoch_interval = minute * 60;
    double mjd = Date::date2MJD(head.date_start);
    head.time_start_mjd = int(mjd);
    head.fraction_day = mjd - int(mjd);
    head.weeksec = Date::date2GPST(head.date_start);

    //line 3
    head.satellite_number = _map.size();
    for (it = _map.begin(); it != _map.end(); it++){
        head.satellite_names.push_back(it->first);
        head.satellite_accuracy.push_back(0);
    }

    //head.write(ofs);


    int y = head.date_start.year;
    int m = head.date_start.month;
    int d = head.date_start.day;
    int hour = head.date_start.hour;
    int min = 0;


    Date date(head.date_start);
    // write record
    for (int i = 0; i < head.epoch_number; i++){
        Date date(y,m,d,hour,min+i*minute);
        SP3Record record;
        record.date = date;

        for (size_t j = 0; j < head.satellite_names.size(); j++){
            PositionPtr pos = compute(head.satellite_names[j], date);
            if (pos){
                record.poses.push_back(pos);
                record.modes.push_back('P');

            }

        }

        record.write(ofs);
    }


    ofs.close();
    return 0;
}


Date BRDCFile::getDateStart(){
    return  _map.begin()->second[0]->toc;
}



/*
 * return 0: success
 * return -1: not open
 * return -2: unknown file format/error
*/
int SP3File::open(const std::string& path){

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        opened = false;
        return -1;
    }

    // unknown type
    if (head.read(ifs)!=0){
        opened = false;
        return -2;
    }


    while (true){
        SP3Record record;
        int return_val = record.read(ifs);
        if (return_val == 0)
            records.push_back(record);
        else if(return_val == -1){
            records.push_back(record);
            break;
        }
        else
            return -2; // error
    }

    opened = true;
    return 0;
}



PositionPtr SP3File::compute(
        const std::string& prn,
        Date date){

    const int k = 9;

    // not enough ephemeris
    if (head.epoch_number < k)
        return nullptr;

    // find nearest 9 ephemeris
    std::vector<PositionPtr> nearest;//(9, Date(0,0,0,0));
    for (size_t i = 0; i < records.size(); i++){
        PositionPtr pos = records[i].getPos(prn);
        if (pos == nullptr || pos->clock_error >= 9999)
            continue;

        nearest.push_back(pos);

    }


    // maybe is not in order?
    // std::sort(...)

    /* sliding window algorithm
     * to find nearest 9 ephemeris*/
    while (nearest.size() > k){
        if (date - nearest[0]->date < nearest[nearest.size() - 1]->date - date){
            nearest.pop_back();
        }
        else{
            nearest.erase(nearest.begin());
        }
    }


    if (nearest.size() < k)
        return nullptr;

    std::vector<double> times;
    std::vector<double> x, y, z, clock_error;
    for (size_t i = 0; i < nearest.size(); i++){
        PositionPtr ptr = nearest[i];

        x.push_back(ptr->pt._x);
        y.push_back(ptr->pt._y);
        z.push_back(ptr->pt._z);
        clock_error.push_back(ptr->clock_error);

        // use difference to avoid overflow
        times.push_back(ptr->date.diff(date));
    }

    double sec = 0.0;
    double x_ = Interpolater::Lagrange(times, x, sec);
    double y_ = Interpolater::Lagrange(times, y, sec);
    double z_ = Interpolater::Lagrange(times, z, sec);
    double clc = Interpolater::Lagrange(times, clock_error, sec)*1e-6;

    PositionPtr ptr(new Position(Pointd(x_,y_,z_),clc,date));
    ptr->prn = prn;
    return ptr;
}



int SP3File::writeSP3(
        const std::string& path,
        int minute){
    return writeSP3(path.c_str(), minute);

}



/*
 * return 0: success
 * return -1: not open
*/
int SP3File::writeSP3(
        const char* path,
        int minute){

    std::ofstream ofs(path);
    if (!ofs.is_open())
        return -1;

    // write head
    SP3Head hd = head;
    hd.epoch_interval = minute;
    hd.epoch_number = int(1440 / minute);

    head.write(ofs);

    int y = head.date_start.year;
    int m = head.date_start.month;
    int d = head.date_start.day;
    int hour = head.date_start.hour;
    int min = 0;
    // write records
    std::vector<std::string> names = hd.satellite_names;

    for (int i = 0; i < hd.epoch_number; i++){
        Date date(y,m,d,hour,min+i*minute);
        SP3Record record;
        record.date = date;
        for (size_t j = 0; j < names.size(); j++){
            PositionPtr pos = compute(names[j], date);
            if (pos){
                record.poses.push_back(pos);
                record.modes.push_back('P');
            }
        }
        record.write(ofs);
    }


    ofs << "EOF\n";

    ofs.close();
    return 0;

}


std::vector<std::string> SP3File::getPrn(){
    return head.satellite_names;
}

}
