#include "chunkview.h"
#include "chunky.h"

#include <cmath>
#include <iostream>

static char tile_to_char(tile_type t)
{
	switch (t)
	{
	case TILE_ROCK:
		return ' ';
	case TILE_EMPTY:
		return '.';
	case TILE_DOOR_OPEN:
		return 'O';
	case TILE_DOOR_CLOSED:
		return 'D';
	case TILE_ONE_WAY_TOP:
		return '^';
	case TILE_ONE_WAY_BOTTOM:
		return 'v';
	case TILE_ONE_WAY_LEFT:
		return '<';
	case TILE_ONE_WAY_RIGHT:
		return '>';
	case TILE_WALL:
		return '#';
	case TILE_WALL_DAMAGED:
		return '%';
	case TILE_DEBRIS:
		return ':';
	case TILE_RAIL:
		return '=';
	default:
		return '?';
	}
}

chunkview::chunkview(const chunkconfig &c, int width, int height)
    : _config(c), _width(width), _height(height)
{
	data.resize(width * height, ' ');
	// Create a dummy chunk to get the chunk dimensions
	chunk dummy_chunk(_config);
	_chunk_width = dummy_chunk.width;
	_chunk_height = dummy_chunk.height;
}

void chunkview::change_position(int x, int y)
{
	_current_x = x;
	_current_y = y;

	int view_x_start = _current_x - _width / 2;
	int view_y_start = _current_y - _height / 2;

	int chunk_x_start = floor((float)view_x_start / _chunk_width);
	int chunk_y_start = floor((float)view_y_start / _chunk_height);
	int chunk_x_end = floor((float)(view_x_start + _width) / _chunk_width);
	int chunk_y_end = floor((float)(view_y_start + _height) / _chunk_height);

	for (int cy = chunk_y_start; cy <= chunk_y_end; ++cy)
	{
		for (int cx = chunk_x_start; cx <= chunk_x_end; ++cx)
		{
			coords chunk_coords = {cx, cy};
			if (chunks.find(chunk_coords) == chunks.end())
			{
				_config.x = cx;
				_config.y = cy;
				chunk new_chunk(_config);
				new_chunk.generate_exits();
				chunk_filter_connect_exits(new_chunk);
				chunk_filter_room_expand(new_chunk);
				chunks.emplace(chunk_coords, std::move(new_chunk));
			}
		}
	}

	for (int vy = 0; vy < _height; ++vy)
	{
		for (int vx = 0; vx < _width; ++vx)
		{
			int world_x = view_x_start + vx;
			int world_y = view_y_start + vy;

			int chunk_x = floor((float)world_x / _chunk_width);
			int chunk_y = floor((float)world_y / _chunk_height);

			int tile_x = world_x - chunk_x * _chunk_width;
			int tile_y = world_y - chunk_y * _chunk_height;

			coords chunk_coords = {chunk_x, chunk_y};
			const chunk &c = chunks.at(chunk_coords);
			data[vy * _width + vx] =
			    tile_to_char(static_cast<tile_type>(c.at(tile_x, tile_y)));
		}
	}
}

std::string_view chunkview::line(int row) const
{
	return std::string_view(data.data() + row * _width, _width);
}

void chunkview::self_test() const
{
	assert(_width > 0);
	assert(_height > 0);
	assert(data.size() == _width * _height);
}

const chunk *chunkview::get_chunk_at(int world_x, int world_y) const
{
	coords c = {(int)floor((float)world_x / _chunk_width),
	            (int)floor((float)world_y / _chunk_height)};
	auto it = chunks.find(c);
	if (it != chunks.end())
	{
		return &it->second;
	}
	return nullptr;
}

chunk *chunkview::get_chunk_at(int world_x, int world_y)
{
	coords c = {(int)floor((float)world_x / _chunk_width),
	            (int)floor((float)world_y / _chunk_height)};
	auto it = chunks.find(c);
	if (it != chunks.end())
	{
		return &it->second;
	}
	return nullptr;
}

tile_type chunkview::get_tile(int world_x, int world_y) const
{
	const chunk *c = get_chunk_at(world_x, world_y);
	if (!c)
		return TILE_ROCK; // Should not happen if change_position is called
		                  // before

	int chunk_x = floor((float)world_x / _chunk_width);
	int chunk_y = floor((float)world_y / _chunk_height);

	int tile_x = world_x - chunk_x * _chunk_width;
	int tile_y = world_y - chunk_y * _chunk_height;

	return (tile_type)c->at(tile_x, tile_y);
}

void chunkview::set_tile(int world_x, int world_y, tile_type t)
{
	chunk *c = get_chunk_at(world_x, world_y);
	if (c)
	{
		int chunk_x = floor((float)world_x / _chunk_width);
		int chunk_y = floor((float)world_y / _chunk_height);

		int tile_x = world_x - chunk_x * _chunk_width;
		int tile_y = world_y - chunk_y * _chunk_height;
		c->build(tile_x, tile_y, t);
	}
}
