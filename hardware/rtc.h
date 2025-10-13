#ifndef RTC_H
#define RTC_H

#define RTC_TICKS_PER_SECOND 1024  /* the interrupt rate set_rtc_handler sets */

void set_rtc_handler(void);
void set_old_rtc_handler(void);

unsigned long current_tick(void);

#endif
