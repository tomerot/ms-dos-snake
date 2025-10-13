/* Intel 8042 keyboard controller, on the motherboard, and the keyboard
   that talks to it.

   The keyboard doesn't send letters. Each key has a number, its scan
   code. When a key goes down, the controller receives its scan code (a
   press); when it comes up, the same code with bit 7 set (a release). A
   key held down repeats its press, after half a second, about 10 times
   a second.

   In a PC the controller can:
   - translate the codes the keyboard sends into the set the BIOS and DOS
     programs expect (the one in keyboard.h)
   - hold each code at port 0x60 and signal on IRQ 1 (an interrupt
     request line; see pic.c), which the CPU sees as interrupt 0x09
   - send commands to the keyboard, such as turning its lights on and off
     or changing its repeat speed
   - report its state and take its own commands, at port 0x64
   - do two jobs unrelated to keys: restart the CPU, and let it reach
     memory above 1 MB (the A20 line); later ones also take a mouse

   This file uses only the scan codes at port 0x60. It takes over
   interrupt 0x09 from the BIOS, which would otherwise turn them into
   letters for DOS. For each code it records whether its key is held,
   and keeps the latest press until it is read. Some keys, like the
   separate arrow keys, send 0xE0 before their code; with bit 7 set, it
   is taken as the release of a code no key uses, so those keys count as
   their keypad twins. */

#include "hardware\keyboard.h"
#include "hardware\pic.h"
#include <dos.h>

#define KEYBOARD_DATA_PORT     0x60  /* the latest scan code, from the 8042 */
#define KEYBOARD_VECTOR        0x09  /* interrupt number of IRQ 1, the keyboard's line */
#define KEYBOARD_RELEASE_SHIFT 7     /* bit 7 of a scan code: set on release */
#define KEYBOARD_SCAN_CODES    128   /* codes 0-127, without bit 7 */

/* The handler that was there before; set_old_keyboard_handler puts it
   back. */
static void interrupt (*old_keyboard_handler)();

/* The latest press, and whether it is still unread. Only presses are
   kept, so a release can't replace a press before it is read. These and
   scan_code_held are volatile because keyboard_handler changes them at
   any moment. */
static volatile byte last_pressed_scan_code;
static volatile bool has_new_press;

/* Whether each key is down right now, by scan code. */
static volatile bool scan_code_held[KEYBOARD_SCAN_CODES];

/* Called by the CPU on each keyboard interrupt, once per scan code.
   Records whether the key is now held, and keeps the code if it is a
   press. */
static void interrupt keyboard_handler(void) {
    byte code;
    bool released;

    /* Reading the code also frees the controller for the next one. */
    code = inportb(KEYBOARD_DATA_PORT);
    released = (code & (1 << KEYBOARD_RELEASE_SHIFT)) != 0;
    scan_code_held[code & ~(1 << KEYBOARD_RELEASE_SHIFT)] = !released;
    if (!released) {
        last_pressed_scan_code = code;
        has_new_press = TRUE;
    }

    /* Tell the PIC this interrupt is done, so it lets the next one
       through. */
    pic_eoi_master();
}

/* Starts receiving keys: puts keyboard_handler in place of the old
   handler. From then on the BIOS gets no keys: nothing reaches DOS, and
   Ctrl+Alt+Del doesn't restart the PC. */
void set_keyboard_handler(void) {
    /* Block interrupts, so no key arrives while the handler is being
       swapped. */
    disable();
    old_keyboard_handler = getvect(KEYBOARD_VECTOR);
    setvect(KEYBOARD_VECTOR, keyboard_handler);
    enable();
}

/* Puts the old handler back, so the BIOS and DOS get keys again. Must
   run before the program exits, or the CPU keeps calling keyboard_handler
   after the program's memory is freed. */
void set_old_keyboard_handler(void) {
    /* Block interrupts, so no key arrives while the handler is being
       swapped. */
    disable();
    setvect(KEYBOARD_VECTOR, old_keyboard_handler);
    enable();
}

/* TRUE if a press came in that read_pressed_scan_code hasn't taken yet. */
bool is_press_available(void) {
    return has_new_press;
}

/* Returns the latest press and marks it read. If several presses came in
   since the last call, only the latest is kept. Check is_press_available
   first, or it returns the old press again. */
byte read_pressed_scan_code(void) {
    has_new_press = FALSE;
    return last_pressed_scan_code;
}

/* TRUE while the key with this scan code is down. code must be below
   128, without bit 7. */
bool is_scan_code_held(byte code) {
    return scan_code_held[code];
}
