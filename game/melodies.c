#include "game\melodies.h"

/* Played when the snake eats the food: one quick rising slide. */
const slide NEW_LEVEL_MELODY[] = {
    { /*from_hz=*/295, /*to_hz=*/400, /*step_hz=*/15, /*step_ms=*/60 },
    MELODY_END
};

/* Played when the game is over: three falling slides, each starting about
   100 Hz below the one before. The first two end on a short held tone,
   the last on a long low one. */
const slide GAME_OVER_MELODY[] = {
    { /*from_hz=*/500, /*to_hz=*/395, /*step_hz=*/15, /*step_ms=*/60  },
    { /*from_hz=*/395, /*to_hz=*/395, /*step_hz=*/0,  /*step_ms=*/100 },
    { /*from_hz=*/400, /*to_hz=*/295, /*step_hz=*/15, /*step_ms=*/60  },
    { /*from_hz=*/295, /*to_hz=*/295, /*step_hz=*/0,  /*step_ms=*/100 },
    { /*from_hz=*/300, /*to_hz=*/195, /*step_hz=*/15, /*step_ms=*/60  },
    { /*from_hz=*/200, /*to_hz=*/200, /*step_hz=*/0,  /*step_ms=*/400 },
    MELODY_END
};
