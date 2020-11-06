#include "helper.hpp"
#include "RMS.h"

int main() {
	vector<task> task_vec;
	int n=10;
	read_input(task_vec);
	print_task_vec(task_vec);
	RMS_Scheduler(task_vec,n);
}
