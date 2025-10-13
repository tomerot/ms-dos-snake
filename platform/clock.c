#include "platform\clock.h"
#include "hardware\rtc.h"

/* Starts the tick count that every wait and interval timer runs on. */
void start_clock(void) {
    set_rtc_handler();
}

/* Stops the tick count. Must run before the program exits, or the CPU
   keeps calling the tick handler after the program's memory is freed. */
void stop_clock(void) {
    set_old_rtc_handler();
}

/* Waits ms milliseconds, doing nothing else. Needs start_clock first:
   without ticks, it waits forever. */
void wait_ms(int ms) {
    /* (long) keeps ms * RTC_TICKS_PER_SECOND from overflowing int's 16
       bits once ms is above 31. */
    unsigned long end_tick = current_tick() + (long)ms * RTC_TICKS_PER_SECOND / 1000;

    while (current_tick() < end_tick);
}

/* Sets t to go off intervals_per_second times a second, starting now.
   Each interval is a whole number of ticks, rounded down, so the timer
   can run slightly fast (0.4% at 10 a second). */
void start_interval_timer(interval_timer *t, int intervals_per_second) {
    t->ticks_per_interval = RTC_TICKS_PER_SECOND / intervals_per_second;
    t->alarm_tick = current_tick() + t->ticks_per_interval;
}

/* TRUE once for each interval that has passed. If the caller is late,
   the missed intervals still come, one per call, so none is lost. */
bool has_interval_passed(interval_timer *t) {
    if (current_tick() < t->alarm_tick)
        return FALSE;

    /* The next alarm counts from the last one, not from now, so a late
       call doesn't push the later intervals back. */
    t->alarm_tick += t->ticks_per_interval;
    return TRUE;
}
