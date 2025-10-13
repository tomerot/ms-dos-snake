#include "platform\sound.h"
#include "platform\clock.h"
#include "hardware\pit.h"
#include "hardware\speaker.h"
#include <stdlib.h>

/* How many tones the slide plays: one if step_hz is 0, otherwise from_hz
   and every step after it that doesn't pass to_hz. */
static int count_steps(const slide *s) {
    return (s->step_hz == 0) ? 1 : abs(s->to_hz - s->from_hz) / s->step_hz + 1;
}

/* Plays each of the slide's tones for step_ms, going down when to_hz is
   below from_hz. */
static void play_slide(const slide *s) {
    int direction = (s->to_hz < s->from_hz) ? -1 : 1;
    int steps = count_steps(s);
    int i;

    for (i = 0; i < steps; i++) {
        set_pit_frequency(s->from_hz + direction * i * s->step_hz);
        wait_ms(s->step_ms);
    }
}

/* Plays the melody and returns when it is over: the program waits for
   it. Needs start_clock first, and every tone must be 19 Hz or more (see
   set_pit_frequency). */
void play_melody(const slide *melody) {
    const slide *s;

    /* A melody with only MELODY_END plays nothing, and leaves the speaker
       off. */
    if (melody->step_ms == 0)
        return;

    /* Set the first tone before switching the speaker on, so it doesn't
       start with the last melody's tone. */
    set_pit_frequency(melody->from_hz);
    enable_sound();
    for (s = melody; s->step_ms != 0; s++)
        play_slide(s);
    disable_sound();
}
