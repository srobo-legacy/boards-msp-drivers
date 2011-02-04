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

void sched_init(void);

/* Add a task to the schedule queue */
void sched_add(const sched_task_t *task);

/* Remove a task from the schedule queue */
void sched_rem(const sched_task_t *task);

#endif /* __SCHED_H */
