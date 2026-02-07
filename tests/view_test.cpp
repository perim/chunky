#include "chunkview.h"
#include <cassert>
#include <iostream>

int main()
{
	seed s(0);
	chunkconfig c(s);
	chunkview v(c, 64, 32);

	v.change_position(0, 0);

	assert(v.view_width() == 64);
	assert(v.view_height() == 32);

	v.change_position(32, 16);

	v.self_test();

	return 0;
}
