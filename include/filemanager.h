#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <QVector>
#include <QStringList>
#include "datatype.h"
#include "gnssfile.h"
#include "observer.h"
#include "mat.h"


class FileManager
{
public:

    ~FileManager();

    // notify all of the observers
    // when new files are added
    void notify();
    void addObserver(Observer* observer);
    void addFile(lyz::GNSSFile*);
    bool empty(){ return _files.empty(); }
    void clearMemory();
    QStringList getNames()const;
    QString getContent(size_t index);
    QStringList getPrn(size_t fileIndex);
    int closeFile(size_t index);

    lyz::Date getFileDate(size_t fileIndex);
    int saveFile(size_t fileIndex, const QString& path);
    lyz::PositionPtr compute(
            const size_t& index,
            const std::string& prn,
            const lyz::Date& date);



    static FileManager* instance();
    static void destroyInstance();

private:
    // singleton
    FileManager(){}
    static FileManager* _instance;

    QVector<lyz::GNSSFile*> _files;
    QVector<Observer*> _observers;
    QVector<QString> _content;
};

#endif // FILEMANAGER_H
