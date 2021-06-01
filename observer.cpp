#include "observer.h"

void ListObserver::execute(){
    _dock->updateElement();
}


void WidgetObserver::execute(){
    _widget->updateElement();
}


