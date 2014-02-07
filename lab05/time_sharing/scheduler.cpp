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


**/

scheduler::scheduler()
{
	currprocess_start_time = 0;
	preemption = 0;
}

void scheduler::addProcess(process newProcess){
	PCB newPCB;
	newPCB.pid = newProcess.p_id;
	newPCB.priority = newProcess.start_priority;
 	newPCB.state = ready_state;
 	newPCB.Phases.assign(newProcess.phases.begin(), newProcess.phases.end());
 	ready_PCBList.push(newPCB);
 	preemption = 0;
 	
}

Event scheduler::schedule(int type){
	Event start_IO;
	start_IO.p_id = -1;
	if (preemption == 0){
		currprocess_start_time = GLOBALCLOCK;
		start_IO.p_id = ready_PCBList.top().pid;
		start_IO.type = Start_IO;
		start_IO.time = GLOBALCLOCK + ready_PCBList.top().Phases.front().cpu_time;
		return start_IO;
	}	
	else {
		return start_IO;
	}
}


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
	currprocess_start_time = GLOBALCLOCK;

/**		
				COMMENT BLOCK
	* if IO burst = 0 and its last iteration(=1) of last phase : terminte process and return pid = -1 in Event.	
	* if IO burst != 0, and iteration = 1 : pop that phase and return pid = process pid
	* if IO burst != 0, and iteration > 1 : iteration = iteration - 1 and return pid = process pid

**/
	int IO_burst = blockPCB.Phases.front().io_time;
	int iteration = blockPCB.Phases.front().iterations;
	int flag = 1;  					// if process terminates then dont push(flag = 0) in blocked pcb list

	if (IO_burst == 0){										//  terminate process if  burst = 0 :
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
		preemption = 1; 			// in this no check for priority because process has terminated
		return 0;
	}

	if (freedPCB.priority > ready_PCBList.top().priority){
		save_state();
		ready_PCBList.push(freedPCB);
		preemption = 0;
		currprocess_start_time = GLOBALCLOCK;
		return -1;
	}
	else {
		ready_PCBList.push(freedPCB);
		preemption = 1;
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