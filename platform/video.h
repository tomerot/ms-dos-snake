#ifndef VIDEO_H
#define VIDEO_H

#include "common.h"
#include "hardware\cga.h"

/* A color number: one of the 16 names in cga.h, BLACK to WHITE. As a
   background, only the first 8 show (BLACK to LIGHT_GRAY). */
typedef byte color;

void start_video(void);
void stop_video(void);

void set_piece(pos p, color fill);
void set_text(pos p, const char *text, color background, color foreground);
void set_big_text(pos p, const char *text, color fill);
int big_text_width(const char *text);

#endif
