/* Intel 8259A programmable interrupt controller (PIC).

   A device that needs the CPU's attention, like a keyboard with a new key
   press, signals on its own wire to a PIC: its interrupt request line
   (IRQ). The PIC then interrupts the CPU, which stops what it was doing
   and runs the handler for that device.

   A PC has two PICs with 8 IRQs each, numbered 0-15. The master takes
   IRQ 0-7 and signals the CPU; the slave takes IRQ 8-15 and signals
   through the master's IRQ 2.

   In a PC they can:
   - ignore chosen IRQs (the mask)
   - pass requests to the CPU one at a time, the most urgent first: the
     lower the IRQ number, the more urgent, and IRQ 8-15 count as IRQ 2
   - tell the CPU which interrupt number each IRQ is; the BIOS sets
     IRQ 0-7 to interrupts 0x08-0x0F and IRQ 8-15 to 0x70-0x77
   - hold back an IRQ, and every less urgent one, until the code says its
     interrupt was handled: the end of interrupt (EOI)
   - report which requests are waiting and which are being handled

   This file uses only the EOI, written to a PIC's command port: 0x20 on
   the master, 0xA0 on the slave. It is the "non-specific" EOI: it ends
   the most urgent interrupt being handled, which is the one just
   finished. */

#include "hardware\pic.h"
#include <dos.h>

#define PIC_MASTER_COMMAND_PORT 0x20  /* the master PIC, IRQ 0-7 */
#define PIC_SLAVE_COMMAND_PORT  0xA0  /* the slave PIC, IRQ 8-15 */
#define PIC_EOI_COMMAND         0x20  /* OCW2 (operation command word 2): non-specific EOI */

/* Tells the master PIC that the interrupt it passed on was handled. A
   handler for IRQ 0-7 must call it before returning. */
void pic_eoi_master(void) {
    outportb(PIC_MASTER_COMMAND_PORT, PIC_EOI_COMMAND);
}

/* Tells both PICs that an interrupt from the slave was handled. A handler
   for IRQ 8-15 must call it, instead of pic_eoi_master, before returning. */
void pic_eoi_slave(void) {
    outportb(PIC_SLAVE_COMMAND_PORT, PIC_EOI_COMMAND);

    /* The slave's request reached the CPU through the master's IRQ 2, so
       the master is waiting for an EOI too. */
    pic_eoi_master();
}
