#include "movement.h"

bool IsCollider(char c)
{
	return c >= 'a' && c <= 'o';
}

bool PlayerReachedFlag(grid_t *level, object_t *player)
{
	return level->grid[(int)player->x / 16][(int)player->y / 16] >= 'O' && level->grid[(int)player->x / 16][(int)player->y / 16] <= 'Q';
}

bool PlayerCollectedCoin(grid_t *level, object_t *player)
{
	if (level->grid[(int)player->x / 16][(int)player->y / 16] == 'R')
	{
		level->grid[(int)player->x / 16][(int)player->y / 16] = '0';
		return true;
	}
	else if (level->grid[(int)player->x / 16 + 1][(int)player->y / 16] == 'R')
	{
		level->grid[(int)player->x / 16 + 1][(int)player->y / 16] = '0';
		return true;
	}
	return false;
}

bool PlayerHitFromBelow(grid_t *level, object_t *player, char block, char replacement)
{
	if ((int)player->y % 16 != 0) return false; // player is not hitting any box

	// left side
	if ((int)player->x % 16 < BLOCK_HIT_MARGIN && level->grid[(int)player->x / 16][(int)player->y / 16 - 1] == block)
	{
		level->grid[(int)player->x / 16][(int)player->y / 16 - 1] = replacement;
		return true;
	}

	// right side
	else if ((int)player->x % 16 > 16 - BLOCK_HIT_MARGIN && level->grid[(int)player->x / 16 + 1][(int)player->y / 16 - 1] == block)
	{
		level->grid[(int)player->x / 16 + 1][(int)player->y / 16 - 1] = replacement;
		return true;
	}
	return false;
}

void JumpObject(object_t *player, object_t *obj, float initialSpeed)
{
	int x = 0;
	if ((int)player->x % 16 < BLOCK_HIT_MARGIN)
		x = (int)player->x / 16;
	else if ((int)player->x % 16 > 16 - BLOCK_HIT_MARGIN)
		x = (int)player->x / 16 + 1;

	obj->x = x * 16 + 1;
	obj->y = ((int)player->y / 16 - 1) * 16;
	obj->active = true;
	obj->vy = -initialSpeed;
}

int PointsForRedFlag(grid_t *level, object_t *player)
{
	// "Huzzah! How many points do I receive??"
	int flagBlocks = 0;
	int y = (int)player->y / 16;
	int h = 0;
	while (level->grid[(int)player->x / 16][y] >= 'O' && level->grid[(int)player->x / 16][y] <= 'Q')
	{
		y++;
		h++;
	}
	if (h <= 1) return 100;
	else if (h >= 2 && h <= 4) return 400;
	else if (h >= 4 && h <= 6) return 800;
	else if (h >= 7 && h <= 9) return 2000;
	else if (h >= 10) return 5000;

	return 0;
}

bool InTheAirTonight(grid_t *level, object_t *player)
{
	return !((int)player->y % 16 == 0 && (IsCollider(level->grid[(int)player->x / 16][(int)player->y / 16 + 1]) || (IsCollider(level->grid[(int)player->x / 16 + 1][(int)player->y / 16 + 1]) && (int)player->x % 16 != 0)));
}

void CalculatePlayerMovement(grid_t *level, float deltat, object_t *player, float *leftMargin, bool god)
{
	// move player x
	float playerMovedX = player->vx * deltat;

	if (player->x < 0)
	{
		// don't go left too much
		player->x = 0;
		player->vx = 0;
	}
	if (player->x - *leftMargin < 0)
	{
		// don't go beyond us, you're a late star, time to fool us
		player->x = *leftMargin;
		player->vx = 0;
	}
	else if (player->x > (level->w - 1) * 16) // -1 = don't go beyond grid
	{
		// don't go right too much
		player->x = (level->w - 1) * 16 - 1;
		player->vx = 0;
	}
	else
	{
		// collision detection x for mortals
		if (!god && (int)player->x % 16 == 0)
		{
			// player is aligned to vertical block grid -> check all 4 right/left possible collisions
			if ((player->x > 16 && playerMovedX < 0 && (IsCollider(level->grid[(int)player->x / 16 - 1][(int)player->y / 16]) || (IsCollider(level->grid[(int)player->x / 16 - 1][(int)player->y / 16 + 1]) && (int)player->y % 16 != 0))) || (playerMovedX > 0 && (IsCollider(level->grid[(int)player->x / 16 + 1][(int)player->y / 16]) || (IsCollider(level->grid[(int)player->x / 16 + 1][(int)player->y / 16 + 1]) && (int)player->y % 16 != 0))))
			{
				playerMovedX = 0;
				player->vx = 0;
			}
		}

		player->x += playerMovedX;
	}

	// move screen right (or left, if you're god)
	if (god || (player->x - *leftMargin > 5 * 16 && player->x < (level->w - 1 - 10) * 16))
		*leftMargin += playerMovedX;
	


	// move player y
	player->vy += deltat * GRAVITY_SPEED;
	float playerMovedY = player->vy * deltat;

	if (player->y < 0)
	{
		// don't fly up too much
		player->y = 0;
		player->vy = 0;
	}
	else if (player->y > (LEVEL_HEIGHT - 1) * 16) // -1 = don't go beyond grid (also handled in main loop but not for god -> god can't dye)
	{
		// don't fall down too much
		player->y = (LEVEL_HEIGHT - 1) * 16;
		player->vy = 0;
	}
	else
	{
		// collision detection y for mortals
		if (!god && (int)player->y % 16 == 0)
		{
			// player is aligned to horizontal block grid -> check all 4 up/down possible collisions
			if ((playerMovedY < 0 && (IsCollider(level->grid[(int)player->x / 16][(int)player->y / 16 - 1]) || (IsCollider(level->grid[(int)player->x / 16 + 1][(int)player->y / 16 - 1]) && (int)player->x % 16 != 0))) || (playerMovedY > 0 && (IsCollider(level->grid[(int)player->x / 16][(int)player->y / 16 + 1]) || (IsCollider(level->grid[(int)player->x / 16 + 1][(int)player->y / 16 + 1]) && (int)player->x % 16 != 0))))
			{
				playerMovedY = 0;
				player->vy = 0;
			}
		}

		player->y += playerMovedY;
	}
}

void CalculateGoombaMovement(grid_t *level, float deltat, object_t *goomba, float leftMargin)
{
	// move x
	float goombaMovedX = goomba->vx * deltat;

	if (goomba->x - leftMargin + 16 < 0)
	{
		// goomba went past left edge -> despawn
		goomba->active = false;
	}
	else
	{
		// collision detection x
		if ((int)goomba->x % 16 == 0)
		{
			// goomba is aligned to vertical block grid
			if (goomba->x > 16 && goombaMovedX < 0 && (IsCollider(level->grid[(int)goomba->x / 16 - 1][(int)goomba->y / 16]) || (IsCollider(level->grid[(int)goomba->x / 16 - 1][(int)goomba->y / 16 + 1]) && (int)goomba->y % 16 != 0)))
			{
				goombaMovedX = 0;
				goomba->vx = GOOMBA_SPEED;
			}
			else if (goombaMovedX > 0 && (IsCollider(level->grid[(int)goomba->x / 16 + 1][(int)goomba->y / 16]) || (IsCollider(level->grid[(int)goomba->x / 16 + 1][(int)goomba->y / 16 + 1]) && (int)goomba->y % 16 != 0)))
			{
				goombaMovedX = 0;
				goomba->vx = -GOOMBA_SPEED;
			}
		}

		goomba->x += goombaMovedX;
	}

	// TODO detect if two goombas collided

	// move goomba y
	goomba->vy += deltat * GRAVITY_SPEED;
	float goombaMovedY = goomba->vy * deltat;

	if (goomba->y > LEVEL_HEIGHT * 16)
	{
		// goomba went past bottom edge -> despawn
		goomba->active = false;
	}
	else
	{
		// collision detection y
		if ((int)goomba->y % 16 == 0)
		{
			// goomba is aligned to horizontal block grid -> check all 4 up/down possible collisions
			if ((goombaMovedY < 0 && (IsCollider(level->grid[(int)goomba->x / 16][(int)goomba->y / 16 - 1]) || (IsCollider(level->grid[(int)goomba->x / 16 + 1][(int)goomba->y / 16 - 1]) && (int)goomba->x % 16 != 0))) || (goombaMovedY > 0 && (IsCollider(level->grid[(int)goomba->x / 16][(int)goomba->y / 16 + 1]) || (IsCollider(level->grid[(int)goomba->x / 16 + 1][(int)goomba->y / 16 + 1]) && (int)goomba->x % 16 != 0))))
			{
				goombaMovedY = 0;
				goomba->vy = 0;
			}
		}

		goomba->y += goombaMovedY;
	}
}