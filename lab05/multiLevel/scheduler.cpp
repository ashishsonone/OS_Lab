#include "scheduler.h"
//extern int TIMER;			
extern int GLOBALCLOCK;


/**

if (timer_flag != 1)			// it might have been set up by another interrupt, eg:- IO-terminate
			timer_flag = 0; 			// in this no check for priority because process has terminated

**/

/**
	* timer_flag = 1 : schedule must output an timer_event
**/

scheduler::scheduler(scheduler_in s_in)
{
	currprocess_level = -1;
	number_of_levels = s_in.no_levels;
	for (int i = 0; i < number_of_levels ; i ++){
		scheduler_level newlevel;
		newlevel.level_number = s_in.levels[i].level_number;
		newlevel.priority = s_in.levels[i].priority;
		newlevel.time_slice = s_in.levels[i].time_slice;
		levelslist.push_back(newlevel);
		map[newlevel.priority] = i; 
	}
}

/**
	struct sc_level{
	int level_number;
	int priority;
	int time_slice;
};
struct scheduler_level{
	int level_number;
	int priority;
	int time_slice;
	queue<PCB, list<PCB > > ready_PCBList;			// PCB list will help scheduler to determine the process to run from the list of ready processes
	list<PCB > blocked_PCBList;						// processes which are blocked are stored in this list
};

struct scheduler_in{
	int no_levels;
	vector<sc_level> levels;
};
	
**/

scheduler:: ~scheduler()
{
}

int scheduler::addProcess(process newProcess){
	PCB newPCB;
	newPCB.pid = newProcess.p_id;
	newPCB.priority = newProcess.start_priority;
 	newPCB.state = ready_state;
 	newPCB.Phases.assign(newProcess.phases.begin(), newProcess.phases.end());
	/**

					NEED TO CHECK IT 
		* case 0 : every ready pcb list is empty
		* 
	**/
	 	
 	if (currprocess_level_index == -1){
 		int index = priority_level_map[newPCB.priority];
 		levelslist[index].ready_PCBList.push(newPCB);
 		return 0;
 	}

 	else if (newPCB.priority > levelslist[currprocess_level_index].priority){
 		save_state();
 		currprocess_level_index = -1;
 		int index = priority_level_map[newPCB.priority];
 		levelslist[index].ready_PCBList.push(newPCB); 
 		return -1;
 	}

 	else {
 		int index = priority_level_map[newPCB.priority];
 		levelslist[index].ready_PCBList.push(newPCB); 
 		return 0;
 	}
}

/**
	* this function is called whenever a timer_event is encountered and the current process is to be preempted
	* if the ready process list was empty, then no need to schedule another timer event
	* timer_flag = 0 : when schedule is called it will not create a timer event
**/

void scheduler::timer_handler(){
	int index = currprocess_level_index;
	if(index == -1 || levelslist[index].ready_PCBList.empty()) {
		cout << "In Timer Handler : Should not come here. Index is : " << index <<endl;
		return;
	}
	save_state();
	PCB downgradePCB;
	downgradePCB = levelslist[index].ready_PCBList.front();
	levelslist[index].ready_PCBList.pop();
	if (index > 0){
		index--;
	}
	downgradePCB.priority = levelslist[index].priority;
	levelslist[index].ready_PCBList.push(downgradePCB);
	currprocess_level_index = -1;
}

/**
	* this function is called every time after a series of events at some given time
	* it returns either an IO start event or an timer event
	* io start event returned if current process wants to start io but its time slice is not over
	* timer event returned if current processes time slice is expired and next process' cpu time is more than time slice 
**/


Event scheduler::schedule(){
	Event preempt;
	preempt.p_id = -1;
	int index = -1;
	for (int i = number_of_levels - 1; i >= 0; i--){
		if (!levelslist[i].ready_PCBList.empty()){
			index = i;
			break;
		}
	}
	if (index == -1 || currprocess_level_index != -1){
		return preempt;
	}

	currprocess_level_index = index;

	int addtime = levelslist[index].ready_PCBList.front().Phases.front().cpu_time;
	currprocess_start_time = GLOBALCLOCK;
	preempt.p_id = levelslist[index].ready_PCBList.front().pid;
	preempt.priority = levelslist[index].priority;
	/**

				NEED TO CHECK IT 
	
	**/
	if (addtime > levelslist[i].time_slice){
		preempt.type = Timer_Event;
		preempt.time = GLOBALCLOCK + levelslist[i].time_slice;
	}
	else {
		preempt.type = Start_IO;
		preempt.time = GLOBALCLOCK + addtime;
	}
	return preempt;
}

/**
	* this function handels IO start event
	* it returns the correspoding IO termintaion event
	* 
**/

/**
	PCB downgradePCB;
	downgradePCB = levelslist[index].ready_PCBList.front();
	levelslist[index].ready_PCBList.pop();
	if (index > 0){
		index--;
	}

	downgradePCB.priority = levelslist[index].priority;
	levelslist[index].ready_PCBList.push(downgradePCB);
	currprocess_level_index = -1;
**/

Event scheduler::IO_start(){
	int index = currprocess_level_index;
	int clock = GLOBALCLOCK;															// current global time
	int elapsed_time = currprocess_start_time - clock;									// current cpu time of process
	int delta = levelslist[index].ready_PCBList.front().Phases.front().io_time;			// new event creation of I/O interrupt type for iostart function
	clock = clock + delta;
	
	Event IO_interrupt;
	IO_interrupt.type = End_IO;
	IO_interrupt.p_id = levelslist[index].ready_PCBList.front().pid;
	IO_interrupt.time = clock;												// IO_interrupt is defined, and this function returns this event to the event handler

	PCB blockPCB = levelslist[index].ready_PCBList.front();
	levelslist[index].ready_PCBList.pop();
	/**
		* we need to demote because the process has used the entire time slice.
		* we upgrade process only if process runs for time < time slice  
	**/
	if (elapsed_time == levelslist[index].time_slice){
		if (index > 0){
			index--;
		}
		blockPCB.priority = levelslist[index].priority;
		//levelslist[index].ready_PCBList.push(blockPCB);
	}
	else {
		if (index < number_of_levels - 1){
			index++ ;
		}
		blockPCB.priority = levelslist[index].priority;
		//levelslist[index].ready_PCBList.push(blockPCB);
	}
	//currprocess_start_time = GLOBALCLOCK;

/**		
				COMMENT BLOCK
	* if IO burst = 0 and its last iteration(=1) of last phase : terminte process and return pid = -1 in Event.	
	* if IO burst != 0, and iteration = 1 : pop that phase and return pid = process pid
	* if IO burst != 0, and iteration > 1 : iteration = iteration - 1 and return pid = process pid

**/
	int IO_burst = blockPCB.Phases.front().io_time;
	int iteration = blockPCB.Phases.front().iterations;
	int flag = 1;  					// if process terminates then dont push:(flag = 0) in blocked pcb list

	if (IO_burst == 0){										
		cout << "Process with pid : " << blockPCB.pid  << "has been terminated.\n"; 
		IO_interrupt.p_id = -1;	
		flag = 0;											// it has to be last iteration of process
	}
	else if (iteration == 1 ){							
		blockPCB.Phases.pop_front();
		flag = 1;
	}
	else if (iteration > 1 ){
		blockPCB.Phases.front().iterations = iteration - 1;
		flag = 1;
	}
	
	if (flag == 1)	levelslist[index].blocked_PCBList.push_back(blockPCB);

	IO_interrupt.priority = levelslist[index].priority;

	return IO_interrupt;	
}

/**
	*
**/

/**
int scheduler::IO_terminate(int p_id, int priority){
	int priority_flag = 0;
	PCB freedPCB;
	int index = priority_level_map[priority];	

	list<PCB >::iterator it = levelslist[index].blocked_PCBList.begin();	
	while(it!=blocked_PCBList.end()){
		if (it->pid == p_id){
			freedPCB = *it;
			blocked_PCBList.erase(it);				// process removed from the blocked process list
			break;
		}
		it++;
	}

	if (freedPCB.Phases.empty()){
		cout << "Process with pid : " << freedPCB.pid  << "has been terminated.\n";
		return 0;
	}
/**
	if (freedPCB.priority > ready_PCBList.front().priority){
		save_state();
		ready_PCBList.push(freedPCB);
		timer_flag = 0;
		currprocess_start_time = GLOBALCLOCK;
		return -1;
	}

**/
/**
	* if the ready list is empty then scheduler must check for new process when its called so set timer flag
	* if ready list is not empty, then the top function will be in execution and will be preempted only on expiration of its slice

	else {
		/**

					NEED TO CHECK IT 
		
		
		if (levelslist[index].ready_PCBList.empty()) currprocess_level_index = -1;					
		
		levelslist[index].ready_PCBList.push(freedPCB);
		return 0;
	}
}



**/




int scheduler::IO_terminate(int p_id, int priority){
    //cout << "IO Terminate entry time premtion " << preemption <<endl;
    //cout << "ready pcb list size IO Terminate" << ready_PCBList.size() <<endl;
	int priority_flag = 0;
	PCB freedPCB;
	int index = priority_level_map[priority];	

	list<PCB >::iterator it = levelslist[index].blocked_PCBList.begin();	
	while(it!=blocked_PCBList.end()){
		if (it->pid == p_id){
			freedPCB = *it;
			blocked_PCBList.erase(it);				// process removed from the blocked process list
			break;
		}
		it++;
	}

    //cout << "ready pcb list size IO Terminate" << ready_PCBList.size() <<endl;
	if (freedPCB.Phases.empty()){
		cout << "Process with pid : " << freedPCB.pid  << "has been terminated" << " @ time ----" << GLOBALCLOCK <<endl;
		//preemption = 1; 			// in this no check for priority because process has terminated

		return 0;
	}

     
    if (index > currprocess_level_index){
    	if (currprocess_level_index != -1)
    		save_state();

    	currprocess_level_index = -1;
    	
		levelslist[index].ready_PCBList.push(freedPCB);
		//currprocess_start_time = GLOBALCLOCK;
		return -1;
    }
	else {
		levelslist[index].ready_PCBList.push(freedPCB);
		return 0;
	}
}






















/**
	* this function will save the state of the current running process
	* it will not worry about what to be done after that, 
	* the correspinding event handler that called this function will take care of the rest 
	* no argument
**/

void scheduler::save_state(){
	int index = currprocess_level_index;
	PCB & topPCB = levelslist[index].ready_PCBList.front();				// pointer used because we need to update top itself
	int iters = topPCB.Phases.front().iterations;
	topPCB.Phases.front().iterations = iters - 1;

	int newcpu_time = topPCB.Phases.front().cpu_time;
	process_phase newphase;
	newphase.iterations = 1;
	newphase.cpu_time = newcpu_time - (GLOBALCLOCK - currprocess_start_time);
	newphase.io_time  = topPCB.Phases.front().io_time;
	if (iters == 1){
		topPCB.Phases.pop_front();
	}
	topPCB.Phases.push_front(newphase);
}
