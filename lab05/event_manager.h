#include <iostream>
#include <vector>
#include <queue>
#include "utils.h"
#include "scheduler.h"

using namespace std;


class EventManager{
public:
    priority_queue <Event, vector<Event> > event_list;
    Event dangling_Start_IO;
    scheduler sch;

    vector<process> process_list;
    EventManager(vector<process> pl); //insert admission events corresponding to process_list
    ~EventManager();
    void handle_event(Event e);
    void run(); //run until event list is empty
};
