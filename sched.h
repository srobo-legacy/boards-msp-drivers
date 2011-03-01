#ifndef __SCHED_H
#define __SCHED_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	/* Timout in milliseconds */
	uint16_t t;
	/* Callback to be called after timeout.
	 * Return true to execute task again */
	bool (*cb) (void *udata);
	/* Pointer to user data, is passed to the callback */
	void *udata;
} sched_task_t;

/* The time */
extern volatile uint16_t sched_time;

/* Return the number of ticks since before */
#define sched_time_since(before) ( sched_time >= before? sched_time - before : (0xffff - before) + sched_time )

void sched_init(void);

/* Add a task to the schedule queue */
void sched_add(const sched_task_t *task);

/* Remove a task from the schedule queue */
void sched_rem(const sched_task_t *task);

#endif /* __SCHED_H */
