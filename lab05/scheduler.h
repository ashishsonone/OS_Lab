#include <iostream>
#include "utils.h"
#include <queue>

using namespace std;


enum process_state{
	ready_state,
	block_state
} ;

struct PCB {
	int pid;
	int priority;
	process_state state;
	list<process_phase> Phases;
	friend bool operator <(const PCB& lhs, const PCB& rhs){
         return lhs.priority < rhs.priority;
    }
} ;


class schedulerLevel;

class scheduler{
public:

	scheduler();
	~scheduler();

	priority_queue<PCB, vector<PCB > > ready_PCBList;							// PCB list will help scheduler to determine the process to run from the list of ready processes
	list<PCB > blocked_PCBList;						// processes which are blocked are stored in this list
	bool preemption;
	int currprocess_start_time;

	int addProcess(struct process);
	Event IO_start();
	int IO_terminate(int);
	Event schedule();
	void save_state();
};
