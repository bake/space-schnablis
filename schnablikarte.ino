#include "MCUFRIEND_kbv.h"
MCUFRIEND_kbv tft;

#include "space_schnabli_100.c"

#define BACKGROUND 0x0000
#define RED 0xF800

#define WIDTH 480
#define HEIGHT 320

struct entity_t
{
	float x, y;
	float dx, dy;
	int width, height;
	uint8_t *sprite;
};

entity_t entity_t_new_schnabli(int y)
{
	int width = 100;
	int height = 41;
	return entity_t{
			.x = float(WIDTH),
			.y = float(y),
			.dx = random(200, 500) / 100.0,
			.dy = random(0, 0) / 100.0,
			.width = width,
			.height = height,
			.sprite = (uint8_t *)space_schnabli_100,
	};
}

void entity_t_update(entity_t *e)
{
	entity_t_move(e);
	entity_t_draw(e);
	entity_t_clear(e);
}

// Clear leftover pixels. The platypus is always flying towards the upper left
// corner, so only the bottom and right side needs to be cleaned.
void entity_t_clear(entity_t *e)
{
	tft.fillRect(e->x + e->width, e->y, ceil(e->dx), e->height, BACKGROUND);
	tft.fillRect(e->x, e->y + e->height, e->width, ceil(e->dy), BACKGROUND);
}

void entity_t_move(entity_t *e)
{
	e->x -= e->dx;
	e->y -= e->dy;
	if (e->x < -e->width)
	{
		e->x = WIDTH;
	}
}

void entity_t_draw(entity_t *e)
{
	tft.fillRect(0, 0, WIDTH, 20, BACKGROUND);
	tft.setCursor(0, 0);

	if (e->x < 0)
	{
		int x_off = -e->x;
		for (int y = 0; y < e->height; y++)
		{
			tft.setAddrWindow(0, e->y + y, e->width - x_off, 1);
			tft.pushColors(&e->sprite[(y * e->width + x_off) * 2], e->width - x_off, 1, false);
		}
		return;
	}

	if (e->x + e->width > WIDTH)
	{
		for (int y = 0; y < e->height; y++)
		{
			tft.setAddrWindow(e->x, e->y + y, WIDTH - e->x, 1);
			tft.pushColors(&e->sprite[(y * e->width) * 2], WIDTH - e->x, 1, false);
		}
		return;
	}

	tft.setAddrWindow(e->x, e->y, e->x + e->width - 1, e->y + e->height - 1);
	tft.pushColors(e->sprite, e->width * e->height, 1, false);
}

entity_t schnabli_1;
entity_t schnabli_2;
entity_t schnabli_3;

void setup()
{
	randomSeed(analogRead(0));

	tft.begin(tft.readID());
	tft.setRotation(3);
	tft.fillScreen(BACKGROUND);

	schnabli_1 = entity_t_new_schnabli(50);
	schnabli_2 = entity_t_new_schnabli(150);
	schnabli_3 = entity_t_new_schnabli(250);
}

void loop()
{
	entity_t_update(&schnabli_1);
	entity_t_update(&schnabli_2);
	entity_t_update(&schnabli_3);
}
