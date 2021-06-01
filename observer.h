#ifndef OBSERVER_H
#define OBSERVER_H
#include "dockwidget.h"
#include "contentwidget.h"

class Observer
{
public:
    Observer(){}
    virtual void execute() = 0;
};



class ListObserver: public Observer
{
public:
    ListObserver(DockWidget* dock):_dock(dock) {}
    virtual void execute();

private:
    DockWidget* _dock;

};



class WidgetObserver: public Observer
{
public:
    WidgetObserver(ContentWidget* widget):_widget(widget) {}
    virtual void execute();

private:
    ContentWidget* _widget;

};
#endif // OBSERVER_H
