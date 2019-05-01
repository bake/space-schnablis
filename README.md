# Space Schnablis

*Space Schnablis* is an Arduino Uno sketch for use with a 3.5" ILI9486 TFT
display. It uses the
[MCUFRIEND_kbv library](https://github.com/prenticedavid/MCUFRIEND_kbv) and
shows two platypodes flying through space with fireworks and stars in the
background. Touching the screen results in even more fireworks.

![Picture](/picture.jpg)

## Fireworks

A group of sparks is called a firework, if the sparks have the same color, were
created at the same time and are moving in different directions. Sparks are
stored in an array which holds 64 elements. Creating a new firework at *(x, y)*
with color `c` results in iterating over the array and modifying up to 10 alive
elements by setting their coordinates to *(x, y)*, their colors to `c`,
assigning random values to `dx` and `dy` and resetting their age. An element is
alive if its `age` is above `0` (`age` might not be the best field name).

The properties `dx` and `dy` are floats that define by how many pixels an entity
moves each frame. `age` is decremented on each frame.

## Clearing leftover pixels

Since clearing the whole screen on every frame would be too slow, only leftover
pixels are redrawn with the background color. This is similar to mrt-prodz
[Flappy Bird clone](https://github.com/mrt-prodz/ATmega328-Flappy-Bird-Clone).
Platypodes, for example, always move towards the top left corner by random
angles, so it is sufficient only to clear two rects: one to the platypuses right
and one below it. The rects have the width or height of the platypuses
horizontal or vertical verlocity.

```c
tft.fillRect(e->x + e->width, e->y, ceil(e->dx), e->height, BLACK);
tft.fillRect(e->x, e->y + e->height, e->width, ceil(e->dy), BLACK);
```

## Drawing only necessary parts

Most elements are one (stars) or five (sparks) pixels small and don't have to
have a special way to be drawn. Sprites on the other hand can overflow the
screen on all four sides which creates glitches on the display. Usually, sprites
are drawn using the libraries `pushColors` function, which writes the values of
an array of colors to the display. If an entity moved outside on the left, right
or bottom, only as many pixels as necessary are drawn (moving outside on the
upper end does not create glitches).

```plain
+----------------+
|            ____|____
|           /....|....\
|          |.....|.....|
|          |.....|.....|
|           \____|____/
|                |
+----------------+
```

For example of this, erm, ball only the left half should be visible. That's why
there are five (its height) individual calls to `pushColors` with six colors
each (now that I think of it, my functions should probably be snake cased).
