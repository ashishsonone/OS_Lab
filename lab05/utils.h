#ifndef _UTILS
#define  _UTILS

#include <list>
#include <vector>

static int GLOBALCLOCK = 0;
using namespace std;
struct process_phase{
	int iterations;
	int cpu_time;
	int io_time;
};

struct process{
	int p_id;
	int start_priority;
	int admission;
	list<process_phase> phases;
};

struct sc_level{
	int level_number;
	int priority;
	int time_slice;
};

struct scheduler{
	int no_levels;
	vector<sc_level> levels;
};

enum EventType{
    Start_IO,
    End_IO,
    Admission
};

struct Event{
    EventType type;
    int p_id;
    int time;
    friend bool operator <(const Event& lhs, const Event& rhs) //friend claim has to be here
      {
         return lhs.time > rhs.time;
      }
};

void print_event(Event e);

#endif
