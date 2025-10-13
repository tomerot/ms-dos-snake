#include "platform\input.h"

/* Starts receiving keys. While input runs, keys don't reach DOS or the
   BIOS, so Ctrl+Alt+Del doesn't restart the PC. */
void start_input(void) {
    set_keyboard_handler();
}

/* Stops receiving keys and gives them back to DOS. Must run before the
   program exits, or the CPU keeps calling the key handler after the
   program's memory is freed. */
void stop_input(void) {
    set_old_keyboard_handler();
}

/* Returns the latest key press not read yet, or NO_KEY if there is none.
   If several presses came in since the last call, only the latest is
   kept. */
key read_key_press(void) {
    if (!is_press_available())
        return NO_KEY;
    return read_pressed_scan_code();
}

/* TRUE while the key is down. k must be one of the KEY_ names. */
bool is_key_held(key k) {
    return is_scan_code_held(k);
}
