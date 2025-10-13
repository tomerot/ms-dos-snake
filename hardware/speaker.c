/* The PC speaker: a small loudspeaker inside the case, with no chip and
   no settings of its own. Its cone has only two positions, pushed out or
   resting, set by one wire: PIT channel 2's output (see pit.c), let
   through only while bit 1 of port 0x61 is set (see ppi.h).

   In a PC it can:
   - play a tone: when PIT channel 2 makes a square wave, the cone moves
     out and back at the wave's frequency
   - click: while PIT channel 2's output is held high, each flip of bit 1
     of port 0x61 moves the cone once; old programs made clicks and even
     recorded sounds this way

   This file uses only the tone. It turns it on by letting PIT channel 2
   count (bit 0 of port 0x61) and letting its output through to the
   speaker (bit 1), and off by clearing both. Port 0x61 is shared, so
   both functions read it and change only these two bits. pit.c sets the
   tone's frequency. */

#include "hardware\speaker.h"
#include "hardware\ppi.h"
#include "common.h"
#include <dos.h>

/* Starts the tone. Set the frequency first (pit.c), or the speaker plays
   whatever PIT channel 2 was last set to. */
void enable_sound(void) {
    byte prev = inportb(PPI_SYSTEM_CONTROL_PORT);
    outportb(PPI_SYSTEM_CONTROL_PORT,
             prev | (1 << PPI_PIT_CHANNEL2_GATE_SHIFT) |
                    (1 << PPI_SPEAKER_CONNECT_SHIFT));
}

/* Stops the tone: stops PIT channel 2 and cuts it off from the speaker. */
void disable_sound(void) {
    byte prev = inportb(PPI_SYSTEM_CONTROL_PORT);
    outportb(PPI_SYSTEM_CONTROL_PORT,
             prev & ~((1 << PPI_PIT_CHANNEL2_GATE_SHIFT) |
                      (1 << PPI_SPEAKER_CONNECT_SHIFT)));
}
