#pragma once

class maptiles;

class maptiles_interfacer
{
private:
	static maptiles* ptr;

public:
	static void set_pointer(maptiles* _ptr) { ptr = _ptr; }
	static maptiles* get_pointer() { return ptr; }
};

maptiles* maptiles_interfacer::ptr = nullptr;