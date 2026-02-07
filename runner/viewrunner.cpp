#include "chunkview.h"
#include "chunky.h"
#include "runner_utils.h"

#include <assert.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int x = 10;
static int y = 10;


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
	config.level_width = 8;
	config.level_height = 8;
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

		if (ch == KEY_SLEFT)
		{
			while (try_move(v, x, y, x - 1, y))
			{
				x--;
				v.change_position(x, y);
				render_view(v, x, y);
				mvaddch(v.view_height() / 2, v.view_width() / 2, me);
				refresh();
				napms(50);
			}
		}
		else if (ch == KEY_SRIGHT)
		{
			while (try_move(v, x, y, x + 1, y))
			{
				x++;
				v.change_position(x, y);
				render_view(v, x, y);
				mvaddch(v.view_height() / 2, v.view_width() / 2, me);
				refresh();
				napms(50);
			}
		}
		else if (ch == KEY_SR)
		{
			while (try_move(v, x, y, x, y - 1))
			{
				y--;
				v.change_position(x, y);
				render_view(v, x, y);
				mvaddch(v.view_height() / 2, v.view_width() / 2, me);
				refresh();
				napms(50);
			}
		}
		else if (ch == KEY_SF)
		{
			while (try_move(v, x, y, x, y + 1))
			{
				y++;
				v.change_position(x, y);
				render_view(v, x, y);
				mvaddch(v.view_height() / 2, v.view_width() / 2, me);
				refresh();
				napms(50);
			}
		}

		v.change_position(x, y);
		render_view(v, x, y);
	}
	endwin();
	return 0;
}
