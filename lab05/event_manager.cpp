#include "event_manager.h"

int GLOBALCLOCK;

EventManager::EventManager(vector<process> pl){
    GLOBALCLOCK = 0;
    process_list = pl;
    dangling_Start_IO.p_id = -1;
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
        dangling_Start_IO.p_id = -1; //no io start event left.
        Event e = sch.IO_start();//e is of type End_IO, if p_id=-1 means that process ended, else means it needs to added to eventlist
        if(e.p_id != -1){
            cout << "adding event IO_terminate to list"; print_event(e);
            event_list.push(e);
        }
        else{ //process had no io, and has ended
        }
    }
    else if(e.type == End_IO){
        int p_id = e.p_id;
	    int result = sch.IO_terminate(p_id);
        if(result == -1) { //curr process has been premted, remove dangling_Start_IO
            cout << "Premting current process " << endl;
            dangling_Start_IO.p_id = -1;
        }
        else{ //result = 0 , no changes need to be made
        }
    }
    else if(e.type == Admission){
        int p_id = e.p_id;
        for(int i=0; i<process_list.size(); i++){
            process pro = process_list[i];
            if(pro.p_id == p_id){ //process found
                int result = sch.addProcess(pro);
                if(result == -1){ //this means the curr process has been premeted, so remove dangling_Start_IO
                    cout << "Premting current process " << endl;
                    dangling_Start_IO.p_id = -1;
                }
                else{ //result = 0 , no changes need to be made
                }
                cout << "Adding Process " << p_id <<endl;
            }
        }
    }
}

void EventManager::run(){
    while(1){
        cout <<endl;
        //schedule() called at end
        cout << "Event list size " << event_list.size() <<endl;
        if(event_list.empty() && dangling_Start_IO.p_id == -1){
            cout << "EventList empty : exiting"<<endl;
            break;  // we are done
        }

        Event e;
        if(!event_list.empty()){
            e = event_list.top();
            if(dangling_Start_IO.p_id != -1 && dangling_Start_IO.time < e.time){
                GLOBALCLOCK = dangling_Start_IO.time; //advance global clock
                cout << "Advancing GLOBALCLOCK to dangling" << GLOBALCLOCK <<endl;
                cout << "handling event  dangle"; print_event(dangling_Start_IO);
                handle_event(dangling_Start_IO);
                dangling_Start_IO.p_id = -1;
            }
            else{
                e = event_list.top();
                GLOBALCLOCK = e.time; //advance global clock
                cout << "Advancing GLOBALCLOCK to notempty" << GLOBALCLOCK <<endl;
                handle_event(e);
                cout << "handling event  top"; print_event(e);
                event_list.pop();
            }
        }
        else{ //ony dangling_Start_IO exists
            GLOBALCLOCK = dangling_Start_IO.time; //advance global clock
            cout << "Advancing GLOBALCLOCK" << GLOBALCLOCK <<endl;
            handle_event(dangling_Start_IO);
            cout << "handling event hhh"; print_event(dangling_Start_IO);
            dangling_Start_IO.p_id = -1;
        }

        //cout << " 001 Advancing GLOBALCLOCK to notempty" << GLOBALCLOCK <<endl;

        if(dangling_Start_IO.p_id != -1 && dangling_Start_IO.time == GLOBALCLOCK){ //dangling Start IO belongs to this time
            handle_event(dangling_Start_IO);
            cout << "handling event "; print_event(dangling_Start_IO);
            dangling_Start_IO.p_id = -1;
        }
        //cout << " 002 Advancing GLOBALCLOCK to notempty" << GLOBALCLOCK <<endl;

        while(!event_list.empty()){ //handle all events with timestamp = GLOBALCLOCK
           // cout << " 003 Advancing GLOBALCLOCK to notempty" << GLOBALCLOCK <<endl;
            e = event_list.top();
            if(e.time != GLOBALCLOCK){
                break;
            }
            handle_event(e);
            cout << "handling event "; print_event(e);
            event_list.pop();
        }

        cout << "calling schedule " << endl;
        e = sch.schedule();

        if(e.p_id != -1){ //event is of type IO_start, pid!=-1 means new process scheduled so set its startIO, set the dangling_Start_IO to this event
            //setting start io
            cout << "setting dangling_Start_IO "; print_event(e);
            dangling_Start_IO = e;
        }
        else{ //means no process in the ready heap
        }
    }
}
