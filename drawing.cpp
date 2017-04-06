#include "drawing.h"

void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset)
{
	int px, py, c, backx = x;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text)
	{
		c = *text & 255;
		if (c == '\n')
		{
			x = backx;
			y += 9;
		}
		else
		{
			px = (c % 16) * 8;
			py = (c / 16) * 8;
			s.x = px;
			s.y = py;
			d.x = x;
			d.y = y;
			SDL_BlitSurface(charset, &s, screen, &d);
			x += 8;
		}
		text++;
	}
}

void DrawSprite16(SDL_Surface *screen, SDL_Surface *sprites, char obj, bool secondRow, int screenx, int screeny)
{
	if (obj == '0') return; // no need to draw empty sprite
		
	int n = 0;
	//if (obj >= '1' && obj <= '9')
	//	n = obj - '1';
	if (obj >= 'a' && obj <= 'z')
		n = obj - 'a';
	else if (obj >= 'A' && obj <= 'Z')
		n = obj - 'A' + 26;
	
	SDL_Rect s = { n * 16, secondRow ? 16 : 0, 16, 16 };
	SDL_Rect d = { screenx, screeny, 16, 16 };
	SDL_BlitSurface(sprites, &s, screen, &d);
}