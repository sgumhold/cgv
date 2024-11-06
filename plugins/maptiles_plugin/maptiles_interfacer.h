#pragma once

class maptiles;

class maptiles_interfacer
{
public:
	static maptiles* ptr;

public:
	static void set_pointer(maptiles* _ptr) { ptr = _ptr; }
	static maptiles* get_ptr() { return ptr; }
};

maptiles* maptiles_interfacer::ptr = nullptr;