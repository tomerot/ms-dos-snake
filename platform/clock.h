#ifndef CLOCK_H
#define CLOCK_H

#include "common.h"

/* An alarm that goes off at a steady rate: alarm_tick is the tick of the
   next alarm, ticks_per_interval the time between alarms. Set it up with
   start_interval_timer, then ask has_interval_passed. */
typedef struct {
    unsigned long alarm_tick;
    int ticks_per_interval;
} interval_timer;

void start_clock(void);
void stop_clock(void);

void wait_ms(int ms);

void start_interval_timer(interval_timer *t, int intervals_per_second);
bool has_interval_passed(interval_timer *t);

#endif
