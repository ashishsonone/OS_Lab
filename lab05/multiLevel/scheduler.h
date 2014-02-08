#include <iostream>
#include "utils.h"
#include <queue>
#include <list>

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
} ;

struct scheduler_level{
	int level_number;
	int priority;
	int time_slice;
	queue<PCB, list<PCB > > ready_PCBList;			// PCB list will help scheduler to determine the process to run from the list of ready processes
	list<PCB > blocked_PCBList;						// processes which are blocked are stored in this list
};

class scheduler;

class scheduler{
public:

	scheduler(scheduler_in);
	~scheduler();

	map<int, int> priority_level_map;				// maps priority of a process to the scheduler level, which is position of a level in the levels list
	vector<scheduler_level > levelslist;			// the list of scheduler levels 
	int currprocess_level_index;					// its -1 if no process running at a specific time otherwise index of highest level with a process in ready pcb list, i.e the level with running process
	int currprocess_start_time;
	int number_of_levels;							// 

	int addProcess(struct process);
	Event IO_start();
	int IO_terminate(int,int);
	Event schedule();
	void save_state();
	void timer_handler();
};
