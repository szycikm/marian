#include "fileops.h"

bool LoadGrid(SDL_Surface *screen, object_t *player, int gridno, grid_t *level, objects_t *goombas)
{
	char filename[16];
	sprintf(filename, "./l%d.txt", gridno);
	FILE *file = fopen(filename, "r");

	if (file == NULL)
		return false;

	int w, goombacnt, time, x, y;
	if (fscanf(file, "%d;%d;%d;%d;%d", &w, &goombacnt, &time, &x, &y) != 5)
	{
		fclose(file);
		return false;
	}
	
	level->CleanArray();
	level->w = w;
	level->h = LEVEL_HEIGHT;
	level->time = time;
	level->InitArray();
	player->x = x;
	player->y = y;

	goombas->CleanArray();
	goombas->cnt = goombacnt;
	goombas->InitArray();

	char obj[2];
	for (int j = 0; j < level->h; j++)
	{
		for (int i = 0; i < level->w; i++)
		{
			if (fscanf(file, "%1s", &obj) != 1)
			{
				// invalid sprite -> fill with blank space
				level->grid[i][j] = '0';
			}
			else if (obj[0] == '8')
			{
				// create new goomba at specified coordinates
				goombas->obj[--goombacnt].active = true;
				goombas->obj[goombacnt].sprite = 'l';
				goombas->obj[goombacnt].vx = -GOOMBA_SPEED;
				goombas->obj[goombacnt].vy = 0;
				goombas->obj[goombacnt].x = i*16;
				goombas->obj[goombacnt].y = j*16;

				// also fill this place with blank space
				level->grid[i][j] = '0';
			}
			else
				level->grid[i][j] = obj[0];
		}
	}

	fclose(file);
	return true;
}

void SaveProgress(grid_t *level, object_t *player, objects_t *goombas, int *world, int *points, int *coins, int *lives, int *pointsToAdd, float *leftMargin, bool *flying, bool *canFly)
{
	FILE *file = fopen("savegame.dat", "wb");
	fwrite(player, sizeof(object_t), 1, file);
	
	fwrite(&level->h, sizeof(int), 1, file);
	fwrite(&level->w, sizeof(int), 1, file);
	fwrite(&level->time, sizeof(float), 1, file);
	for (int i = 0; i < level->w; i++)
		fwrite(level->grid[i], sizeof(char), level->h, file);

	fwrite(&goombas->cnt, sizeof(int), 1, file);
	fwrite(goombas->obj, sizeof(object_t), goombas->cnt, file);

	fwrite(world, sizeof(int), 1, file);
	fwrite(points, sizeof(int), 1, file);
	fwrite(coins, sizeof(int), 1, file);
	fwrite(lives, sizeof(int), 1, file);
	fwrite(pointsToAdd, sizeof(int), 1, file);
	fwrite(leftMargin, sizeof(float), 1, file);
	fwrite(flying, sizeof(bool), 1, file);
	fwrite(canFly, sizeof(bool), 1, file);
	
	fclose(file);
}

bool LoadProgress(grid_t *level, object_t *player, objects_t *goombas, int *world, int *points, int *coins, int *lives, int *pointsToAdd, float *leftMargin,bool *flying, bool *canFly)
{
	FILE *file = fopen("savegame.dat", "rb");
	if (file == NULL) return false;

	fread(player, sizeof(object_t), 1, file);

	level->CleanArray();
	int h, w;
	float time;
	fread(&h, sizeof(int), 1, file);
	fread(&w, sizeof(int), 1, file);
	fread(&time, sizeof(float), 1, file);
	level->h = h;
	level->w = w;
	level->time = time;
	level->InitArray();
	for (int i = 0; i < w; i++)
		fread(level->grid[i], sizeof(char), h, file);
	
	goombas->CleanArray();
	int cnt;
	fread(&cnt, sizeof(int), 1, file);
	goombas->cnt = cnt;
	goombas->InitArray();
	fread(goombas->obj, sizeof(object_t), cnt, file);

	fread(world, sizeof(int), 1, file);
	fread(points, sizeof(int), 1, file);
	fread(coins, sizeof(int), 1, file);
	fread(lives, sizeof(int), 1, file);
	fread(pointsToAdd, sizeof(int), 1, file);
	fread(leftMargin, sizeof(float), 1, file);
	fread(flying, sizeof(bool), 1, file);
	fread(canFly, sizeof(bool), 1, file);

	fclose(file);
	return true;
}