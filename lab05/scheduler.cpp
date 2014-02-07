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
	* if IO burst != 0, and iteration = 1 : pop that phase and return pid = process pid
	* if IO burst != 0, and iteration > 1 : iteration = iteration - 1 and return pid = process pid

**/
	int IO_burst = blockPCB.Phases.front().io_time;
	int iteration = blockPCB.Phases.front().iterations;
	int flag = 1;  					// if process terminates then dont push(flag = 0) in blocked pcb list

	if (IO_burst == 0){										//  terminate process if  burst = 0 : 
		IO_interrupt.pid = -1;	
		flag = 0;											// it has to be last iteration of process
	}
	else if (iteration == 1 ){							
		blockPCB.Phases.pop_front();	
	}
	else if (iteration > 1 ){
		blockPCB.Phases.front().iterations = iteration - 1;
	}
	
	if (flag == 0)	blocked_PCBList.push_back(blockPCB);
	return IO_interrupt;	
}



Event scheduler::schedule(){

}

int scheduler::IO_terminate(int pid){
	int size = blocked_PCBList.size();
	for (int i = 0; )
}