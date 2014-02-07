#include <iostream>
#include <vector>
#include <queue>
using namespace std;

int GLOBALCLOCK = 0;

enum EventType{
    Start_IO,
    End_IO,
    Admission
};

struct Event{
    EventType type;
    int pid;
    int time;
    friend bool operator <(const Event& lhs, const Event& rhs) //friend claim has to be here
      {
         return lhs.time > rhs.time;
      }
};

class Event_Manager{
    //priority_queue <Event, vector<Event> > eventlist;
    Event * dangling_event;
};
