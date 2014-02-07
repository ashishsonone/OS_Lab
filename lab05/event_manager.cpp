#include "event_manager.h"

EventManager::EventManager(vector<process> pl){
    GLOBALCLOCK = 0;
    process_list = pl;
    for(int i=0; i<process_list.size(); i++){
        process p = process_list[i];
        Event e = {Admission, p.p_id, GLOBALCLOCK + p.admission};
        event_list.push(e);
    }
}

EventManager::~EventManager(){
}

void EventManager::handle_event(Event e){
    if(e.type == Start_IO){
    }
    else if(e.type == End_IO){
    }
    else if(e.type == Admission){
    }
}

void EventManager::run(){
    while(1){
        //schedule() called at end
        cout << "Event list size " << event_list.size() <<endl;
        if(event_list.empty()){
            cout << "EventList empty : exiting"<<endl;
            break;  // we are done
        }

        Event e = event_list.top();
        GLOBALCLOCK = e.time; //advance global clock
        cout << "Advancing GLOBALCLOCK to " << GLOBALCLOCK <<endl;

        handle_event(e);
        cout << "handling event "; print_event(e);
        event_list.pop();


        while(!event_list.empty()){ //handle all events with timestamp = GLOBALCLOCK
            e = event_list.top();
            if(e.time != GLOBALCLOCK){
                break;
            }
            handle_event(e);
            cout << "handling event "; print_event(e);
            event_list.pop();
        }
        //scheduler.schedule();
    }
}
