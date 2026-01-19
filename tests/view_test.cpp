#include "chunkview.h"
#include <cassert>
#include <iostream>

void print_view(const chunkview& v)
{
	for (int i = 0; i < v.view_height(); ++i)
	{
		std::cout << v.line(i) << std::endl;
	}
}

int main()
{
	seed s(0);
	chunkconfig c(s);
	chunkview v(c, 64, 32);

	v.change_position(0, 0);

	assert(v.view_width() == 64);
	assert(v.view_height() == 32);

	std::string_view line0 = v.line(0);
	assert(!line0.empty());

	// Before moving, let's store a part of the view
	std::string_view old_line15 = v.line(15);
	std::string old_part(old_line15.substr(0, 32));

	v.change_position(32, 16);

	std::string_view new_line15 = v.line(15);
	std::string new_part(new_line15.substr(0, 32));

	// After moving, the view should have changed
	assert(old_part != new_part);

	v.self_test();

	// Optional: visual inspection
	std::cout << "--- Initial View (0,0) ---" << std::endl;
	print_view(v);
	v.change_position(32, 16);
	std::cout << "--- Moved View (32,16) ---" << std::endl;
	print_view(v);

	return 0;
}
