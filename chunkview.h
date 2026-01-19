#pragma once

#include "chunky.h"

#include <string>
#include <string_view>
#include <unordered_map>

// Coordinate systems:
// * World coordinates are relative to the space of possible chunks defined by
//   chunkconfig when chunkview is created.
// * View coordinates are local to the view defined when chunkview is created,
//   typically what the user would see on screen.

struct coords
{
	int x;
	int y;

	bool operator<(const coords& other) const { if (x != other.x) { return x < other.x; } return y < other.y; }
	bool operator==(const coords& other) const { return x == other.x && y == other.y; }
};
template<> struct std::hash<coords>
{
	size_t operator()(const coords& cc) const
	{
		return std::hash<int>()(cc.x) ^ (std::hash<int>()(cc.y) << 1);
	}
};

/// A chunkview is a matrix collection of chunks giving you a movable window
/// into the collection, usable for moving around in a world described by it
/// without having to load all of it into memory at once.
struct chunkview
{
	/// Create a chunk view of the given size in tiles you want to observe. We will load
	/// enough chunks to cover the view.
	chunkview(const chunkconfig& c, int width, int height);

	/// Set our current position in world coordinates, updating the view by generating
	/// new chunks if necessary.
	void change_position(int x, int y);

	/// Get a row of data from the view. The row is in view coordinates. The returned
	/// string is the length of the defined view. `row` must not be larged than the
	/// size of the defined view.
	std::string_view line(int row) const;

	/// Get the total row count
	int view_width() const { return _width; };

	/// Get the total row count
	int view_height() const { return _height; };

	/// A bunch of assertions to verify that our internal state is still good.
	void self_test() const;

	tile_type get_tile(int world_x, int world_y) const;
	void set_tile(int world_x, int world_y, tile_type t);

private:
	const chunk* get_chunk_at(int world_x, int world_y) const;
	chunk* get_chunk_at(int world_x, int world_y);

	/// Chunk data
	std::unordered_map<coords, chunk> chunks;

	/// The view data, copied from each chunk.
	std::string data;

	int _width = -1;
	int _height = -1;
	int _current_x = -1;
	int _current_y = -1;
	int _chunk_width = -1;
	int _chunk_height = -1;

	chunkconfig _config;
};
