#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <vr_view_interactor.h>
#include <cg_nui/label_manager.h>

/// class manages static and dynamic parts of scene
class vr_scene : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
private:
	// keep reference to vr view (initialized in init function)
	vr_view_interactor* vr_view_ptr;
protected:
	//@name boxes and table
	//@{	

	// store the static part of the scene as colored boxes with the table in the last 5 boxes
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	// ui parameters for table construction
	float table_width, table_depth, table_height, leg_width, leg_offset;
	rgb table_color, leg_color;

	// rendering style for rendering of boxes
	cgv::render::box_render_style style;

	/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
	void construct_table(float tw, float td, float th, float tW, float tO, rgb table_clr, rgb leg_clr);
	/// construct boxes that represent a room of dimensions w,d,h and wall width W
	void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
	/// construct boxes for environment
	void construct_environment(float s, float ew, float ed, float w, float d, float h);
	/// construct a scene with a table
	void build_scene(float w, float d, float h, float W);
	//@}


	//@name labels
	//@{	
	/// use label manager to organize labels in texture
	cgv::nui::label_manager lm;

	/// store label placements for rectangle renderer
	std::vector<vec3> label_positions;
	std::vector<quat> label_orientations;
	std::vector<vec2> label_extents;
	std::vector<vec4> label_texture_ranges;

	// label visibility
	std::vector<bool> label_visibilities;

	/// for rectangle renderer a plane_render_style is needed
	cgv::render::rectangle_render_style prs;
public:
	/// different coordinate systems used to place labels
	enum CoordinateSystem
	{
		CS_LAB,
		CS_TABLE,
		CS_HEAD,
		CS_LEFT_CONTROLLER,
		CS_RIGHT_CONTROLLER
	};
	/// different alignments
	enum LabelAlignment
	{
		LA_CENTER,
		LA_LEFT,
		LA_RIGHT,
		LA_BOTTOM,
		LA_TOP
	};
	/// for each label coordinate system
	std::vector<CoordinateSystem> label_coord_systems;
	/// size of pixel in meters
	float pixel_scale;
	/// add a new label without placement information and return its index
	uint32_t add_label(const std::string& text, const rgba& bg_clr, int _border_x = 4, int _border_y = 4, int _width = -1, int _height = -1) {
		uint32_t li = lm.add_label(text, bg_clr, _border_x, _border_y, _width, _height);
		label_positions.push_back(vec3(0.0f));
		label_orientations.push_back(quat());
		label_extents.push_back(vec2(1.0f));
		label_texture_ranges.push_back(vec4(0.0f));
		label_coord_systems.push_back(CS_LAB);
		label_visibilities.push_back(true);
		return li;
	}
	/// update label text
	void update_label_text(uint32_t li, const std::string& text) { lm.update_label_text(li, text); }
	/// fix the label size based on the font metrics even when text is changed later on
	void fix_label_size(uint32_t li) { lm.fix_label_size(li); }
	/// place a label relative to given coordinate system
	void place_label(uint32_t li, const vec3& pos, const quat& ori = quat(), 
		             CoordinateSystem coord_system = CS_LAB, LabelAlignment align = LA_CENTER, float scale = 1.0f) {
		label_extents[li] = vec2(scale * pixel_scale * lm.get_label(li).get_width(), scale * pixel_scale * lm.get_label(li).get_height());
		static vec2 offsets[5] = { vec2(0.0f,0.0f), vec2(0.5f,0.0f), vec2(-0.5f,0.0f), vec2(0.0f,0.5f), vec2(0.0f,-0.5f) };
		label_positions[li] = pos + ori.get_rotated(vec3(offsets[align] * label_extents[li],0.0f));
		label_orientations[li] = ori;
		label_coord_systems[li] = coord_system;
	}
	/// hide a label
	void hide_label(uint32_t li) { label_visibilities[li] = false; }
	/// show a label
	void show_label(uint32_t li) { label_visibilities[li] = true; }
	//@}

public:
	/// standard constructor for scene
	vr_scene();
	/// return type name
	std::string get_type_name() const { return "vr_scene"; }
	/// reflect member variables
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// callback on member updates to keep data structure consistent
	void on_set(void* member_ptr);
	//@name cgv::gui::event_handler interface
	//@{
	/// provide interaction help to stream
	void stream_help(std::ostream& os);
	/// handle events
	bool handle(cgv::gui::event& e);
	//@}

	//@name cgv::render::drawable interface
	//@{
	/// initialization called once per context creation
	bool init(cgv::render::context& ctx);
	/// initialization called once per frame
	void init_frame(cgv::render::context& ctx);
	/// called before context destruction to clean up GPU objects
	void clear(cgv::render::context& ctx);
	/// draw scene here
	void draw(cgv::render::context& ctx);
	//@}

	/// cgv::gui::provider function to create classic UI
	void create_gui();
};

