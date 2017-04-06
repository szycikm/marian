#pragma once

extern "C"
{
#ifdef BIT64
#include "./sdl64/include/SDL.h"
#include "./sdl64/include/SDL_main.h"
#else
#include "./sdl/include/SDL.h"
#include "./sdl/include/SDL_main.h"
#endif
}

// pixel values
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define BLOCK_HIT_MARGIN 5

// block values
#define LEVEL_HEIGHT 15

// speed variables
#define PLAYER_SPEED_X_MAX 130
#define PLAYER_SPEED_X_DELTA 200
#define PLAYER_SPEED_Y_DELTA 1000
#define PLAYER_JUMP_INITIAL_SPEED 260
#define JUMPING_COIN_INITIAL_SPEED 400
#define JUMPING_BRICK_INITIAL_SPEED 200
#define GRAVITY_SPEED 1500
#define GOOMBA_SPEED 30

// time variables
#define SPLASH_SCREEN_DELAY 3.0

// The Grid. A digital frontier.
typedef struct
{
	bool initialized;
	int w;
	int h;
	float time;
	char **grid;

	void InitArray()
	{
		if (!initialized)
		{
			grid = new char*[w];
			for (int i = 0; i < w; i++)
				grid[i] = new char[h];

			initialized = true;
		}
	}

	void CleanArray()
	{
		if (initialized)
		{
			for (int i = 0; i < w; i++)
				delete[] grid[i];

			delete[] grid;

			initialized = false;
		}
	}
} grid_t;

typedef struct
{
	float x;
	float y;
	float vx;
	float vy;
	char sprite;
	float animationTimer;
	bool active;
} object_t;

typedef struct
{
	object_t *obj;
	int cnt;
	bool initialized;
	void InitArray()
	{
		if (!initialized)
		{
			obj = new object_t[cnt];
			initialized = true;
		}
	}

	void CleanArray()
	{
		if (initialized)
		{
			delete obj;
			initialized = false;
		}
	}
} objects_t;