#include "chunkview.h"
#include "chunky.h"

#include <algorithm>
#include <cassert>

chunkview::chunkview(const chunkconfig &c, int width, int height)
    : _config(c), _width(width), _height(height)
{
	// Create a dummy chunk to get the chunk dimensions
	chunk dummy_chunk(_config);
	_chunk_width = dummy_chunk.width;
	_chunk_height = dummy_chunk.height;
}

static int floor_div(int value, int divisor)
{
	assert(divisor > 0);
	int q = value / divisor;
	int r = value % divisor;
	if (r != 0 && value < 0)
		--q;
	return q;
}

void chunkview::change_position(int x, int y)
{
	_current_x = x;
	_current_y = y;

	int view_x_start = _current_x - _width / 2;
	int view_y_start = _current_y - _height / 2;
	int view_x_end = view_x_start + _width - 1;
	int view_y_end = view_y_start + _height - 1;

	int chunk_x_start = floor_div(view_x_start, _chunk_width);
	int chunk_y_start = floor_div(view_y_start, _chunk_height);
	int chunk_x_end = floor_div(view_x_end, _chunk_width);
	int chunk_y_end = floor_div(view_y_end, _chunk_height);

	const int max_chunk_x = _config.level_width - 1;
	const int max_chunk_y = _config.level_height - 1;
	int clamped_x_start = std::max(0, chunk_x_start);
	int clamped_y_start = std::max(0, chunk_y_start);
	int clamped_x_end = std::min(max_chunk_x, chunk_x_end);
	int clamped_y_end = std::min(max_chunk_y, chunk_y_end);

	if (clamped_x_start > clamped_x_end || clamped_y_start > clamped_y_end)
	{
		_chunk_x_start = 0;
		_chunk_x_end = -1;
		_chunk_y_start = 0;
		_chunk_y_end = -1;
		return;
	}

	const bool has_bounds = (_chunk_x_end >= _chunk_x_start && _chunk_y_end >= _chunk_y_start);
	if (has_bounds &&
	    clamped_x_start == _chunk_x_start && clamped_x_end == _chunk_x_end &&
	    clamped_y_start == _chunk_y_start && clamped_y_end == _chunk_y_end)
	{
		return;
	}

	for (int cy = clamped_y_start; cy <= clamped_y_end; ++cy)
	{
		for (int cx = clamped_x_start; cx <= clamped_x_end; ++cx)
		{
			if (has_bounds &&
			    cx >= _chunk_x_start && cx <= _chunk_x_end &&
			    cy >= _chunk_y_start && cy <= _chunk_y_end)
			{
				continue;
			}
			coords chunk_coords = {cx, cy};
			if (chunks.find(chunk_coords) == chunks.end())
			{
				chunkconfig fresh_config = _config;
				fresh_config.x = cx;
				fresh_config.y = cy;
				chunk new_chunk(fresh_config);
				new_chunk.generate_exits();
				chunk_filter_connect_exits(new_chunk);
				chunk_filter_room_expand(new_chunk);
				chunks.emplace(chunk_coords, std::move(new_chunk));
			}
		}
	}

	_chunk_x_start = clamped_x_start;
	_chunk_x_end = clamped_x_end;
	_chunk_y_start = clamped_y_start;
	_chunk_y_end = clamped_y_end;
}

void chunkview::self_test() const
{
	assert(_width > 0);
	assert(_height > 0);
}

const chunk *chunkview::get_chunk_at(int world_x, int world_y) const
{
	if (world_x < 0 || world_y < 0)
		return nullptr;
	const int world_width = _config.level_width * _chunk_width;
	const int world_height = _config.level_height * _chunk_height;
	if (world_x >= world_width || world_y >= world_height)
		return nullptr;
	coords c = {floor_div(world_x, _chunk_width),
	            floor_div(world_y, _chunk_height)};
	auto it = chunks.find(c);
	if (it != chunks.end())
	{
		return &it->second;
	}
	return nullptr;
}

chunk *chunkview::get_chunk_at(int world_x, int world_y)
{
	if (world_x < 0 || world_y < 0)
		return nullptr;
	const int world_width = _config.level_width * _chunk_width;
	const int world_height = _config.level_height * _chunk_height;
	if (world_x >= world_width || world_y >= world_height)
		return nullptr;
	coords c = {floor_div(world_x, _chunk_width),
	            floor_div(world_y, _chunk_height)};
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

	int chunk_x = floor_div(world_x, _chunk_width);
	int chunk_y = floor_div(world_y, _chunk_height);

	int tile_x = world_x - chunk_x * _chunk_width;
	int tile_y = world_y - chunk_y * _chunk_height;

	return (tile_type)c->at(tile_x, tile_y);
}

void chunkview::set_tile(int world_x, int world_y, tile_type t)
{
	chunk *c = get_chunk_at(world_x, world_y);
	if (c)
	{
		int chunk_x = floor_div(world_x, _chunk_width);
		int chunk_y = floor_div(world_y, _chunk_height);

		int tile_x = world_x - chunk_x * _chunk_width;
		int tile_y = world_y - chunk_y * _chunk_height;
		c->build(tile_x, tile_y, t);
	}
}
