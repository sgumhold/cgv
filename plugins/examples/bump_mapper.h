#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/media/illum/textured_surface_material.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/textured_material.h>

class bump_mapper : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	enum SurfacePrimitive { SQUARE, CUBE, SPHERE, TORUS };
	enum TextureSelection { CHECKER, WAVES, ALHAMBRA, CARTUJA };
protected:
	SurfacePrimitive surface_primitive;
	float minor_radius;
	unsigned surface_resolution;
	cgv::render::textured_material material;

	bool use_bump_map;
	bool use_diffuse_map;
	bool wire_frame;
	float bump_scale;

	TextureSelection texture_selection;
	float texture_frequency, texture_frequency_aspect;
	unsigned texture_resolution;

	float texture_u_offset, texture_v_offset;
	float texture_rotation;
	float texture_scale,  texture_aspect;

	int tex_index;
	cgv::render::texture bump_map;
public:
	/// construct from texture resolution
	bump_mapper(unsigned _texture_resolution = 1024);
	///
	void on_set(void* member_ptr);
	/// return type name
	std::string get_type_name() const { return "bump_mapper"; }
	/// add custom texture to material
	bool init(cgv::render::context& ctx);
	/// initialize potentially changed texture
	void init_frame(cgv::render::context& ctx);
	/// draw selected primitive with selected bump map
	void draw(cgv::render::context& ctx);
	/// destruct program and texture
	void clear(cgv::render::context& ctx);
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const { return "example/bump_mapper"; }
	/// gui creation
	void create_gui();
};
