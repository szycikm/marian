#pragma once
#include "defines.h"

// Checks weather player is touching flag
bool PlayerReachedFlag(grid_t *level, object_t *player);
// Self explainatory
bool PlayerCollectedCoin(grid_t *level, object_t *player);
// Checks weather player hath hit box while jumping
bool PlayerHitFromBelow(grid_t *level, object_t *player, char block, char replacement);
// Checks all things neccessary, then gives an object a nice push from below so it can jump to the sky!
void JumpObject(object_t *player, object_t *obj, float initialSpeed);
// Calculates points for reaching flag
int PointsForRedFlag(grid_t *level, object_t *player);
// Checks weather player is in air
bool InTheAirTonight(grid_t *level, object_t *player);
// Moves the player
void CalculatePlayerMovement(grid_t *level, float deltat, object_t *player, float *leftMargin, bool god);
// Moves a single goomba
void CalculateGoombaMovement(grid_t *level, float deltat, object_t *goomba, float leftMargin);