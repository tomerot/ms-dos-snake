/* Intel 8254 programmable interval timer (PIT).

   A PIT counter counts down from a number the code gives it, one step for
   each tick of a 1193180 Hz clock, and changes its output wire when it
   reaches zero.

   A PC's PIT has three counters, called channels, each wired to
   something else:
   - channel 0 to IRQ 0 (see pic.c), 18.2 times a second; the BIOS reads
     the time from the RTC (see rtc.c) at boot, then moves it forward on
     each of these interrupts, and DOS reads the time from the BIOS
   - channel 1 to the memory refresh: memory chips forget their contents
     unless refreshed regularly, and channel 1 says when
   - channel 2 to the speaker, through port 0x61 (see ppi.h)

   In a PC each channel can:
   - count down from any number from 1 to 65536, in binary or in decimal
     digits (BCD)
   - shape its output in one of six ways (modes), among them a single
     pulse, a repeating pulse, and a square wave: high for half the count,
     low for the other half, then again
   - let the code read how far it has counted

   This file uses only channel 2, as a square wave, which the speaker
   plays as a tone. The wave repeats 1193180 / number times a second, so
   the number sets the tone's frequency. Port 0x43 takes a command saying
   which channel, how its number will be sent, which mode, and binary or
   BCD; port 0x42 then takes channel 2's number. */

#include "hardware\pit.h"
#include <dos.h>

#define PIT_COMMAND_PORT       0x43     /* the control word register */
#define PIT_CHANNEL2_DATA_PORT 0x42     /* channel 2's number, a byte at a time */

#define PIT_CHANNEL2_SELECT    0x80     /* bits 6-7, SC1-SC0: channel 2 */
#define PIT_LOW_HIGH_ACCESS    0x30     /* bits 4-5, RW1-RW0: low byte, then high */
#define PIT_SQUARE_WAVE_MODE   0x06     /* bits 1-3, M2-M0: mode 3, square wave */
#define PIT_BINARY_FORMAT      0x00     /* bit 0, BCD: 0 counts in binary */

#define PIT_CLOCK_HZ           1193180L /* the clock all channels count */

/* Sets channel 2's square wave to the given frequency, in Hz. hertz must
   be at least 19: below that, the number doesn't fit in the channel's 16
   bits. */
void set_pit_frequency(unsigned int hertz) {
    unsigned int divisor = PIT_CLOCK_HZ / hertz;

    outportb(PIT_COMMAND_PORT, PIT_CHANNEL2_SELECT | PIT_LOW_HIGH_ACCESS |
                               PIT_SQUARE_WAVE_MODE | PIT_BINARY_FORMAT);

    /* The port takes 8 bits, so the 16-bit number goes in two writes, low
       byte first, as the command said. */
    outportb(PIT_CHANNEL2_DATA_PORT, divisor & 0xFF);
    outportb(PIT_CHANNEL2_DATA_PORT, divisor >> 8);
}
