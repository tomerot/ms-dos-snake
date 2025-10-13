#include "game\gameplay.h"
#include "platform\video.h"
#include "platform\clock.h"
#include "platform\input.h"

/* Takes over the screen, the clock and the keyboard, runs the game until
   the player quits, then gives all three back to DOS. */
int main(void) {
    start_video();
    start_clock();
    start_input();

    start_game();

    while (is_game_running()) {
        update_game();
    }

    stop_input();
    stop_clock();
    stop_video();

    return 0;
}
