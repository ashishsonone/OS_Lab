#include "event_manager.h"
int main(){
    priority_queue <Event, vector<Event> > eventlist;
    Event e1 = {Start_IO, 2, 3};
    Event e2 = {Start_IO, 2, 0};
    Event e3 = {Start_IO, 2, 5};
    eventlist.push(e1);
    eventlist.push(e2);
    eventlist.push(e3);
    Event e;
    e = eventlist.top();
    eventlist.pop();
    cout << e.time <<endl;
    e = eventlist.top();
    eventlist.pop();
    cout << e.time <<endl;
}
