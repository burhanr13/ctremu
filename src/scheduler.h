#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"

typedef enum {
    EVENT_GSP,

    EVENT_MAX = 128,
} EventType;

typedef struct {
    u64 time;
    EventType type;
    u32 arg;
} SchedulerEvent;

typedef struct _3DS HLE3DS;

typedef struct {
    u64 now;

    HLE3DS* master;

    FIFO(SchedulerEvent, EVENT_MAX) event_queue;
} Scheduler;

void run_to_present(Scheduler* sched);
int run_next_event(Scheduler* sched);

static inline bool event_pending(Scheduler* sched) {
    return sched->event_queue.size &&
           sched->now >= FIFO_peek(sched->event_queue).time;
}

void add_event(Scheduler* sched, EventType t, u32 event_arg, s64 reltime);
void remove_event(Scheduler* sched, EventType t);
u64 find_event(Scheduler* sched, EventType t);

void print_scheduled_events(Scheduler* sched);

#endif