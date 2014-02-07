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
{}

Event scheduler::addProcess(process newProcess){
	PCB newPCB;
	newPCB.pid = newProcess.p_id;
	newPCB.priority = newProcess.start_priority;
 	newPCB.state = ready_state;
 	newPCB.Phases.assign(newProcess.phases.begin(), newProcess.phases.end());
 	ready_PCBList.push_back(newPCB);
}

Event scheduler::IO_start(){

	int clock = GLOBALCLOCK;												// current global time
	int delta = ready_PCBList.top().Phases.front().io_time;					// new event creation of I/O interrupt type for iostart function
	clock = clock + delta;
	Event IO_interrupt;
	IO_nterrupt.type = End_IO;
	IO_interrupt.p_id = ready_PCBList.top().pid;
	IO_interrupt.time = clock;												// IO_interrupt is defined, and this function returns this event to the event handler

	PCB blockPCB = ready_PCBList.top();
	ready_PCBList.pop();

/**		
				COMMENT BLOCK
	* if IO burst = 0 and its last iteration(=1) of last phase : terminte process and return pid = -1 in Event.
	* if IO burst = 0 and its last iteration of a phase : pop that phase and return pid = -1 in Event.
	* if IO burst = 0 and its not the last iteration of a phase : iteration-- and return pid = -1 in Event.	
	* if IO burst != 0, and iteration = 1 : pop that phase and return pid = process pid
	* if IO burst != 0, and iteration > 1 : iteration = iteration - 1 and return pid = process pid

**/
	int IO_burst = blockPCB.Phases.front().io_time;
	int iteration = blockPCB.Phases.front().iterations;
	if (iteration == 1 && IO_burst == 0){							
		blockPCB.Phases.pop_front();										
	}
	else {
		blockPCB.Phases.front().iterations = blockPCB.Phases.front().iterations - 1;
	}

	if (blockPCB.Phases.empty()){

	}
	blocked_PCBList.push_back(blockPCB);
}



Event scheduler::schedule(){

}

int scheduler::IO_terminate(int pid){

}