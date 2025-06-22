#include "cesium_tiles.h"

cesium_tiles::cesium_tiles() : node("Cesium Tiles") {

}

cesium_tiles::~cesium_tiles() {}

bool cesium_tiles::init(cgv::render::context& ctx) { return true; }

void cesium_tiles::init_frame(cgv::render::context& ctx) {}

void cesium_tiles::clear(cgv::render::context& ctx) {}

void cesium_tiles::draw(cgv::render::context& ctx) {}

std::string cesium_tiles::get_type_name() const 
{ 
	return "cesium_tiles"; 
}

void cesium_tiles::stream_help(std::ostream& os)
{
	os << "Cesium Tiles:\a\n"
	   << "";
}

bool cesium_tiles::handle(cgv::gui::event& e) { return false; }

void cesium_tiles::on_set(void* member_ptr) {}

void cesium_tiles::create_gui() {}
