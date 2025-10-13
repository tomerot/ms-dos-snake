/* Motorola MC146818 real-time clock (RTC), kept running by a battery on
   the motherboard while the PC is off.

   In a PC it can:
   - keep the date and time, even while the PC is off
   - keep 50 bytes of memory while the PC is off; the BIOS keeps its
     settings there
   - interrupt at a steady rate, from 2 to 8192 times a second
   - interrupt at a set time of day (the alarm)
   - interrupt once a second, each time it updates the time

   People often call the chip and its memory "the CMOS", since it uses
   CMOS technology to keep the 50 bytes of memory alive on a battery.

   This file uses only the steady-rate interrupt, at 1024 times a second,
   and counts each one as a tick. The RTC signals on IRQ 8 (an interrupt
   request line; see pic.c), on the slave PIC; the CPU sees it as
   interrupt 0x70.

   Its registers have no ports of their own: the code writes a register's
   number to port 0x70, then reads or writes that register through port
   0x71. A read can reset that choice, so the code picks the register
   again before writing. Bit 7 of port 0x70 is not part of the number:
   when set, it blocks the non-maskable interrupt (NMI). This file leaves
   it clear. */

#include "hardware\rtc.h"
#include "hardware\pic.h"
#include "common.h"
#include <dos.h>

#define RTC_SELECT_PORT        0x70    /* picks the register 0x71 reaches */
#define RTC_DATA_PORT          0x71    /* reads or writes the picked register */
#define RTC_FREQUENCY_REGISTER 0x0A    /* register A: interrupt rate */
#define RTC_CONTROL_REGISTER   0x0B    /* register B: which interrupts are on */
#define RTC_FLAGS_REGISTER     0x0C    /* register C: which interrupts fired */
#define RTC_TICK_ENABLE_SHIFT  6       /* register B bit 6, PIE: periodic interrupt enable */
#define RTC_RATE_MASK          0x0F    /* register A bits 0-3, RS0-RS3: rate select */
#define RTC_1024HZ_RATE        0x06    /* rate 6: 32768 Hz halved 5 times */
#define RTC_CLOCK_HZ           32768L  /* the crystal the RTC counts */
#define RTC_VECTOR             0x70    /* interrupt number of IRQ 8, the RTC's line */

#if (RTC_CLOCK_HZ >> (RTC_1024HZ_RATE - 1)) != RTC_TICKS_PER_SECOND
#error RTC_1024HZ_RATE does not produce RTC_TICKS_PER_SECOND
#endif

/* Ticks counted so far. volatile because rtc_handler changes it at any
   moment, so the compiler must read it from memory every time. */
static volatile unsigned long tick_count;

/* The handler that was there before; set_old_rtc_handler puts it back. */
static void interrupt (*old_rtc_handler)();

/* Called by the CPU on each RTC interrupt, 1024 times a second. Counts
   one tick. */
static void interrupt rtc_handler(void) {
    tick_count++;

    /* The RTC sends no more interrupts until register C is read. The
       value itself isn't needed. */
    outportb(RTC_SELECT_PORT, RTC_FLAGS_REGISTER);
    inportb(RTC_DATA_PORT);

    /* Tell the PICs this interrupt is done, so they let the next one
       through. */
    pic_eoi_slave();
}

/* Starts counting ticks: puts rtc_handler in place of the old handler,
   and has the RTC interrupt 1024 times a second. */
void set_rtc_handler(void) {
    byte prev;

    /* Block interrupts, so none arrives while the handler and the RTC's
       registers are half changed. */
    disable();

    /* Put rtc_handler in place, saving the old one for
       set_old_rtc_handler to put back. */
    old_rtc_handler = getvect(RTC_VECTOR);
    setvect(RTC_VECTOR, rtc_handler);

    /* Turn the periodic interrupt on. Register B's other bits switch the
       other interrupts and set the date format, so they stay as they are. */
    outportb(RTC_SELECT_PORT, RTC_CONTROL_REGISTER);
    prev = inportb(RTC_DATA_PORT);
    outportb(RTC_SELECT_PORT, RTC_CONTROL_REGISTER);
    outportb(RTC_DATA_PORT, prev | (1 << RTC_TICK_ENABLE_SHIFT));

    /* Set the rate to 1024 times a second. Register A's other bits keep
       the clock running, so they stay as they are. */
    outportb(RTC_SELECT_PORT, RTC_FREQUENCY_REGISTER);
    prev = inportb(RTC_DATA_PORT);
    outportb(RTC_SELECT_PORT, RTC_FREQUENCY_REGISTER);
    outportb(RTC_DATA_PORT, (prev & ~RTC_RATE_MASK) | RTC_1024HZ_RATE);

    enable();
}

/* Stops counting ticks and puts the old handler back. Must run before the
   program exits, or the CPU keeps calling rtc_handler after the program's
   memory is freed. */
void set_old_rtc_handler(void) {
    byte prev;

    /* Block interrupts, so none arrives while the handler and the RTC's
       registers are half changed. */
    disable();

    /* Turn the periodic interrupt off, keeping register B's other bits. */
    outportb(RTC_SELECT_PORT, RTC_CONTROL_REGISTER);
    prev = inportb(RTC_DATA_PORT);
    outportb(RTC_SELECT_PORT, RTC_CONTROL_REGISTER);
    outportb(RTC_DATA_PORT, prev & ~(1 << RTC_TICK_ENABLE_SHIFT));

    setvect(RTC_VECTOR, old_rtc_handler);

    enable();
}

/* Returns the number of ticks counted so far, 1024 per second. */
unsigned long current_tick(void) {
    unsigned long tick;

    /* tick_count is read in two 16-bit halves. Blocking interrupts stops
       a tick from landing between them and mixing old and new. */
    disable();
    tick = tick_count;
    enable();
    return tick;
}
