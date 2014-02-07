#include <iostream>

using namespace std;

class schedulerLevel;
class Process;

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
	void addProcess(Process);
	void removeProcess(Process);
};