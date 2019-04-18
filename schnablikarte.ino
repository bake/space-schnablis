#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

#include "space_schnabli_100.c"

#define BLACK 0x0000
#define WHITE 0xffff
#define GRAY 0xce59
#define RED 0xe000
#define ORANGE 0xfc60
#define YELLOW 0xff60
#define GREEN 0x0404
#define BLUE 0x027f
#define VIOLET 0x7030

#define WIDTH 480
#define HEIGHT 320

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// Number of frames a spark is alive.
#define SPARK_AGE_MIN 25
#define SPARK_AGE_MAX 35
#define SPARKS_PER_TAP 10

// Frames between spawning fireworks.
#define FIREWORK_CD 5
// Chance that a random firework explodes.
#define FIREWORK_CHANCE 15

struct entity_t
{
	float x, y;
	float dx, dy;
	int width, height;
	uint8_t *sprite;
};

struct star_t
{
	float x, y;
	float dx;
	uint16_t color;
};

struct spark_t
{
	float x, y;
	float dx, dy;
	int8_t age;
	int16_t color;
};

const int XP = 8, XM = A2, YP = A3, YM = 9;
const int TS_LEFT = 97, TS_RT = 949, TS_BOT = 128, TS_TOP = 905;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

entity_t schnablis[3];
uint8_t schnablis_num = sizeof(schnablis) / sizeof(*schnablis);

star_t stars[25];
uint8_t stars_num = sizeof(stars) / sizeof(*stars);

spark_t sparks[64];
uint8_t sparks_num = sizeof(sparks) / sizeof(*sparks);

uint16_t spark_colors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET};
uint8_t spark_colors_num = sizeof(spark_colors) / sizeof(uint16_t);

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

/* stars */

star_t star_t_new()
{
	return star_t{
		x : (float)random(0, WIDTH - 1),
		y : (float)random(0, HEIGHT - 1),
		dx : random(10, 100) / 100.0,
		color : GRAY,
	};
}

void star_t_update(star_t *s)
{
	star_t_clear(s);
	star_t_move(s);
	star_t_draw(s);
}

void star_t_move(star_t *s)
{
	s->x += s->dx;
	if (s->x > WIDTH)
	{
		s->x = 0;
	}
}

void star_t_clear(star_t *s)
{
	tft.drawPixel(s->x, s->y, BLACK);
}

void star_t_draw(star_t *s)
{
	tft.drawPixel(s->x, s->y, s->color);
}

/* sparks */

void spark_t_update(spark_t *s)
{
	spark_t_clear(s);
	if (s->age <= 0)
	{
		return;
	}
	spark_t_move(s);
	spark_t_draw(s);
}

void spark_t_move(spark_t *s)
{
	s->x += s->dx;
	s->y += s->dy;
	s->age--;
	if (WIDTH < s->x || s->x < 0)
	{
		s->age = 0;
	}
	if (HEIGHT < s->y || s->y < 0)
	{
		s->age = 0;
	}
}

void spark_t_clear(spark_t *s)
{
	tft.fillCircle(s->x, s->y, 1, BLACK);
}

void spark_t_draw(spark_t *s)
{
	tft.fillCircle(s->x, s->y, 1, s->color);
}

/* firework */

int firework_cd = 0;

// Spawn same colored sparks. If force is true, a soarks are created regardless
// of the fireworks cooldown.
void firework_new(int x, int y, bool force)
{
	if (!force)
	{
		if (firework_cd > 0)
		{
			return;
		}
		firework_cd = FIREWORK_CD;
	}

	uint8_t sparks_added = 0;
	uint16_t color = spark_colors[random(0, spark_colors_num - 1)];
	for (int i = 0; i < sparks_num; i++)
	{
		if (sparks[i].age > 0)
		{
			continue;
		}
		if (sparks_added >= SPARKS_PER_TAP)
		{
			break;
		}
		sparks_added++;
		sparks[i].x = x;
		sparks[i].y = y;
		sparks[i].dx = random(-200, 200) / 100.0;
		sparks[i].dy = random(-200, 200) / 100.0;
		sparks[i].age = random(SPARK_AGE_MIN, SPARK_AGE_MAX);
		sparks[i].color = color;
	}
}

void firework_update()
{
	firework_cd--;

	if (random(0, 100) < FIREWORK_CHANCE)
	{
		firework_new(random(0, WIDTH), random(0, HEIGHT), true);
	}
}

/* main */

void setup()
{
	randomSeed(analogRead(0));

	tft.begin(tft.readID());
	tft.setRotation(3);
	tft.fillScreen(BLACK);

	for (int i = 0; i < stars_num; i++)
	{
		stars[i] = star_t_new();
	}
	for (int i = 0; i < schnablis_num; i++)
	{
		uint16_t y = HEIGHT / schnablis_num * i;
		y += random(0, HEIGHT / schnablis_num / 2);
		schnablis[i] = entity_t_new_schnabli(y);
	}
}

void loop()
{
	touch();

	for (int i = 0; i < stars_num; i++)
	{
		star_t_update(&stars[i]);
	}
	for (int i = 0; i < schnablis_num; i++)
	{
		entity_t_update(&schnablis[i]);
	}
	for (int i = 0; i < sparks_num; i++)
	{
		spark_t_update(&sparks[i]);
	}
	firework_update();
}

void touch()
{
	TSPoint p = ts.getPoint();
	pinMode(YP, OUTPUT);
	pinMode(XM, OUTPUT);
	digitalWrite(YP, HIGH);
	digitalWrite(XM, HIGH);
	if (MINPRESSURE < p.z && p.z < MAXPRESSURE)
	{
		int x = map(p.y, TS_LEFT, TS_RT, 0, WIDTH);
		int y = map(p.x, TS_BOT, TS_TOP, 0, HEIGHT);
		firework_new(x, y, false);
	}
}
