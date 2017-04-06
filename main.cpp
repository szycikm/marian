#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "drawing.h"
#include "fileops.h"
#include "movement.h"

#ifdef __cplusplus
extern "C"
#endif

void RageQuit(grid_t *level, objects_t *goombas, SDL_Surface *sprites, SDL_Surface *charset, SDL_Surface *screen, SDL_Texture *scrtex, SDL_Renderer *renderer, SDL_Window *window)
{
	level->CleanArray();
	goombas->CleanArray();
	SDL_FreeSurface(sprites);
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char **argv)
{
	int rc;
	SDL_Event event;
	SDL_Surface *screen, *charset, *sprites;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer); // fullscreen
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer); // windowed resizable
	SDL_MaximizeWindow(window);

	if(rc != 0)
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest"); // or "linear" for blurry everything
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Super Turbo Mario 2k16");
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE);

	// load bitmaps
	charset = SDL_LoadBMP("./cs8x8m.bmp");
	if(charset == NULL)
	{
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	SDL_SetColorKey(charset, true, 0x00FF00);

	sprites = SDL_LoadBMP("./sprites.bmp");
	if(sprites == NULL)
	{
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	SDL_SetColorKey(sprites, true, 0x00FF00);

	// SDL loaded -> the fun begins ------------------------------------------------------------------------------------------------------------------------------------------
	bool quit = false, god = false, canFly = true, flying = false;
	int t1, t2, points = 0, coins = 0, world = 1, lives = 3, pointsToAdd = 0;
	float deltat, leftMargin = 0, splashScreen = SPLASH_SCREEN_DELAY, coinTimer = 0;
	object_t player = { 0, 0, 0, 0, 'a', 0, true }, jumpingCoin = { 0, 0, 0, 0, 'q', 0, false }, jumpingBrick = { 0, 0, 0, 0, 'b', 0, false };
	const Uint8* keystates;
	grid_t level;
	objects_t goombas;
	level.initialized = false;
	goombas.initialized = false;

	if (!LoadGrid(screen, &player, world, &level, &goombas))
	{
		RageQuit(&level, &goombas, sprites, charset, screen, scrtex, renderer, window);
		return 1;
	}

	t1 = SDL_GetTicks();

	while(!quit)
	{
		// deltat = seconds elapsed since last update
		t2 = SDL_GetTicks();
		deltat = (t2 - t1) * 0.001;
		t1 = t2;

		if (pointsToAdd > 0)
		{
			SDL_Delay(10);
			points += 50;
			pointsToAdd -= 50;
		}

		if (coins >= 100)
			lives++;
		
		if (splashScreen > 0)
		{
			splashScreen -= deltat;
			// draw black background
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
			char buffor[8];
			sprintf(buffor, "WORLD %d", world);
			DrawString(screen, 104, 100, buffor, charset);
			DrawSprite16(screen, sprites, 'a', true, 100, 115);
			sprintf(buffor, "%c  %d", 0x04, lives);
			DrawString(screen, 130, 120, buffor, charset);
		}
		else
		{
			splashScreen = 0;
			level.time -= deltat;
			coinTimer += deltat;
			player.animationTimer += deltat;
			if (jumpingCoin.active) jumpingCoin.animationTimer += deltat;

			// if player is alive (active)
			if (player.active)
			{
				CalculatePlayerMovement(&level, deltat, &player, &leftMargin, god);

				// check if player must die
				if ((player.y > (LEVEL_HEIGHT - 1) * 16 || level.time <= 0) && !god)
				{
					// fly, fly, PIZZA DIE!
					lives--;
					player.vx = 0;
					player.vy = -PLAYER_JUMP_INITIAL_SPEED * 2;
					player.active = false;
					player.sprite = 'k';
				}

				// check if player won
				if (PlayerReachedFlag(&level, &player))
				{
					pointsToAdd = PointsForRedFlag(&level, &player) + (int)level.time * 50;
					if (!LoadGrid(screen, &player, ++world, &level, &goombas))
					{
						// no more levels -> quit game
						RageQuit(&level, &goombas, sprites, charset, screen, scrtex, renderer, window);
						return 2;
					}
					splashScreen = SPLASH_SCREEN_DELAY;
					canFly = true;
					player.vx = 0;
					player.vy = 0;
					flying = false;
					leftMargin = 0;
				}

				// check if player collected any coins
				if (PlayerCollectedCoin(&level, &player)) coins++;

				// check if player hit box from below
				if (player.vy < 0)
				{
					// question box hit
					if (PlayerHitFromBelow(&level, &player, 'e', 'c') || PlayerHitFromBelow(&level, &player, 'f', 'c') || PlayerHitFromBelow(&level, &player, 'g', 'c'))
					{
						pointsToAdd = 200;
						coins++;
						JumpObject(&player, &jumpingCoin, JUMPING_COIN_INITIAL_SPEED);
						jumpingBrick.sprite = 'e';
						JumpObject(&player, &jumpingBrick, JUMPING_BRICK_INITIAL_SPEED);
					}

					// brick box hit
					if (PlayerHitFromBelow(&level, &player, 'b', '0'))
					{
						// make sure the player will fall down after hitting the box
						player.vy = 0;
						jumpingBrick.sprite = 'b';
						JumpObject(&player, &jumpingBrick, JUMPING_BRICK_INITIAL_SPEED);
					}

					// used question box hit -> don't replace anyghing
					if (PlayerHitFromBelow(&level, &player, 'c', 'c'))
					{
						// make sure the player will fall down after hitting the box
						player.vy = 0;
						jumpingBrick.sprite = 'c';
						JumpObject(&player, &jumpingBrick, JUMPING_BRICK_INITIAL_SPEED);
					}
				}
			}
			else
			{
				// player has died -> display falling animation
				player.vy += deltat * GRAVITY_SPEED;
				player.y += player.vy * deltat;
				if (player.y > (LEVEL_HEIGHT) * 16)
				{
					// animation has ended (player has fallen to screen bottom)

					if (lives <= 0)
					{
						// you snooze, you looze
						RageQuit(&level, &goombas, sprites, charset, screen, scrtex, renderer, window);
						return 0;
					}
					player.sprite = 'a';
					player.vy = 0;
					canFly = true;
					flying = false;
					leftMargin = 0;
					if (!LoadGrid(screen, &player, world, &level, &goombas))
					{
						RageQuit(&level, &goombas, sprites, charset, screen, scrtex, renderer, window);
						return 1;
					}
					player.active = true;
					splashScreen = SPLASH_SCREEN_DELAY;
				}
			}

			// draw blue background
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x5C, 0x94, 0xFC));

			// draw level
			for (int j = 0; j < level.h; j++)
			{
				for (int i = (player.x / 16) - 5 > 0 ? (player.x / 16) - 5 : 0; i < ((player.x / 16) + 16 < level.w ? (player.x / 16) + 16 : level.w); i++)
				{
					// animate coins
					char obj = level.grid[i][j];
					if (obj == 'e')
					{
						if (coinTimer < 0.8) obj = 'e';
						else if (coinTimer < 0.9) obj = 'f';
						else if (coinTimer < 1) obj = 'g';
						else if (coinTimer >= 1) coinTimer = 0;
					}
					else if (obj == 'R')
					{
						if (coinTimer < 0.8) obj = 'R';
						else if (coinTimer < 0.9) obj = 'S';
						else if (coinTimer < 1) obj = 'T';
						else if (coinTimer >= 1) coinTimer = 0;
					}
					else if (obj == 'c' && jumpingBrick.active && (int)jumpingBrick.x / 16 == i && (int)jumpingBrick.y / 16 + 1 == j) obj = '0'; // there's already a jumping block in this location!
					DrawSprite16(screen, sprites, obj, false, i * 16 - (int)leftMargin, j * 16);
				}
			}

			// draw jumping coin
			if (jumpingCoin.active)
			{
				if (jumpingCoin.vy >= JUMPING_COIN_INITIAL_SPEED)
				{
					// thanks physics! It's so simple! v_e = -v_s !
					jumpingCoin.vy = 0;
					jumpingCoin.active = false;
				}
				else
				{
					if (jumpingCoin.animationTimer < 0.05) jumpingCoin.sprite = 'o';
					else if (jumpingCoin.animationTimer < 0.0) jumpingCoin.sprite = 'p';
					else if (jumpingCoin.animationTimer < 0.15) jumpingCoin.sprite = 'q';
					else if (jumpingCoin.animationTimer < 0.2) jumpingCoin.sprite = 'r';
					else if (jumpingCoin.animationTimer >= 0.2) jumpingCoin.animationTimer = 0;

					// basic gravity calculations
					jumpingCoin.vy += deltat * GRAVITY_SPEED;
					jumpingCoin.y += jumpingCoin.vy * deltat;

					DrawSprite16(screen, sprites, jumpingCoin.sprite, true, jumpingCoin.x - leftMargin, jumpingCoin.y);
				}
			}

			// draw jumping brick
			if (jumpingBrick.active)
			{
				if (jumpingBrick.vy >= JUMPING_BRICK_INITIAL_SPEED)
				{
					// thanks physics! It's so simple! v_e = -v_s !
					jumpingBrick.vy = 0;
					jumpingBrick.active = false;
				}
				else
				{
					// basic gravity calculations
					jumpingBrick.vy += deltat * GRAVITY_SPEED;
					jumpingBrick.y += jumpingBrick.vy * deltat;

					DrawSprite16(screen, sprites, jumpingBrick.sprite, false, jumpingBrick.x - leftMargin, jumpingBrick.y);
				}
			}

			// draw player
			if (player.vy == 0)
			{
				if (player.vx != 0)
				{
					// player is walking
					if (player.animationTimer < 0.1) player.sprite = player.vx > 0 ? 'b' : 'g';
					else if (player.animationTimer < 0.2) player.sprite = player.vx > 0 ? 'c' : 'h';
					else if (player.animationTimer < 0.3) player.sprite = player.vx > 0 ? 'd' : 'i';
					else if (player.animationTimer < 0.4) player.sprite = player.vx > 0 ? 'c' : 'h';
					else if (player.animationTimer >= 0.4) player.animationTimer = 0;
				}
				else
				{
					// player is standing
					if (player.sprite >= 'a' && player.sprite <= 'e') player.sprite = 'a';
					else if (player.sprite >= 'f' && player.sprite <= 'j') player.sprite = 'f';
				}
			}
			else
			{
				// player is flying
				if (player.sprite >= 'a' && player.sprite <= 'e') player.sprite = 'e';
				else if (player.sprite >= 'f' && player.sprite <= 'j') player.sprite = 'j';
			}
			DrawSprite16(screen, sprites, player.sprite, true, player.x - leftMargin, player.y + 1); // 1 pixel lower for next-gen walking effect

			// move and draw goombas, also check if any died
			if (goombas.cnt > 0)
			{
				for (int i = 0; i < goombas.cnt; i++)
				{
					if (goombas.obj[i].active && goombas.obj[i].x > leftMargin - 32 && goombas.obj[i].x < leftMargin + 16*16) // needs to be 32 for despawn detection
					{
						// goomba is alive and in player's view

						goombas.obj[i].animationTimer += deltat;

						// move it, detect collisions, change vx if needed
						CalculateGoombaMovement(&level, deltat, &goombas.obj[i], leftMargin);

						// check if player touched it from above
						if (goombas.obj[i].sprite != 'n' && goombas.obj[i].y - player.y > 10 && goombas.obj[i].y - player.y < 16 && ((player.x - goombas.obj[i].x > 0 && player.x - goombas.obj[i].x < BLOCK_HIT_MARGIN * 2) || (player.x - goombas.obj[i].x < 0 && player.x - goombas.obj[i].x > -BLOCK_HIT_MARGIN * 2)))
						{
							goombas.obj[i].sprite = 'n';
							goombas.obj[i].animationTimer = 0;
							goombas.obj[i].vx = 0;
							pointsToAdd = 100;
							player.vy = -PLAYER_JUMP_INITIAL_SPEED;
						}
						else if (player.active && goombas.obj[i].sprite != 'n' && ((player.x - goombas.obj[i].x > 0 && player.x - goombas.obj[i].x <= 16) || (player.x - goombas.obj[i].x < 0 && player.x - goombas.obj[i].x >= -16)) && ((player.y - goombas.obj[i].y > 0 && player.y - goombas.obj[i].y <= 16) || (player.y - goombas.obj[i].y < 0 && player.y - goombas.obj[i].y >= -16)))
						{
							// player diededed
							lives--;
							player.vx = 0;
							player.vy = -PLAYER_JUMP_INITIAL_SPEED * 2;
							player.active = false;
							player.sprite = 'k';
						}
						
						if (goombas.obj[i].sprite == 'n')
						{
							if (goombas.obj[i].animationTimer > 0.5)
								goombas.obj[i].active = false;
						}
						else if (goombas.obj[i].animationTimer < 0.25) goombas.obj[i].sprite = 'l';
						else if (goombas.obj[i].animationTimer < 0.5) goombas.obj[i].sprite = 'm';
						else if (goombas.obj[i].animationTimer >= 0.5) goombas.obj[i].animationTimer = 0;

						DrawSprite16(screen, sprites, goombas.obj[i].sprite, true, goombas.obj[i].x - leftMargin, goombas.obj[i].y + 1); // 1 pixel lower for next-gen walking effect
					}
				}
			}

			// handling movement and other stuff
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.sym)
					{
					case SDLK_ESCAPE:
						quit = true;
						break;
					case SDLK_g:
						god = !god;
						break;
					case SDLK_n:
						// reset everything
						canFly = true;
						points = 0;
						coins = 0;
						lives = 3;
						world = 1;
						player.vx = 0;
						player.vy = 0;
						flying = false;
						leftMargin = 0;
						if (!LoadGrid(screen, &player, world, &level, &goombas))
						{
							RageQuit(&level, &goombas, sprites, charset, screen, scrtex, renderer, window);
							return 1;
						}
						break;
					case SDLK_s:
						SaveProgress(&level, &player, &goombas, &world, &points, &coins, &lives, &pointsToAdd, &leftMargin, &flying, &canFly);
						break;
					case SDLK_l:
						LoadProgress(&level, &player, &goombas, &world, &points, &coins, &lives, &pointsToAdd, &leftMargin, &flying, &canFly);
						break;
					}
				}
				else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_UP && InTheAirTonight(&level, &player))
					canFly = false;
				else if (event.type == SDL_QUIT)
					quit = true;
			}

			keystates = SDL_GetKeyboardState(NULL);
			if (!god)
			{
				if (keystates[SDL_SCANCODE_LEFT] && player.vx > -PLAYER_SPEED_X_MAX)
					player.vx -= PLAYER_SPEED_X_DELTA * deltat;
				else if (keystates[SDL_SCANCODE_RIGHT] && player.vx < PLAYER_SPEED_X_MAX)
					player.vx += PLAYER_SPEED_X_DELTA * deltat;

				if (canFly && keystates[SDL_SCANCODE_UP])
				{
					if (!flying)
					{
						player.vy = -PLAYER_JUMP_INITIAL_SPEED;
						flying = true;
					}
					else
						player.vy -= PLAYER_SPEED_Y_DELTA * deltat;
				}
				else if (!InTheAirTonight(&level, &player))
				{
					flying = false;
					canFly = true;
				}
			}
			else
			{
				player.vy = 0;
				if (keystates[SDL_SCANCODE_LEFT])
					player.vx -= PLAYER_SPEED_X_DELTA * deltat;
				else if (keystates[SDL_SCANCODE_RIGHT])
					player.vx += PLAYER_SPEED_X_DELTA * deltat;

				if (keystates[SDL_SCANCODE_UP])
					player.y -= PLAYER_SPEED_Y_DELTA * deltat;
				else if (keystates[SDL_SCANCODE_DOWN])
					player.y += PLAYER_SPEED_Y_DELTA * deltat;
			}

			// loose speed if no keys pressed
			if (!keystates[SDL_SCANCODE_LEFT] && !keystates[SDL_SCANCODE_RIGHT] && player.vx != 0)
			{
				player.vx += (player.vx > 0 ? -PLAYER_SPEED_X_DELTA * deltat : PLAYER_SPEED_X_DELTA * deltat);
				// make sure speed IS zero and not ABOUT zero
				if ((player.vx > 0 && player.vx < PLAYER_SPEED_X_DELTA * deltat * 2) || (player.vx < 0 && player.vx > -PLAYER_SPEED_X_DELTA * deltat * 2))
					player.vx = 0;
			}
		}

		// draw HUD
		char buffor[7];
		DrawString(screen, 24, 16, "MARIO", charset);
		DrawString(screen, 144, 16, "WORLD", charset);
		DrawString(screen, 202, 16, "TIME", charset);
		sprintf(buffor, "%.6d", points);
		DrawString(screen, 24, 25, buffor, charset);
		
		char coin = 0x01;
		if (coinTimer < 0.8) coin = 0x01;
		else if (coinTimer < 0.9) coin = 0x02;
		else if (coinTimer < 1) coin = 0x03;
		else if (coinTimer >= 1) coinTimer = 0;

		sprintf(buffor, "%c%c%.2d", coin, 0x04, coins);
		DrawString(screen, 88, 25, buffor, charset);
		sprintf(buffor, "%d", world);
		DrawString(screen, 160, 25, buffor, charset);
		if (splashScreen == 0)
		{
			sprintf(buffor, "%03.0f", level.time);
			DrawString(screen, 210, 25, buffor, charset);
		}
		if (god) DrawString(screen, 24, 80, "GOD IS GOD.", charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer); // what's this?
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	// game ended successfully
	RageQuit(&level, &goombas, sprites, charset, screen, scrtex, renderer, window);
	return 0;
}