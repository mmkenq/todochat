#ifndef USUAL_TASK_H
#define USUAL_TASK_H

#include "./primitives/color.h"
#include "./primitives/date.h"
#include "./primitives/time.h"

struct utask {
	struct status {
		char is_completed:1;
		char in_unmarked: 1;
		char is_canceled: 1;
		char is_postponed:1;
	}
	struct date start_date;
	struct date end_date;
	struct time start_time;
	struct time end_time;

	struct color color;
};

#endif