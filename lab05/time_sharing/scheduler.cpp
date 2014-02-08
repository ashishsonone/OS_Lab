#include "scheduler.h"


/**

class schedulerLevel{
public:

	schedulerLevel(int,int,int);
	~schedulerLevel();

	vector<Process > PCBList;							// PCB list will help scheduler to determine the process to run
	int scheduling_level;
	int priority;
	int timeslice;

private:
	int get_scheduler_level();
	int get_priority();
	int get_timeslice();
	void addProcess(struct process);
	void removeProcess(struct process);
	void blockProcess();
};

if (timer_flag != 1)			// it might have been set up by another interrupt, eg:- IO-terminate
			timer_flag = 0; 			// in this no check for priority because process has terminated

**/

/**
	* timer_flag = 1 : schedule must output an timer_event
**/

scheduler::scheduler()
{
	currprocess_start_time = GLOBALCLOCK;
	timer_flag = 0;
}

void scheduler::addProcess(process newProcess){
	PCB newPCB;
	newPCB.pid = newProcess.p_id;
	newPCB.priority = newProcess.start_priority;
 	newPCB.state = ready_state;
 	newPCB.Phases.assign(newProcess.phases.begin(), newProcess.phases.end());
	/**

					NEED TO CHECK IT 
		
	**/
	if (ready_PCBList.empty()){
 		timer_flag = 1;
 	} 	
 	ready_PCBList.push(newPCB);
}

/**
	* this function is called whenever a timer_event is encountered and the current process is to be preempted
	* if the ready process list was empty, then no need to schedule another timer event
	* timer_flag = 0 : when schedule is called it will not create a timer event
**/

void scheduler::timer_handler(){
	// Event timer;
	if(ready_PCBList.empty()) {
		cout << "In Timer Handler : Should not come here " <<endl;
		return;
	}
	save_state();
	//PCB = ready_PCBList.pop();
	timer_flag = 1;	
}

/**
	* this function is called every time after a series of events at some given time
	* it returns either an IO start event or an timer event
	* io start event returned if current process wants to start io but its time slice is not over
	* timer event returned if current processes time slice is expired
**/


Event scheduler::schedule(){
	Event preempt;
	preempt.p_id = -1;
	if (timer_flag == 1 && ready_PCBList.size() != 0){
		timer_flag = 0;											// reset the timer flag 
		int addtime = ready_PCBList.top().front().cpu_time;
		currprocess_start_time = GLOBALCLOCK;
		preempt.p_id = ready_PCBList.top().pid;
		/**

					NEED TO CHECK IT 
		
		**/
		if (addtime > TIMER){
			preempt.type = Timer_Event;
			preempt.time = GLOBALCLOCK + TIMER;
		}
		else {
			preempt.type = Start_IO;
			preempt.time = GLOBALCLOCK + addtime;
		}
		return preempt;
	}	
	else {
		return preempt;			// with pid = -1  i.e either ready list empty or timer_flag is not set which means not change to schedule
	}
}

/**
	* this function handels IO start event
**/
Event scheduler::IO_start(){

	int clock = GLOBALCLOCK;												// current global time
	int delta = ready_PCBList.top().Phases.front().io_time;					// new event creation of I/O interrupt type for iostart function
	clock = clock + delta;
	Event IO_interrupt;
	IO_interrupt.type = End_IO;
	IO_interrupt.p_id = ready_PCBList.top().pid;
	IO_interrupt.time = clock;												// IO_interrupt is defined, and this function returns this event to the event handler

	PCB blockPCB = ready_PCBList.top();
	ready_PCBList.pop();
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
	
	if (flag == 1)	blocked_PCBList.push_back(blockPCB);

	timer_flag = 1;					// now we need to schedule new process, in each case so we set timer flag
	return IO_interrupt;	
}




int scheduler::IO_terminate(int p_id){
	int priority_flag = 0;
	PCB freedPCB;

	list<PCB >::iterator it = blocked_PCBList.begin();	
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
	if (freedPCB.priority > ready_PCBList.top().priority){
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
**/
	else {
		/**

					NEED TO CHECK IT 
		
		**/
		if (ready_PCBList.empty()) timer_flag = 1;					
		
		ready_PCBList.push(freedPCB);
		return 0;
	}
}

void scheduler::save_state(){
	PCB topPCB = ready_PCBList.top();
	ready_PCBList.pop();
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
	ready_PCBList.push(topPCB);
}