#include "chunky.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <vector>
#include <string>

static int p__debug_level = 0;
static int p__gen_method = 0;
static int width = 64;
static int height = 16;
static int level_width = 4;
static int level_height = 4;
static int xpos = 1;
static int ypos = 1;

static void usage()
{
	printf("chunkgen command line options\n");
	printf("-h/--help              This help\n");
	printf("-d/--debug L           Set debug level [0,1,2,3]\n");
	printf("-s/--seed S            Set random seed value\n");
	printf("-W/--width M           Width of chunk (default %d, must be power-of-two)\n", width);
	printf("-H/--height H          Height of chunk (default %d, must be power-of-two)\n", height);
	printf("-lw/--level-width W    Width of level in number of chunks (default %d)\n", level_width);
	printf("-lh/--level-height H   Height of level in number of chunks (default %d)\n", level_height);
	printf("-x/--level-x-pos X     Level X position of chunk (default %d)\n", xpos);
	printf("-y/--level-y-pos Y     Level Y position of chunks (default %d)\n", ypos);
	exit(-1);
}

static inline bool match(const char* in, const char* short_form, const char* long_form, int& remaining)
{
	if (strcmp(in, short_form) == 0 || strcmp(in, long_form) == 0)
	{
		remaining--;
		return true;
	}
	return false;
}

static int get_int(const char* in, int& remaining)
{
	if (remaining == 0)
	{
		usage();
	}
	remaining--;
	return atoi(in);
}

static std::string get_str(const char* in, int& remaining)
{
	if (remaining == 0)
	{
		usage();
	}
	remaining--;
	return in;
}

static void test_inner_loop(chunkconfig& config)
{
	chunk c(config);
	c.generate_exits();
	bool r = chunk_filter_connect_exits_inner_loop(c);
	if (r)
	{
		c.beautify();
		c.print_chunk();
	}
}

static void test_grand_central(chunkconfig& config)
{
	chunk c(config);
	c.generate_exits();
	bool r = chunk_filter_connect_exits_grand_central(c);
	if (r)
	{
		c.beautify();
		c.print_chunk();
	}
}

static void stress_test(chunkconfig& config, seed s)
{
	chunk c(config);
	c.generate_exits();
	c.self_test();
	chunk_filter_connect_exits(c);
	const int iter = s.roll(2, 8);
	chunk_filter_room_expand(c, iter, iter + 6);
	c.self_test();
	chunk_filter_room_in_room(c);
	c.self_test();
	chunk_filter_one_way_doors(c, s.roll(0, 4));
	c.beautify();
	room r = chunk_filter_boss_placement(c, 0);
	chunk_filter_protect_room(c, r);
	chunk_filter_wildlife(c);
	print_room(c, r.index);
}

int main(int argc, char **argv)
{
	int method = 0;
	int remaining = argc - 1; // zeroth is name of program
	uint64_t value = time(nullptr);
	for (int i = 1; i < argc; i++)
	{
		if (match(argv[i], "-h", "--help", remaining))
		{
			usage();
		}
		else if (match(argv[i], "-d", "--debug", remaining))
		{
			p__debug_level = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-W", "--width", remaining))
		{
			width = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-H", "--height", remaining))
		{
			height = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-lw", "--level-width", remaining))
		{
			level_width = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-lh", "--level-height", remaining))
		{
			level_height = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-x", "--level-x-pos", remaining))
		{
			xpos = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-y", "--level-y-pos", remaining))
		{
			ypos = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-s", "--seed", remaining))
		{
			value = get_int(argv[++i], remaining);
		}
		else if (match(argv[i], "-m", "--method", remaining))
		{
			std::string v = get_str(argv[++i], remaining);
			if (v == "main") method = 0;
			else if (v == "inner") method = 1;
			else if (v == "grand") method = 2;
			else usage();
		}
	}
	if (remaining > 0) usage();
	if (xpos >= level_width || ypos >= level_height)
	{
		printf("Level position must be within level width and height!\n");
		exit(-1);
	}
	printf("Showing room from seed %llu:\n", (unsigned long long)value);

	seed s(value, value);
	chunkconfig config(s);
	config.width = width;
	config.height = height;
	config.level_width = level_width;
	config.level_height = level_height;
	config.x = xpos;
	config.y = ypos;
	config.chaos = s.roll(0, 4);
	config.openness = s.roll(0, 4);

	switch (method)
	{
	case 2: test_grand_central(config); break;
	case 1: test_inner_loop(config); break;
	case 0: stress_test(config, s); break;
	default: assert(false); break;
	}

	return 0;
}
