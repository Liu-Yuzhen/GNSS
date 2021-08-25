#include "dpwidget.h"
#include "ui_dpwidget.h"
#include "filemanager.h"
#include <QFileDialog>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <map>
#include "massfuncs.h"



DPWidget::DPWidget(DockWidget* dock,QWidget *parent) :
    QWidget(parent),_dock(dock), obsr(nullptr),obsu(nullptr),
    ui(new Ui::DPWidget)
{
    ui->setupUi(this);

    ui->methodBox->addItem("SD");
    ui->methodBox->addItem("DD");
    // delete when closed
    this->setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->buttonOpen1, SIGNAL(clicked(bool)),
            this, SLOT(openFile1()));

    connect(ui->buttonOpen2, SIGNAL(clicked(bool)),
            this, SLOT(openFile2()));

    connect(ui->buttonClear, SIGNAL(clicked(bool)),
            this, SLOT(clear()));

    connect(ui->buttonCompute, SIGNAL(clicked(bool)),
            this, SLOT(computeClick()));
}


void fix(const Eigen::Matrix<double, Dynamic, Dynamic>& Q,
         const Eigen::Matrix<double, Dynamic, Dynamic>& a,
         Eigen::Matrix<double, Dynamic, Dynamic>& a_hat,
         double sigma = 0.05)
{
    int n = Q.rows();
    a_hat.resize(n, 1);
    double min_val = 1e10;
    // initialize
    Eigen::Matrix<double, Dynamic, Dynamic> aint;
    aint.resize(n, 1);
    for (int i = 1; i < n; i++)
    {
        aint(i, 0) = round(a(i, 0));
    }

    for (int i = 0; i < n; i++)
    {
        double interval = 3 * sigma * sqrt(Q(i, i));
        int start = static_cast<int>(a(i, 0) - interval);
        int end = static_cast<int>(a(i, 0) + interval);
        for (int k = start; k < end; k++)
        {
            aint(i, 0) = k;
            Eigen::Matrix<double, Dynamic, Dynamic> diff = aint - a;
            Eigen::Matrix<double, Dynamic, Dynamic> res = diff.transpose() * Q * diff;
            double val = res(0, 0);
            if (val < min_val)
            {
                min_val = val;
                a_hat = aint.replicate(1, 1);
            }
        }

    }

    std::cout << "a_hat:\n" << a_hat << std::endl;
}


void DPWidget::openFile1(){
    QString path = QFileDialog::getOpenFileName(
                this, tr("Choose reference station"),QString(), "*.20o");

    if (path == "")
        return;

    if (obsr != nullptr)
            delete obsr;

    obsr = new lyz::ObsFile(path.toStdString());


    if (obsr->isopen())
        ui->textBrowser->setText(path);
    else
    {
        QMessageBox::information(
                    this, "Error",
                    "Failed to open file: " + path,
                    QMessageBox::Yes);
        return;
    }

    // update code
    if (obsu != nullptr){
        // date not match
        if (obsr->getDate() != obsu->getDate())
        {
            ui->textBrowser->clear();
            delete obsr;
            obsr = nullptr;
            QMessageBox::information(
                        this, "Error",
                        "Date does NOT match!" ,
                        QMessageBox::Yes);
            return;
        }

        ui->codeBox->clear();
        std::vector<std::string> types = obsu->getObsType();
        for (size_t i = 0; i < types.size(); i++){
            std::string type = types[i];
            if (obsr->codeIndex(type) >= 0)
                ui->codeBox->addItem(QString::fromStdString(type));
        }
    }
}


void DPWidget::openFile2(){
    QString path = QFileDialog::getOpenFileName(
                this, tr("Choose unknown station"),QString(), "*.20o");

    if (path == "")
        return;

    if (obsu != nullptr)
        delete obsu;

    obsu = new lyz::ObsFile(path.toStdString());


    if (obsu->isopen())
        ui->textBrowser_2->setText(path);
    else
    {
        QMessageBox::information(
                    this, "Error",
                    "Failed to open file: " + path,
                    QMessageBox::Yes);
        return;
    }

    // update code
    if (obsr != nullptr){

        // date not match
        if (obsr->getDate() != obsu->getDate())
        {
            ui->textBrowser_2->clear();
            delete obsu;
            obsu = nullptr;
            QMessageBox::information(
                        this, "Error",
                        "Date does NOT match!" ,
                        QMessageBox::Yes);
            return;
        }


        // update code box
        ui->codeBox->clear();
        std::vector<std::string> types = obsu->getObsType();
        for (size_t i = 0; i < types.size(); i++){
            std::string type = types[i];
            if (obsr->codeIndex(type) >= 0)
                ui->codeBox->addItem(QString::fromStdString(type));
        }
    }
}


int getIndex(const std::vector<std::string>& strings,
          const std::string& str){
    for (int i = 0; i < strings.size(); i++){
        if (str == strings[i])
            return i;
    }
    return -1;
}


void DPWidget::computeClick(){
    if (!obsr){
        QMessageBox::information(
                    this, "Error",
                    "Reference station is not selected!",
                    QMessageBox::Yes);
        return;
    }

    if (!obsu){
        QMessageBox::information(
                    this, "Error",
                    "Unknown station is not selected!",
                    QMessageBox::Yes);
        return;
    }

    int fileindex = _dock->selectedIndex();
    if (fileindex < 0){
        QMessageBox::information(
                    this, "Error",
                    "No navigation file selected!",
                    QMessageBox::Yes);

        return;
    }

    std::string mode = ui->codeBox->currentText().toStdString();
    if (mode[0] == 'S' || mode[0] == 'D')
    {
        QMessageBox::information(
                    this, "Error",
                    "Only 'L', 'P', 'C' signals are implemented",
                    QMessageBox::Yes);
        return;
    }

    // get selected date
    QDate qdate =  ui->dateTimeEdit->date();
    int day = qdate.day();
    int month = qdate.month();
    int y = qdate.year();
    QTime time = ui->dateTimeEdit->time();
    int hour = time.hour();
    int min = time.minute();
    int sec = time.second();
    lyz::Date date(y, month, day, hour, min, sec);

    // get the closet date
    date = obsr->getClosetDate(date);

    QDate dt(date.year, date.month, date.day);
    QTime t(date.hour, date.minute, date.second);
    ui->dateTimeEdit->setDate(dt);
    ui->dateTimeEdit->setTime(t);


    // get Nt epoch
    int Nt = 10;
    std::vector<std::map<std::string, double>> _ref_prn_value;
    std::vector<std::map<std::string, double>> _unk_prn_value;
    for (int i = 0; i < Nt; i++)
    {
        lyz::Date dt(date.year, date.month, date.day,
                  date.hour, date.minute, date.second + 30 * i);
        std::vector<double> values;
        std::vector<std::string> prns;

        obsr->getValues(dt, mode, values, prns);
        std::map<std::string, double> m;
        for (int j = 0; j < prns.size(); j++){
            m[prns[j]] = values[j];
        }
        _ref_prn_value.push_back(m);


        m.clear();
        values.clear();
        prns.clear();
        obsu->getValues(dt, mode, values, prns);
        for (int j = 0; j < prns.size(); j++){
            m[prns[j]] = values[j];
        }
        _unk_prn_value.push_back(m);
    }

    // select satellites that appear in all epochs
    std::map<std::string, int> counter;
    for (int i = 0; i < Nt; i++)
    {
        for (auto it = _ref_prn_value[i].begin();
             it != _ref_prn_value[i].end(); it++)
        {
            if (counter.count(it->first)==0)
                counter[it->first] = 1;
            else
                ++counter[it->first];
        }


        for (auto it = _unk_prn_value[i].begin();
             it != _unk_prn_value[i].end(); it++)
        {
            if (counter.count(it->first)==0)
                counter[it->first] = 1;
            else
                ++counter[it->first];
        }


    }

    std::vector<std::vector<double>> ref_values;
    std::vector<std::vector<double>> unk_values;
    std::vector<std::string> prns;

    for (auto it = _ref_prn_value[0].begin();
         it != _ref_prn_value[0].end(); it++)
    {
        std::string prn_ = it->first;
        if (prn_[0] == 'R' || prn_[0] == 'S')
            continue;

        // it means the satellite appear in all epochs
        if (counter[prn_] == 2 * Nt){
            prns.push_back(prn_);
        }


    }

    // not enough satellites
    if ((mode[0] == 'L' && prns.size() < (Nt + 2) * 1./ (Nt - 1)) ||
           (mode[0] != 'L' && prns.size() < 3. / Nt + 1))
    {
        QMessageBox::information(
                    this, "Error",
                    "Not enough satellite!",
                    QMessageBox::Yes);
        return;
    }


    // get values
    for (int i = 0; i < Nt; i++)
    {
        std::vector<double> valsr;
        std::vector<double> valsu;
        for (int j = 0; j < prns.size(); j++){
            valsr.push_back(_ref_prn_value[i][prns[j]]);
            valsu.push_back(_unk_prn_value[i][prns[j]]);
        }

        ref_values.push_back(valsr);
        unk_values.push_back(valsu);
    }


    // get and correct satellite's xyzt
    std::vector<std::vector<Vector4d>> xyzts;
    for (int i = 0; i < Nt; i++)
    {
        lyz::Date dt(date.year, date.month, date.day,
                  date.hour, date.minute, date.second + 30 * i);

        std::vector<double> ranges;
        std::vector<std::string> prns_;
        obsr->getSeudoRanges(dt, ranges, prns_);
        // time of emission
        std::vector<Vector4d> xyzt;

        for (size_t j = 0; j < prns.size(); j++)
        {
            std::string pn = prns[j];
            int index = getIndex(prns_, pn);

            if (index >= 0)
            {
                lyz::PositionPtr pos;
                double p = ranges[index];
                lyz::Date ts_date = dt;
                double deltt = 0.0;

                // for 10 iterations at maximun
                for (size_t k = 0; k < 10; k++){
                    ts_date.second = dt.second - p / c - deltt;
                    pos = FileManager::instance()
                            ->compute(fileindex, pn, ts_date);
                    if (pos == nullptr)
                    {
                        QMessageBox::information(
                                    this, "Error",
                                    "Navigation file has not enough ephemeris!",
                                    QMessageBox::Yes);
                        return;
                    }


                    if (abs(deltt - pos->clock_error) < 1e-10)
                    {
                        break;
                    }
                    deltt = pos->clock_error;
                }

                // earth rotation correction
                lyz::Matd rot = lyz::Matd::creat(p / c * w_earth,
                                                 lyz::Axis::zAxis);
                lyz::Pointd pt = 1000 * (rot * pos->pt);

                xyzt.push_back(Vector4d(pt._x, pt._y,
                                        pt._z, pos->clock_error));

            }
            // error
            else
            {
                qDebug() << "index < 0!!!";
            }


        }
        xyzts.push_back(xyzt);


    }



    // spp for reference station
    int index = _dock->selectedIndex();
    if (index < 0){
        QMessageBox::information(
                    this, "Error",
                    "No navigation file selected!",
                    QMessageBox::Yes);

        return;
    }

    Matrix3d Q;
    Vector4d* refxyz = spp(index, obsr, date, Q);
    (*refxyz)[0] =  ui->boxX->toPlainText().toDouble();
    (*refxyz)[1] =  ui->boxY->toPlainText().toDouble();
    (*refxyz)[2] =  ui->boxZ->toPlainText().toDouble();
    // failed to calulate
    if (refxyz == nullptr)
    {
        QMessageBox::information(
                    this, "Error",
                    "Failed to calculate the reference station\nPlease check the file validity",
                    QMessageBox::Yes);

        return;
    }

    QString method = ui->methodBox->currentText();
    Vector3d* xyz;
    if (mode[0] == 'L'){
        double wave = 0;
        if (mode[1] == '1')
            wave = 0.19029367279836487;// 1575.42e6
        else if (mode[1] == '2')
            wave = 0.24421021342456825;//1227.6e6
        else if (mode[1] == '5')
            wave = 0.25482804879085386;//1176.45e6

        if (method == "SD")
            xyz = phaseSD(ref_values, unk_values, xyzts, refxyz, wave);
        else
            xyz = phaseDD(ref_values, unk_values, xyzts, refxyz, wave);
    }
    else if (mode[0] == 'P' || mode[0] == 'C'){
        if (method == "SD")
            xyz = pseudoSD(ref_values, unk_values, xyzts, refxyz);
        else
            xyz = pseudoDD(ref_values, unk_values, xyzts, refxyz);
    }

    //failed to calculate
    if (xyz == nullptr){
        QMessageBox::information(
                    this, "Error",
                    "Failed to calculate unknown station.\nPlease check the file validity",
                    QMessageBox::Yes);
        return;
    }




    QString str = "[";
    str += QString::number(date.year) + "/" +  QString::number(date.month) + "/" +  QString::number(date.day) + " ";
    str +=  QString::number(date.hour) + ":";

    if (date.minute < 10)
        str += "0";
    str += QString::number(date.minute) + ":";

    if (date.second < 10)
        str += "0";
    str += QString::number(date.second) + "]\n";
//    str += "Reference station: \n";
//    str += "x = " + QString::number((*refxyz)[0], 'f', 3) + " m\n";
//    str += "y = " + QString::number((*refxyz)[1], 'f', 3) + " m\n";
//    str += "z = " + QString::number((*refxyz)[2], 'f', 3) + " m\n";
//    str += "clock error = " + QString::number((*refxyz)[3]*1e6, 'f', 3) + " μs\n";
//    str += "qxx = " + QString::number(sqrt(Q(0,0)), 'f', 3) + ", qyy = "
//            + QString::number(sqrt(Q(1,1)), 'f', 3)
//            + ", qzz = " + QString::number(sqrt(Q(2,2)), 'f', 3) + "\n";
//    str += "PDOP = " + QString::number(sqrt(Q(0,0) + Q(1,1) + Q(2,2)), 'f', 3) + "\n";
    str += "Unknown station: \n";
    str += "x = " + QString::number((*xyz)[0], 'f', 3) + " m\n";
    str += "y = " + QString::number((*xyz)[1], 'f', 3) + " m\n";
    str += "z = " + QString::number((*xyz)[2], 'f', 3) + " m\n\n";

    ui->textBrowser_3->setPlainText(ui->textBrowser_3->toPlainText() + str);
}


Vector3d* DPWidget::pseudoSD(
        const std::vector<std::vector<double>>& ref_ranges,
        const std::vector<std::vector<double>>& unk_ranges,
        const std::vector<std::vector<Vector4d>>& xyzts,
        Vector4d* xyztref)
{
    int nt = ref_ranges.size(); // epoch number
    int nj = ref_ranges[0].size(); // satellite number

    // get pseudorange single difference
    std::vector<std::vector<double>> sd;
    std::vector<std::vector<double>> ref_sat_dis;

    for (int t = 0; t < nt; t++)
    {
        std::vector<double> range_dif;
        std::vector<double> dis;

        for (int s = 0; s < nj; s++)
        {
            range_dif.push_back(unk_ranges[t][s]-
                                ref_ranges[t][s]);

            double dx = (*xyztref)[0] - xyzts[t][s][0];
            double dy = (*xyztref)[1] - xyzts[t][s][1];
            double dz = (*xyztref)[2] - xyzts[t][s][2];

            dis.push_back(sqrt(dx*dx + dy*dy + dz*dz));
        }
        sd.push_back(range_dif);
        ref_sat_dis.push_back(dis);

    }

    Matrix<double, Dynamic, Dynamic> B;
    Matrix<double, Dynamic, Dynamic> L;
    Matrix<double, Dynamic, Dynamic> v;
    Matrix<double, Dynamic, Dynamic> x;
    B.setZero(nj * nt, 3 + nt);
    L.resize(nj * nt, 1);
    v.resize(3 + nt, 1);
    x.setZero(3 + nt, 1);

    int max_iteration = 20;
    // for each iteration
    for (int it = 0; it < max_iteration; it++)
    {
        //for each epoch
        for (int t = 0; t < nt; t++)
        {
            // for each satellite
            for (int s = 0; s < nj; s++)
            {
                double dx = x(0, 0) - xyzts[t][s][0];
                double dy = x(1, 0) - xyzts[t][s][1];
                double dz = x(2, 0) - xyzts[t][s][2];

                double r = sqrt(dx*dx + dy*dy + dz*dz);

                int row = t * nj + s;
                B(row, 0) = dx / r;
                B(row, 1) = dy / r;
                B(row, 2) = dz / r;
                B(row, 3 + t) = 1;

                L(row, 0) = sd[t][s] + ref_sat_dis[t][s] - r - x(3 + t, 0);

            }

        }

        Matrix<double, Dynamic, Dynamic> BT = B.transpose();
        v = (BT * B).inverse() * (BT * L);
        x += v;
        if (v.norm() < 1e-4)
            break;
    }


    qDebug() << "[range SD]";
    qDebug() << "unk: ";
    for (int i = 0; i < L.rows(); i++){
        qDebug() << L(i, 0) << ", ";
    }
    qDebug() << "\n";

    return new Vector3d(x(0, 0), x(1, 0), x(2, 0));
}


Vector3d* DPWidget::pseudoDD(
        const std::vector<std::vector<double>>& ref_ranges,
        const std::vector<std::vector<double>>& unk_ranges,
        const std::vector<std::vector<Vector4d>>& xyzts,
        Vector4d* xyztref)
{
    int nt = ref_ranges.size(); // epoch number
    int nj = ref_ranges[0].size(); // satellite number

    // get pseudorange single difference
    std::vector<std::vector<double>> dd;
    std::vector<std::vector<double>> ref_sat_dis;

    for (int t = 0; t < nt; t++)
    {
        std::vector<double> range_dif;
        std::vector<double> dis;

        double dx0 = (*xyztref)[0] - xyzts[t][0][0];
        double dy0 = (*xyztref)[1] - xyzts[t][0][1];
        double dz0 = (*xyztref)[2] - xyzts[t][0][2];
        double r0 = sqrt(dx0*dx0 + dy0*dy0 + dz0*dz0);

        for (int s = 1; s < nj; s++)
        {
            double dx = (*xyztref)[0] - xyzts[t][s][0];
            double dy = (*xyztref)[1] - xyzts[t][s][1];
            double dz = (*xyztref)[2] - xyzts[t][s][2];
            double r = sqrt(dx*dx + dy*dy + dz*dz);


            range_dif.push_back(
                        unk_ranges[t][s]-
                        ref_ranges[t][s]-
                        unk_ranges[t][0]+
                        ref_ranges[t][0]);

            dis.push_back(r - r0);
        }
        dd.push_back(range_dif);
        ref_sat_dis.push_back(dis);

    }

    Matrix<double, Dynamic, Dynamic> B;
    Matrix<double, Dynamic, Dynamic> L;
    Matrix<double, Dynamic, Dynamic> P;
    Matrix<double, Dynamic, Dynamic> v;
    Matrix<double, Dynamic, Dynamic> x;
    B.setZero((nj - 1) * nt, 3);
    L.resize((nj - 1) * nt, 1);
    P.setZero((nj - 1) * nt, (nj - 1) * nt);
    v.resize(3, 1);
    x.setZero(3, 1);


    // weight matrix
    for (int t = 0; t < nt; t++)
    {
        for (int r = t * (nj-1); r < (t + 1) * (nj -1); r++)
        {
            for (int c = t * (nj-1); c < (t + 1) * (nj -1); c++)
            {
                if (r != c)
                    P(r, c) = -1;
                else
                    P(r, c) = nj - 1;
            }
        }

    }


    int max_iteration = 20;
    for (int it = 0; it <max_iteration; it++)
    {
        // for each epoch
        for (int t = 0; t < nt; t++)
        {

            // distance between unknown and the satellite 0
            double dx = x(0, 0) - xyzts[t][0][0];
            double dy = x(1, 0) - xyzts[t][0][1];
            double dz = x(2, 0) - xyzts[t][0][2];
            double r0 = sqrt(dx*dx + dy*dy + dz*dz);

            // for each satellite
            for (int s = 1; s < nj; s++)
            {
                // distance between unknown and the satellite s
                double dxs = x(0, 0) - xyzts[t][s][0];
                double dys = x(1, 0) - xyzts[t][s][1];
                double dzs = x(2, 0) - xyzts[t][s][2];
                double rs = sqrt(dxs*dxs + dys*dys + dzs*dzs);

                int row = t * (nj - 1) + s - 1;

                L(row, 0) = dd[t][s-1] + ref_sat_dis[t][s-1] -
                        (rs - r0);


                B(row, 0) = dxs / rs - dx / r0;
                B(row, 1) = dys / rs - dy / r0;
                B(row, 2) = dzs / rs - dz / r0;

            }
        }

        Matrix<double, Dynamic, Dynamic> BT = B.transpose();
        v = (BT * P * B).inverse() * (BT * P * L);
        x += v;

        if (v.norm() < 1e-4)
            break;
    }

    qDebug() << "[range DD]";
    qDebug() << "unk: ";
    for (int i = 0; i < L.rows(); i++){
        qDebug() << L(i, 0) << ", ";
    }
    qDebug() << "\n";

    return new Vector3d(x(0, 0), x(1, 0), x(2, 0));
}


Vector3d* DPWidget::phaseSD(
        const std::vector<std::vector<double>>& ref_values,
        const std::vector<std::vector<double>>& unk_values,
        const std::vector<std::vector<Vector4d>>& xyzts,
        Vector4d* xyztref, double wavelen)
{
    size_t nt = ref_values.size();
    size_t nj = ref_values[0].size();

    Matrix<double, Dynamic, Dynamic> B;
    Matrix<double, Dynamic, Dynamic> L;
    Matrix<double, Dynamic, Dynamic> x;
    Matrix<double, Dynamic, Dynamic> v;
    B.setZero(nj * nt, nj + nt + 2);
    L.resize(nj * nt, 1);
    x.setZero(nj + nt + 2, 1);
    v.resize(nj + nt + 2, 1);

    // station difference
    std::vector<std::vector<double>> sd;
    for (int t = 0; t < nt; t++)
    {
        std::vector<double> vals;
        for (int s = 0; s < nj; s++)
        {
            vals.push_back((unk_values[t][s] - ref_values[t][s])*wavelen);
        }
        sd.push_back(vals);
    }


    // distance from reference to satellites
    std::vector<std::vector<double>> distance_ref_sat;
    for (int t = 0; t < nt; t++)
    {
        std::vector<double> dis;
        for (int s = 0; s < nj; s++)
        {
            double dx = (*xyztref)[0] - xyzts[t][s][0];
            double dy = (*xyztref)[1] - xyzts[t][s][1];
            double dz = (*xyztref)[2] - xyzts[t][s][2];
            dis.push_back(sqrt(dx*dx + dy*dy + dz*dz));
        }
        distance_ref_sat.push_back(dis);
    }



    const int max_iteration = 20;
    // first try to fix cycle ambiguity
    for (int it = 0; it < max_iteration; it++)
    {
        // for every epoch
        for (int t = 0; t < nt; t++)
        {
            // for every satellite
            double dx = x(0,0) - xyzts[t][0][0];
            double dy = x(1,0) - xyzts[t][0][1];
            double dz = x(2,0) - xyzts[t][0][2];
            double r = sqrt(dx*dx+dy*dy+dz*dz);
            L(t * nj, 0) = sd[t][0] + distance_ref_sat[t][0] - x(3+t, 0) - r;
            B(t * nj, 0) = dx / r;//dx
            B(t * nj, 1) = dy / r;//dy
            B(t * nj, 2) = dz / r;//dz

            B(t * nj, 3 + t) = 1;

            for (int s = 1; s < nj; s++)
            {
                int row = t * nj + s;
                double dx = x(0,0) - xyzts[t][s][0];
                double dy = x(1,0) - xyzts[t][s][1];
                double dz = x(2,0) - xyzts[t][s][2];
                double r = sqrt(dx*dx+dy*dy+dz*dz);

                L(row, 0) = sd[t][s] + distance_ref_sat[t][s] - r
                        - x(3+t, 0) - wavelen * x(3+nt+s-1,0);

                B(row, 0) = dx / r;
                B(row, 1) = dy / r;
                B(row, 2) = dz / r;

                B(row, 3 + t) = 1;
                B(row, 3 + nt + s - 1) = wavelen;

            }
        }

        Matrix<double, Dynamic, Dynamic> BT = B.transpose();
        v = (BT * B).inverse() * (BT * L);
        x += v;
        if (v.norm() < 1e-4)
            break;

    }



//    // fix ambiguity and solve again
//    for (int i = 3 + nt; i < 2 + nt + nj; i++)
//    {
//        x(i, 0) = round(x(i, 0));
//    }
//    Matrix<double, Dynamic, Dynamic> _x;
//    _x.resize(3 + nt, 1);
//    for (int i = 0; i < 3 + nt; i++)
//    {
//        _x(i,0) = x(i,0);
//    }



//    B.setZero(nj * nt, 3 + nt);
//    v.resize(nt + 3, 1);

//    for (int it = 0; it < max_iteration; it++)
//    {
//        // for every epoch
//        for (int t = 0; t < nt; t++)
//        {
//            // for every satellite
//            double dx = x(0,0) - xyzts[t][0][0];
//            double dy = x(1,0) - xyzts[t][0][1];
//            double dz = x(2,0) - xyzts[t][0][2];
//            double r = sqrt(dx*dx+dy*dy+dz*dz);
//            L(t * nj, 0) = sd[t][0] + distance_ref_sat[t][0] - x(3+t, 0) - r;
//            B(t * nj, 0) = dx / r;//dx
//            B(t * nj, 1) = dy / r;//dy
//            B(t * nj, 2) = dz / r;//dz

//            B(t * nj, 3 + t) = 1;

//            for (int s = 1; s < nj; s++)
//            {
//                int row = t * nj + s;
//                double dx = _x(0,0) - xyzts[t][s][0];
//                double dy = _x(1,0) - xyzts[t][s][1];
//                double dz = _x(2,0) - xyzts[t][s][2];
//                double r = sqrt(dx*dx+dy*dy+dz*dz);

//                L(row, 0) = sd[t][s] + distance_ref_sat[t][s] - r
//                        - _x(3+t, 0) - wavelen * x(3+nt+s-1,0);

//                B(row, 0) = dx / r;
//                B(row, 1) = dy / r;
//                B(row, 2) = dz / r;

//                B(row, 3 + t) = 1;
//            }


//        }

//        Matrix<double, Dynamic, Dynamic> BT = B.transpose();
//        v = (BT * B).inverse() * (BT * L);
//        _x += v;
//        if (v.norm() < 1e-4)
//            break;
//    }

    qDebug() << "[phase SD]";
    qDebug() << "unk: ";
    for (size_t i = 0; i < nj * nt; i++){
        qDebug() << L(i, 0) << ", ";
    }
    qDebug() << "\n";

    return new Vector3d(x(0,0), x(1,0), x(2,0));
}


Vector3d* DPWidget::phaseDD(
        const std::vector<std::vector<double>>& ref_values,
        const std::vector<std::vector<double>>& unk_values,
        const std::vector<std::vector<Vector4d>>& xyzts,
        Vector4d* xyztref, double wavelen)
{
    int nt = ref_values.size();
    int nj = ref_values[0].size();

    Matrix<double, Dynamic, Dynamic> B;
    Matrix<double, Dynamic, Dynamic> P;
    Matrix<double, Dynamic, Dynamic> L;
    Matrix<double, Dynamic, Dynamic> x;
    Matrix<double, Dynamic, Dynamic> v;
    B.setZero((nj-1) * nt, nj + 2);
    P.setZero((nj-1) * nt, (nj-1) * nt);
    L.resize((nj-1) * nt, 1);
    x.setZero(nj + 2, 1);
    v.resize(nj + 2, 1);

    // double difference of phase
    std::vector<std::vector<double>> DD;//phase double difference
    std::vector<std::vector<double>> ref_sat_dif;
    for (int t = 0; t < nt; t++)
    {
        // distance between reference station and satellite 1
        double x = (*xyztref)[0] - xyzts[t][0][0];
        double y = (*xyztref)[1] - xyzts[t][0][1];
        double z = (*xyztref)[2] - xyzts[t][0][2];
        double r0 = sqrt(x*x + y*y + z*z);


        std::vector<double> d;
        std::vector<double> refsatd;
        for (int s = 1; s < nj; s++)
        {
            d.push_back((unk_values[t][s] - ref_values[t][s]
                        - unk_values[t][0] + ref_values[t][0])*wavelen);

            double dx = (*xyztref)[0] - xyzts[t][s][0];
            double dy = (*xyztref)[1] - xyzts[t][s][1];
            double dz = (*xyztref)[2] - xyzts[t][s][2];
            double r = sqrt(dx*dx + dy*dy + dz*dz);

            refsatd.push_back(r - r0);
        }
        DD.push_back(d);
        ref_sat_dif.push_back(refsatd);
    }

    // weight matrix
    for (int t = 0; t < nt; t++)
    {
        for (int r = t * (nj-1); r < (t + 1) * (nj -1); r++)
        {
            for (int c = t * (nj-1); c < (t + 1) * (nj -1); c++)
            {
                if (r != c)
                    P(r, c) = -1;
                else
                    P(r, c) = nj - 1;
            }
        }

    }


    const int max_iteration = 20;
    for (int it = 0; it < max_iteration; it++)
    {
        for (int t = 0; t < nt; t++)
        {
            // distance between unknown and the satellite 0
            double dx = x(0, 0) - xyzts[t][0][0];
            double dy = x(1, 0) - xyzts[t][0][1];
            double dz = x(2, 0) - xyzts[t][0][2];
            double r0 = sqrt(dx*dx + dy*dy + dz*dz);


            for (int s = 1; s < nj; s++)
            {
                int row = t * (nj - 1) + s - 1;
                // distance between unknown and the satellite s
                double dxs = x(0, 0) - xyzts[t][s][0];
                double dys = x(1, 0) - xyzts[t][s][1];
                double dzs = x(2, 0) - xyzts[t][s][2];
                double rs = sqrt(dxs*dxs + dys*dys + dzs*dzs);

                L(row, 0) = DD[t][s-1] + ref_sat_dif[t][s-1] -
                        (rs - r0) - wavelen * x(s + 2, 0);


                B(row, 0) = dxs / rs - dx / r0;
                B(row, 1) = dys / rs - dy / r0;
                B(row, 2) = dzs / rs - dz / r0;

                B(row, 2 + s) = wavelen;
            }

        }


        Matrix<double, Dynamic, Dynamic> BT = B.transpose();
        v = (BT * P * B).inverse() * (BT * P * L);
        x += v;

        if (v.norm() < 1e-4)
            break;

    }


    // fix ambiguity
    Matrix<double, Dynamic, Dynamic> Q, Q_for_a;
    Q = (B.transpose() * P * B).inverse();
    Q_for_a.resize(Q.rows() - 3, Q.cols() - 3);
    for (int i = 0; i < Q_for_a.rows(); i++)
    {
        for (int j = 0; j < Q_for_a.rows(); j++)
        {
            Q_for_a(i, j) = Q(i + 3, j + 3);
        }
    }
    Matrix<double, Dynamic, Dynamic> ambiguity;
    ambiguity.resize(nj - 1, 1);
    for (int i = 0; i < nj - 1; i++)
    {
        ambiguity(i, 0) = x(i + 3, 0);
    }
    Matrix<double, Dynamic, Dynamic> ambiguity_fixed;
    fix(Q_for_a, ambiguity, ambiguity_fixed);

    for (int n = 3; n < nj + 2; n++)
    {
        x(n, 0) = ambiguity_fixed(n - 3, 0);
    }


    B.resize((nj-1) * nt, 3);
    v.resize(3, 1);
    Matrix<double, 3, 1> _x;
    for (int i = 0; i < 3; i++){
        _x(i, 0) = x(i, 0);
    }

    for (int it = 0; it < max_iteration; it++)
    {
        for (int t = 0; t < nt; t++)
        {
            double dx = _x(0, 0) - xyzts[t][0][0];
            double dy = _x(1, 0) - xyzts[t][0][1];
            double dz = _x(2, 0) - xyzts[t][0][2];
            double r0 = sqrt(dx*dx + dy*dy + dz*dz);

            for (int s = 1; s < nj; s++)
            {
                int row = t * (nj - 1) + s - 1;
                double dxs = _x(0, 0) - xyzts[t][s][0];
                double dys = _x(1, 0) - xyzts[t][s][1];
                double dzs = _x(2, 0) - xyzts[t][s][2];
                double rs = sqrt(dxs*dxs + dys*dys + dzs*dzs);

                L(row, 0) = DD[t][s-1] + ref_sat_dif[t][s-1] -
                        (rs - r0) - wavelen * x(s + 2, 0);

                B(row, 0) = dxs / rs - dx / r0;
                B(row, 1) = dys / rs - dy / r0;
                B(row, 2) = dzs / rs - dz / r0;
            }
        }

        Matrix<double, Dynamic, Dynamic> BT = B.transpose();
        v = (BT * P * B).inverse() * (BT * P * L);
        _x += v;

        if (v.norm() < 1e-4)
            break;

    }

    qDebug() << "[phase DD]";
    qDebug() << "unk: ";
    for (int i = 0; i < L.rows(); i++){
        qDebug() << L(i, 0) << ", ";
    }
    qDebug() << "\n";

    return new Vector3d(_x(0,0), _x(1,0), _x(2,0));
}


void DPWidget::clear(){
    ui->textBrowser_3->clear();
}



DPWidget::~DPWidget()
{
    if (obsr != nullptr)
        delete obsr;

    if (obsu != nullptr)
        delete obsu;

    delete ui;
}
