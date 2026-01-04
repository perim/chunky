#include "chunky.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <ncurses.h>

static char me = '@';
static int width = 64;
static int height = 32;
static int level_width = 4;
static int level_height = 4;
static int chunk_xpos = 1;
static int chunk_ypos = 1;
static int x = 10;
static int y = 10;

static void init_colors_once()
{
	static bool ready = false;
	if (ready) return;
	ready = true;
	if (!has_colors()) return;
	start_color();
	use_default_colors();
	init_pair(1, COLOR_YELLOW, -1);
	init_pair(2, COLOR_CYAN, -1);
	init_pair(3, COLOR_RED, -1);
	init_pair(4, COLOR_BLUE, -1);
	init_pair(5, COLOR_GREEN, -1);
	init_pair(6, COLOR_WHITE, -1);
}

static chtype tile_glyph(uint8_t t)
{
	switch (t)
	{
	case TILE_ROCK: return ' ';
	case TILE_WALL: return '#';
	case TILE_DOOR_CLOSED: return '+';
	case TILE_DOOR_OPEN: return '\'';
	case TILE_ONE_WAY_TOP: return '^';
	case TILE_ONE_WAY_BOTTOM: return 'v';
	case TILE_ONE_WAY_RIGHT: return '>';
	case TILE_ONE_WAY_LEFT: return '<';
	case TILE_EMPTY: return '.';
	case TILE_WALL_DAMAGED: return '#';
	case TILE_DEBRIS: return '*';
	case TILE_RAIL: return '.';
	case TILE_SENTINEL: return 'S';
	case TILE_TURRET: return 'U';
	case TILE_TOTEM: return 'I';
	case TILE_TRAP: return '~';
	case TILE_ALTAR: return 'A';
	case TILE_SHRINE: return 'R';
	case TILE_HIDDEN_GROVE: return 'G';
	case TILE_SHRUB: return 't';
	case ENTITY_BOSS: return 'B';
	case ENTITY_LEADER: return 'L';
	case ENTITY_SUPPORT: return 's';
	case ENTITY_TANK: return 'T';
	case ENTITY_DAMAGE: return 'd';
	case ENTITY_SPECIALIST: return 'P';
	case ENTITY_WILD: return 'w';
	default: assert(false); break;
	}
	return '?';
}

static short tile_color_pair(uint8_t t)
{
	switch (t)
	{
	case TILE_DOOR_CLOSED:
	case TILE_DOOR_OPEN:
	case TILE_ONE_WAY_TOP:
	case TILE_ONE_WAY_BOTTOM:
	case TILE_ONE_WAY_RIGHT:
	case TILE_ONE_WAY_LEFT:
		return 2;
	case TILE_ALTAR:
	case TILE_SHRINE:
	case TILE_HIDDEN_GROVE:
	case TILE_RAIL:
		return 4;
	case TILE_SHRUB:
		return 5;
	case ENTITY_BOSS:
	case ENTITY_LEADER:
	case ENTITY_SUPPORT:
	case ENTITY_TANK:
	case ENTITY_DAMAGE:
	case ENTITY_SPECIALIST:
	case ENTITY_WILD:
		return 3;
	default:
		return 0;
	}
}

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
	chunk_filter_one_way_doors(c, s.roll(0, 4));
	c.beautify();
	// Start inside the first room. Pretty random.
	room r = c.rooms.at(0);
	x = r.x1;
	y = r.y1;
}

static bool try_move(chunk& c, int x, int y)
{
	if (x < 0 || y < 0 || x >= c.width || y >= c.height) return false;
	uint8_t tile = c.at(x, y);
	if (tile == TILE_EMPTY) return true;
	if (tile == TILE_DOOR_OPEN) return true;
	if (tile == TILE_DOOR_CLOSED)
	{
		c.build(x, y, TILE_DOOR_OPEN);
		restore(c, y, x);
		return false;
	}
	// TBD handle one-way doors here
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
		else if (ch == KEY_LEFT && try_move(c, x - 1, y))
		{
			restore(c, y, x);
			x--;
			place_me(c, y, x);
		}
		else if (ch == KEY_RIGHT && try_move(c, x + 1, y))
		{
			restore(c, y, x);
			x++;
			place_me(c, y, x);
		}
		else if (ch == KEY_UP && y > 0 && try_move(c, x, y - 1))
		{
			restore(c, y, x);
			y--;
			place_me(c, y, x);
		}
		else if (ch == KEY_DOWN && try_move(c, x, y + 1))
		{
			restore(c, y, x);
			y++;
			place_me(c, y, x);
		}
	}
	endwin();
	return 0;
}
