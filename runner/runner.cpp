#include "chunky.h"
#include "runner_utils.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <ncurses.h>

static int width = 64;
static int height = 32;
static int level_width = 4;
static int level_height = 4;
static int chunk_xpos = 1;
static int chunk_ypos = 1;
static int x = 10;
static int y = 10;


static void render_room(const chunk& c)
{
	for (int y = 0; y < c.height; y++)
	{
		for (int x = 0; x < c.width; x++)
		{
			const int t = c.at(x, y);
			chtype attrs = 0;
			if (has_colors())
			{
				const short pair = tile_color_pair(t);
				if (pair != 0) attrs |= COLOR_PAIR(pair);
			}
			mvaddch(y, x, tile_glyph(t) | attrs);
		}
	}
	refresh();
}

static void restore(const chunk& c, int y, int x)
{
	const uint8_t t = c.at(x, y);
	chtype attrs = 0;
	if (has_colors())
	{
		const short pair = tile_color_pair(t);
		if (pair != 0) attrs |= COLOR_PAIR(pair);
	}
	mvaddch(y, x, tile_glyph(t) | attrs);
}

static void place_me(const chunk& c, int y, int x)
{
	mvaddch(y, x, me);
	refresh();
}

static void generate_room(chunk& c, int method)
{
	seed s = c.config.state;
	c.generate_exits();
	c.self_test();
	bool success = true;
	switch (method)
	{
	case 2: success = chunk_filter_connect_exits_grand_central(c); break;
	case 1: success = chunk_filter_connect_exits_inner_loop(c); break;
	case 0: chunk_filter_connect_exits(c); break; // always works
	default: assert(false); break;
	}
	if (!success)
	{
		printf("Failed generatation with given method!\n");
		exit(-1);
	}

	const int iter = s.roll(2, 8);
	chunk_filter_room_expand(c, iter, iter + 6);
	c.self_test();
	chunk_filter_room_in_room(c);
	c.self_test();
	chunk_filter_one_way_doors(c, s.roll(0, 2));
	chunk_filter_chest(c);
	c.beautify();
	// Start inside the first room. Pretty random.
	room r = c.rooms.at(0);
	x = r.x1;
	y = r.y1;
}

static void load_chunk(chunk& c, chunkconfig& config, int new_chunk_x, int new_chunk_y, int new_x, int new_y)
{
	config.x = new_chunk_x;
	config.y = new_chunk_y;
	c = chunk(config);
	generate_room(c, 0);
	clear();
	render_room(c);
	x = new_x;
	y = new_y;
	place_me(c, y, x);
}

static bool try_switch_chunk(chunk& c, chunkconfig& config, int dx, int dy)
{
	const int new_chunk_x = config.x + dx;
	const int new_chunk_y = config.y + dy;
	if (new_chunk_x < 0 || new_chunk_y < 0 || new_chunk_x >= config.level_width || new_chunk_y >= config.level_height)
	{
		return false;
	}

	int new_x = x;
	int new_y = y;
	if (dx < 0) new_x = c.width - 1;
	else if (dx > 0) new_x = 0;
	if (dy < 0) new_y = c.height - 1;
	else if (dy > 0) new_y = 0;

	load_chunk(c, config, new_chunk_x, new_chunk_y, new_x, new_y);
	return true;
}


static bool try_move(chunk& c, int from_x, int from_y, int to_x, int to_y)
{
	if (to_x < 0 || to_y < 0 || to_x >= c.width || to_y >= c.height) return false;
	const int dx = to_x - from_x;
	const int dy = to_y - from_y;
	uint8_t tile = c.at(to_x, to_y);
	if (tile == TILE_EMPTY) return true;
	if (tile == TILE_DOOR_OPEN) return true;
	if (tile == TILE_DOOR_CLOSED)
	{
		c.build(to_x, to_y, TILE_DOOR_OPEN);
		restore(c, to_y, to_x);
		return false;
	}
	if (tile == TILE_ONE_WAY_TOP || tile == TILE_ONE_WAY_BOTTOM || tile == TILE_ONE_WAY_LEFT || tile == TILE_ONE_WAY_RIGHT)
	{
		if (!can_open_one_way(tile, dx, dy)) return false;
		c.build(to_x, to_y, TILE_DOOR_OPEN);
		restore(c, to_y, to_x);
		return true;
	}
	return false;
}

int main()
{
	uint64_t value = time(nullptr);
	seed s(value, value);
	chunkconfig config(s);
	config.width = width;
	config.height = height;
	config.level_width = level_width;
	config.level_height = level_height;
	config.x = chunk_xpos;
	config.y = chunk_ypos;
	config.chaos = s.roll(0, 4);
	config.openness = s.roll(0, 4);
	chunk c(config);
	generate_room(c, 0);

	initscr();
	clear();
	noecho();
	cbreak();
	set_escdelay(25); // avoid annoying ESC handling delay; we won't use ALT keys anyways
	keypad(stdscr, TRUE);
	curs_set(0);
	clear();
	init_colors_once();
	render_room(c);
	int ch = 0;
	while (1)
	{
		mvaddch(y, x, me);
		ch = getch();
		if (ch == 'q' || ch == 'Q' || ch == 27) break;
		else if (ch == KEY_LEFT && x == 0)
		{
			try_switch_chunk(c, config, -1, 0);
		}
		else if (ch == KEY_LEFT && try_move(c, x, y, x - 1, y))
		{
			restore(c, y, x);
			x--;
			place_me(c, y, x);
		}
		else if (ch == KEY_RIGHT && x == c.width - 1)
		{
			try_switch_chunk(c, config, 1, 0);
		}
		else if (ch == KEY_RIGHT && try_move(c, x, y, x + 1, y))
		{
			restore(c, y, x);
			x++;
			place_me(c, y, x);
		}
		else if (ch == KEY_UP && y == 0)
		{
			try_switch_chunk(c, config, 0, -1);
		}
		else if (ch == KEY_UP && y > 0 && try_move(c, x, y, x, y - 1))
		{
			restore(c, y, x);
			y--;
			place_me(c, y, x);
		}
		else if (ch == KEY_DOWN && y == c.height - 1)
		{
			try_switch_chunk(c, config, 0, 1);
		}
		else if (ch == KEY_DOWN && try_move(c, x, y, x, y + 1))
		{
			restore(c, y, x);
			y++;
			place_me(c, y, x);
		}
		else if (ch == KEY_SLEFT)
		{
			while (try_move(c, x, y, x - 1, y))
			{
				restore(c, y, x);
				x--;
				place_me(c, y, x);
				napms(50);
				refresh();
			}
		}
		else if (ch == KEY_SRIGHT)
		{
			while (try_move(c, x, y, x + 1, y))
			{
				restore(c, y, x);
				x++;
				place_me(c, y, x);
				napms(50);
				refresh();
			}
		}
		else if (ch == KEY_SR)
		{
			while (try_move(c, x, y, x, y - 1))
			{
				restore(c, y, x);
				y--;
				place_me(c, y, x);
				napms(50);
				refresh();
			}
		}
		else if (ch == KEY_SF)
		{
			while (try_move(c, x, y, x, y + 1))
			{
				restore(c, y, x);
				y++;
				place_me(c, y, x);
				napms(50);
				refresh();
			}
		}
	}
	endwin();
	return 0;
}
