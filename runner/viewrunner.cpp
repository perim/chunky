#include "chunkview.h"
#include "chunky.h"

#include <assert.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static char me = '@';
static int x = 10;
static int y = 10;

static void init_colors_once()
{
	static bool ready = false;
	if (ready)
		return;
	ready = true;
	if (!has_colors())
		return;
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
	case TILE_ROCK:
		return ' ';
	case TILE_WALL:
		return '#';
	case TILE_DOOR_CLOSED:
		return '+';
	case TILE_DOOR_OPEN:
		return '\'';
	case TILE_ONE_WAY_TOP:
		return '^';
	case TILE_ONE_WAY_BOTTOM:
		return 'v';
	case TILE_ONE_WAY_RIGHT:
		return '>';
	case TILE_ONE_WAY_LEFT:
		return '<';
	case TILE_EMPTY:
		return '.';
	case TILE_WALL_DAMAGED:
		return '#';
	case TILE_DEBRIS:
		return '*';
	case TILE_RAIL:
		return '.';
	case TILE_SENTINEL:
		return 'S';
	case TILE_TURRET:
		return 'U';
	case TILE_TOTEM:
		return 'I';
	case TILE_TRAP:
		return '~';
	case TILE_CHEST:
		return '&';
	case TILE_ALTAR:
		return 'A';
	case TILE_SHRINE:
		return 'R';
	case TILE_HIDDEN_GROVE:
		return 'G';
	case TILE_SHRUB:
		return 't';
	case ENTITY_BOSS:
		return 'B';
	case ENTITY_LEADER:
		return 'L';
	case ENTITY_SUPPORT:
		return 's';
	case ENTITY_TANK:
		return 'T';
	case ENTITY_DAMAGE:
		return 'd';
	case ENTITY_SPECIALIST:
		return 'P';
	case ENTITY_WILD:
		return 'w';
	default:
		assert(false);
		break;
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
	case TILE_CHEST:
		return 1;
	default:
		return 0;
	}
}

static void render_view(const chunkview &v, int player_x, int player_y)
{
	int view_x_start = player_x - v.view_width() / 2;
	int view_y_start = player_y - v.view_height() / 2;

	for (int i = 0; i < v.view_height(); ++i)
	{
		for (int j = 0; j < v.view_width(); ++j)
		{
			tile_type t = v.get_tile(view_x_start + j, view_y_start + i);
			chtype attrs = 0;
			if (has_colors())
			{
				const short pair = tile_color_pair(t);
				if (pair != 0)
					attrs |= COLOR_PAIR(pair);
			}
			mvaddch(i, j, tile_glyph(t) | attrs);
		}
	}
	refresh();
}

static bool can_open_one_way(uint8_t tile, int dx, int dy)
{
	switch (tile)
	{
	case TILE_ONE_WAY_TOP:
		return dx == 0 && dy == -1;
	case TILE_ONE_WAY_BOTTOM:
		return dx == 0 && dy == 1;
	case TILE_ONE_WAY_LEFT:
		return dx == -1 && dy == 0;
	case TILE_ONE_WAY_RIGHT:
		return dx == 1 && dy == 0;
	default:
		return false;
	}
}

static bool try_move(chunkview &v, int from_x, int from_y, int to_x, int to_y)
{
	const int dx = to_x - from_x;
	const int dy = to_y - from_y;
	tile_type tile = v.get_tile(to_x, to_y);
	if (tile == TILE_EMPTY)
		return true;
	if (tile == TILE_DOOR_OPEN)
		return true;
	if (tile == TILE_DOOR_CLOSED)
	{
		v.set_tile(to_x, to_y, TILE_DOOR_OPEN);
		return false;
	}
	if (tile == TILE_ONE_WAY_TOP || tile == TILE_ONE_WAY_BOTTOM ||
	    tile == TILE_ONE_WAY_LEFT || tile == TILE_ONE_WAY_RIGHT)
	{
		if (!can_open_one_way(tile, dx, dy))
			return false;
		v.set_tile(to_x, to_y, TILE_DOOR_OPEN);
		return true;
	}
	return false;
}

int main()
{
	uint64_t value = time(nullptr);
	seed s(value, value);
	chunkconfig config(s);
	config.level_width = 4;
	config.level_height = 4;
	config.chaos = s.roll(0, 4);
	config.openness = s.roll(0, 4);

	initscr();
	clear();
	noecho();
	cbreak();
	set_escdelay(25);
	keypad(stdscr, TRUE);
	curs_set(0);
	init_colors_once();

	int term_width, term_height;
	getmaxyx(stdscr, term_height, term_width);

	chunkview v(config, term_width, term_height);
	v.change_position(x, y);
	render_view(v, x, y);

	int ch = 0;
	while (1)
	{
		mvaddch(v.view_height() / 2, v.view_width() / 2, me);
		refresh();

		ch = getch();
		if (ch == 'q' || ch == 'Q' || ch == 27)
			break;

		int new_x = x;
		int new_y = y;

		if (ch == KEY_LEFT)
			new_x--;
		else if (ch == KEY_RIGHT)
			new_x++;
		else if (ch == KEY_UP)
			new_y--;
		else if (ch == KEY_DOWN)
			new_y++;

		if (try_move(v, x, y, new_x, new_y))
		{
			x = new_x;
			y = new_y;
		}

		v.change_position(x, y);
		render_view(v, x, y);
	}
	endwin();
	return 0;
}
