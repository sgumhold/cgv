#pragma once

#include <cgv/render/context.h>


class maptiles;

class maptiles_interfacer
{
private:
	static maptiles* ptr;

public:
	static void set_pointer(maptiles* _ptr) { ptr = _ptr; }
	static maptiles* get_pointer() { return ptr; }
	static void force_draw (cgv::render::context &ctx);
};
