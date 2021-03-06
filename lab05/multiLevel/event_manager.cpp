#include "event_manager.h"

int GLOBALCLOCK;

EventManager::EventManager(vector<process> pl, scheduler_in sch_in){
    sch = new scheduler(sch_in);
    GLOBALCLOCK = 0;
    process_list = pl;
    danglingEvent.p_id = -1;
    for(int i=0; i<process_list.size(); i++){
        process p = process_list[i];
        Event e = {Admission, p.p_id, GLOBALCLOCK + p.admission, p.start_priority};
        event_list.push(e);
    }
}

EventManager::~EventManager(){
}

void EventManager::handle_event(Event e){
    if(e.type == Start_IO){
        cout << "handling Start_IO event "; print_event(e);
        danglingEvent.p_id = -1; //no dangling event(timeslice or iostart) left.
        Event e = sch->IO_start();//e is of type End_IO, if p_id=-1 means that process ended, else means it needs to added to eventlist
        if(e.p_id != -1){
            cout << "adding event IO_terminate to EventList "; print_event(e);
            event_list.push(e);
        }
        else{ //process had no io, and has ended
        }
    }
    else if(e.type == End_IO){
        cout << "Event end io "; print_event(e);
	    int result = sch->IO_terminate(e.p_id, e.priority); //process will be pushed from blocked to the queue(if still has something left to do)
        if(result == -1){ //curr process premted, so set danglingEvent to -1
            danglingEvent.p_id = -1;
        }
        else{ //no preemption
        }
    }
    else if(e.type == Admission){
        cout << "Event Admission "; print_event(e);
        int p_id = e.p_id;
        for(int i=0; i<process_list.size(); i++){
            process pro = process_list[i];
            if(pro.p_id == p_id){ //process found
                int result = sch->addProcess(pro);
                if(result == -1){ //this means the curr process has been premeted, so remove dangling_Start_IO
                    danglingEvent.p_id = -1;
                }
                else{ //result = 0 , no changes need to be made
                }
                return;
            }
        }
        cout << "Error while admitting: process with pid : " << e.p_id << " not found in process list" <<endl;
    }
    else if(e.type == Timer_Event){
        cout << "handling Timer Event "; print_event(e);
        danglingEvent.p_id = -1; //no danglingEvent left
        sch->timer_handler();
    }
}

void EventManager::run(){
    while(1){
        cout <<endl;
        //schedule() called at end
        cout << "looking at rem events" <<endl;
        cout << "Event list size " << event_list.size() <<endl;
        if(danglingEvent.p_id != -1) {
            cout << "danglingEvent event ";
            print_event(danglingEvent);
        }

        if(event_list.empty() && danglingEvent.p_id == -1){
            cout << "EventList empty : exiting"<<endl;
            break;  // we are done
        }
        cout << "$$\n";

        Event e;
        if(!event_list.empty()){
            e = event_list.top();
            if(danglingEvent.p_id != -1 && danglingEvent.time <= e.time){
                GLOBALCLOCK = danglingEvent.time; //advance global clock
                cout << "Advancing GLOBALCLOCK to dangling  " << GLOBALCLOCK <<endl;
                //cout << "handling event  dangle"; print_event(danglingEvent);
                handle_event(danglingEvent);
                danglingEvent.p_id = -1;
            }
            else{
                e = event_list.top();
                GLOBALCLOCK = e.time; //advance global clock
                cout << "Advancing GLOBALCLOCK to notempty  " << GLOBALCLOCK <<endl;
                handle_event(e);
                //cout << "handling event  top"; print_event(e);
                event_list.pop();
            }
        }
        else{ //ony danglingEvent exists
            GLOBALCLOCK = danglingEvent.time; //advance global clock
            cout << "Advancing GLOBALCLOCK  " << GLOBALCLOCK <<endl;
            //cout << "handling event hhh"; print_event(danglingEvent);
            handle_event(danglingEvent);
            danglingEvent.p_id = -1;
        }

        while(!event_list.empty()){ //handle all events with timestamp = GLOBALCLOCK
           // cout << " 003 Advancing GLOBALCLOCK to notempty" << GLOBALCLOCK <<endl;
            e = event_list.top();
            if(e.time != GLOBALCLOCK){
                break;
            }
            handle_event(e);
            //cout << "handling event "; print_event(e);
            event_list.pop();
        }

        cout << "calling schedule " << endl;
        e = sch->schedule();

        if(e.p_id != -1){ //event is of type IO_start, pid!=-1 means new process scheduled so set its startIO, set the danglingEvent to this event
            //setting start io
            cout << "setting danglingEvent "; print_event(e);
            danglingEvent = e;
        }
        else{ //means no process in the ready heap
        }
    }
}
