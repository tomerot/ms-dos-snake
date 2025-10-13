#ifndef SOUND_H
#define SOUND_H

/* One part of a melody: the tone starts at from_hz and moves toward
   to_hz by step_hz, without passing it, holding each tone for step_ms
   milliseconds. With step_hz 0, it is one tone held for step_ms. */
typedef struct {
    int from_hz;
    int to_hz;
    int step_hz;
    int step_ms;
} slide;

/* Ends every melody: the one slide with step_ms 0. */
#define MELODY_END { /*from_hz=*/0, /*to_hz=*/0, /*step_hz=*/0, /*step_ms=*/0 }

void play_melody(const slide *melody);

#endif
