#pragma once

#include "defines.h"

// draw letters on screen, starting in (x, y) charset = 128x128 bmp with chars
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset);

// Draw a block from horizontal file (n = nth sprite)
void DrawSprite16(SDL_Surface *screen, SDL_Surface *sprites, char obj, bool secondRow, int screenx, int screeny);