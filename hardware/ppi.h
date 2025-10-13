/* Port 0x61, the system control port. It is named after the Intel 8255
   programmable peripheral interface (PPI) of the IBM PC and XT, whose
   port B it was; the AT dropped the chip and kept only this port, as one
   register on the motherboard.

   In a PC it can:
   - start and stop PIT channel 2 (see pit.c)
   - connect PIT channel 2's output to the speaker
   - switch the memory and expansion-card error checks on and off
   - report those errors, PIT channel 2's output, and a bit that flips
     with each memory refresh

   Its bits belong to unrelated devices, so code that changes the port
   reads it first, changes only its own bits, and writes it back.

   This file names only the two speaker bits, which speaker.c uses. */

#ifndef PPI_H
#define PPI_H

#define PPI_SYSTEM_CONTROL_PORT     0x61  /* system control port B in IBM's manuals */

#define PPI_PIT_CHANNEL2_GATE_SHIFT 0     /* bit 0, timer 2 gate: 1 lets PIT channel 2 count */
#define PPI_SPEAKER_CONNECT_SHIFT   1     /* bit 1, speaker data: 1 sends channel 2 to the speaker */

#endif
