#include "MCUFRIEND_kbv.h"
MCUFRIEND_kbv tft;

#include "space_schnabli_100.c"

#define BLACK 0x0000
#define WHITE 0xffff

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

/* entity_t */

void entity_t_randomize_angles(entity_t *e)
{
	e->dx = random(200, 500) / 100.0;
	e->dy = random(10, 100) / 100.0;
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
	tft.fillRect(e->x + e->width, e->y, ceil(e->dx), e->height, BLACK);
	tft.fillRect(e->x, e->y + e->height, e->width, ceil(e->dy), BLACK);
}

void entity_t_move(entity_t *e)
{
	e->x -= e->dx;
	e->y -= e->dy;
	if (e->x < -e->width)
	{
		e->x = WIDTH;
		entity_t_randomize_angles(e);
	}
	if (e->y < -e->height)
	{
		e->y = HEIGHT;
		entity_t_randomize_angles(e);
	}
}

void entity_t_draw(entity_t *e)
{
	// Do not draw pixels that would be outside the displays left side.
	if (e->x < 0)
	{
		for (int y = max(-e->y, 0); y < min(e->height, HEIGHT - e->y); y++)
		{
			uint16_t i = y * e->width - e->x;
			tft.setAddrWindow(0, e->y + y, e->width + e->x, 1);
			tft.pushColors(&e->sprite[i * 2], e->width + e->x, 1, false);
		}
		return;
	}

	// Do not draw pixels that would be outside the displays right side.
	if (e->x + e->width > WIDTH)
	{
		for (int y = max(-e->y, 0); y < min(e->height, HEIGHT - e->y); y++)
		{
			uint16_t i = y * e->width;
			tft.setAddrWindow(e->x, e->y + y, WIDTH - e->x, 1);
			tft.pushColors(&e->sprite[i * 2], WIDTH - e->x, 1, false);
		}
		return;
	}

	// The display is able to push colors outside its top but not bottom. So we
	// make sure to only draw as much pixels as necessary if we're on the
	// displays bottom.
	uint16_t height = min(e->height, HEIGHT - e->y);
	tft.setAddrWindow(e->x, e->y, e->x + e->width - 1, e->y + e->height - 1);
	tft.pushColors(e->sprite, e->width * height, 1, false);
}

/* entity_t schnabli */

// Generate a new schnabli. It starts outside the displa and moves towards the
// upper left corner where it wraps around.
entity_t entity_t_new_schnabli(int y)
{
	entity_t s = entity_t{
		x : float(WIDTH),
		y : float(y),
		dx : 0.0,
		dy : 0.0,
		width : 100,
		height : 41,
		sprite : (uint8_t *)space_schnabli_100,
	};
	entity_t_randomize_angles(&s);
	return s;
}

/* main */

entity_t schnablis[3];
uint8_t schnablis_num = sizeof(schnablis) / sizeof(*schnablis);

void setup()
{
	randomSeed(analogRead(0));

	tft.begin(tft.readID());
	tft.setRotation(3);
	tft.fillScreen(BLACK);

	for (int i = 0; i < schnablis_num; i++)
	{
		uint16_t y = HEIGHT / schnablis_num * i;
		y += random(0, HEIGHT / schnablis_num / 2);
		schnablis[i] = entity_t_new_schnabli(y);
	}
}

void loop()
{
	for (int i = 0; i < schnablis_num; i++)
	{
		entity_t_update(&schnablis[i]);
	}
}
