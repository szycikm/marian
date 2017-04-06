#pragma once

#include <cstdio>
#include "defines.h"

// Load level from file
bool LoadGrid(SDL_Surface *screen, object_t *player, int gridno, grid_t *level, objects_t *goombas);
// Saves everythin in a neat binary file
void SaveProgress(grid_t *level, object_t *player, objects_t *goombas, int *world, int *points, int *coins, int *lives, int *pointsToAdd, float *leftMargin, bool *flying, bool *canFly);
// Loads everythin
bool LoadProgress(grid_t *level, object_t *player, objects_t *goombas, int *world, int *points, int *coins, int *lives, int *pointsToAdd, float *leftMargin, bool *flying, bool *canFly);