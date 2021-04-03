#include "filemanager.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>

void FileManager::notify(){
    for (QVector<Observer*>::const_iterator it = _observers.begin();
         it != _observers.end(); it++){
        (*it)->execute();
    }
}


void FileManager::addObserver(Observer* observer){
    _observers.push_back(observer);
}


void FileManager::addFile(lyz::GNSSFile* f){
    _files.append(f);

    QString path = QString::fromStdString(f->path());
    QString cont = "";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        cont = "Error";
    else{
        QTextStream qss(&file);
         cont = qss.readAll();
    }

    _content.append(cont);
    notify();
}


int FileManager::closeFile(size_t index){
    if (index >= _files.size())
        return -1;

    delete _files[index];

    _files.removeAt(index);
    _content.removeAt(index);

}


FileManager* FileManager::_instance = nullptr;


FileManager* FileManager::instance(){
    if (!_instance)
        _instance = new FileManager();

    return _instance;
}


FileManager::~FileManager(){
    clearMemory();
}


void FileManager::clearMemory(){
    for (QVector<lyz::GNSSFile*>::const_iterator it =
         _files.begin(); it != _files.end(); it++){

        if (*it)
            delete *it;
    }
    _files.clear();
    _content.clear();
}


void FileManager::destroyInstance(){
    if (_instance){
        _instance->clearMemory();
        delete _instance;
        _instance = nullptr;
    }
}


QStringList FileManager::getNames()const{
    QStringList names;
    for (QVector<lyz::GNSSFile*>::const_iterator it =
         _files.begin(); it != _files.end(); it++){
        lyz::GNSSFile* file = *it;
        QFileInfo path = QString::fromStdString(file->path());
        names.push_back(path.baseName());
    }

    return names;
}


QString FileManager::getContent(size_t index){
    if (index < _content.size())
        return _content[index];
    else
        return "";
}


lyz::PositionPtr FileManager::compute(
        const size_t& index,
        const std::string& prn,
        const lyz::Date& date){

    if (index > _files.size())
        return nullptr;

    return _files[index]->compute(prn, date);

}




QStringList FileManager::getPrn(size_t fileIndex){
    QStringList list;
    if (fileIndex >= _files.size())
        return list;


    std::vector<std::string> names = _files[fileIndex]->getPrn();
    for (size_t i = 0; i < names.size(); i++){
        list.append(QString::fromStdString(names[i]));
    }

    return list;
}


lyz::Date FileManager::getFileDate(size_t fileIndex){
    return _files[fileIndex]->getDateStart();
}


int FileManager::saveFile(size_t fileIndex, const QString& path){
    return _files[fileIndex]->writeSP3(path.toStdString(), 5);
}
