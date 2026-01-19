#pragma once

#include "chunky.h"
#include <ncurses.h>

static char me = '@';

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
	case TILE_CHEST: return '&';
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
	case TILE_CHEST:
		return 1;
	default:
		return 0;
	}
}

static bool can_open_one_way(uint8_t tile, int dx, int dy)
{
	switch (tile)
	{
	case TILE_ONE_WAY_TOP: return dx == 0 && dy == -1;
	case TILE_ONE_WAY_BOTTOM: return dx == 0 && dy == 1;
	case TILE_ONE_WAY_LEFT: return dx == -1 && dy == 0;
	case TILE_ONE_WAY_RIGHT: return dx == 1 && dy == 0;
	default: return false;
	}
}
