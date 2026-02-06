#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_post/depth_halos.h>
#include <cgv_post/depth_masking.h>
#include <cgv_post/outline.h>
#include <cgv_post/screen_space_ambient_occlusion.h>
#include <cgv_post/temporal_anti_aliasing.h>

using namespace cgv::render;

class post_processor :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
protected:
	view* view_ptr = nullptr;
	
	cgv::post::depth_halos dh;
	cgv::post::depth_masking dm;
	cgv::post::outline ol;
	cgv::post::screen_space_ambient_occlusion ssao;
	cgv::post::temporal_anti_aliasing taa;

	bool dh_enabled  = false;
	bool dm_enabled  = false;
	bool ol_enabled  = false;
	bool ssao_enabled = false;
	bool taa_enabled = false;

public:
	post_processor() : cgv::base::node("Post Processor") 
	{
		dh.set_enabled(false);
		dm.set_enabled(false);
		ol.set_enabled(false);
		ssao.set_enabled(false);
		taa.set_enabled(false);
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("depth_halos", dh_enabled) &&
			rh.reflect_member("depth_masking", dm_enabled) &&
			rh.reflect_member("outline", ol_enabled) &&
			rh.reflect_member("ssao", ssao_enabled) &&
			rh.reflect_member("taa", taa_enabled);
	}
	void on_set(void* member_ptr) 
	{
		if (member_ptr == &dh_enabled)
			dh.set_enabled(dh_enabled);
		if (member_ptr == &dm_enabled)
			dm.set_enabled(dm_enabled);
		if (member_ptr == &ol_enabled)
			ol.set_enabled(ol_enabled);
		if (member_ptr == &ssao_enabled)
			ssao.set_enabled(ssao_enabled);
		if (member_ptr == &taa_enabled)
			taa.set_enabled(taa_enabled);
		taa.reset();
		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const 
	{
		return "post_processor";
	}
	void clear(cgv::render::context& ctx) 
	{
		dh.destruct(ctx);
		dm.destruct(ctx);
		ol.destruct(ctx);
		ssao.destruct(ctx);
		taa.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) 
	{
		bool success = dh.init(ctx);
		success &= dm.init(ctx);
		success &= ol.init(ctx);
		success &= ssao.init(ctx);
		success &= taa.init(ctx);

		// setup some scene specific parameters
		dh.set_strength(2.0f);
		dh.set_depth_scale(0.3f);

		return success;
	}

	void init_frame(cgv::render::context& ctx) 
	{
		if(!view_ptr && (view_ptr = find_view_as_node())) {
			// temporal anti aliasing needs access to the current view
			taa.set_view(view_ptr);
			dm.set_view(view_ptr);
		}
		// call ensure in init_frame on post processing effects to make sure the internal buffers are created and sized accordingly
		dh.ensure(ctx);
		dm.ensure(ctx);
		ol.ensure(ctx);
		ssao.ensure(ctx);
		taa.ensure(ctx);

		taa.begin(ctx);
		dh.begin(ctx);
		dm.begin(ctx);
		ol.begin(ctx);
		ssao.begin(ctx);
	}
	void finish_frame(cgv::render::context& ctx)
	{
		ssao.end(ctx);
		ol.end(ctx);
		dm.end(ctx);
		dh.end(ctx);
		taa.end(ctx);
	}
	void create_gui() 
	{
		add_decorator("Post Processor", "heading");
		taa.create_gui_tree_node(this, "TAA", false);
		ssao.create_gui_tree_node(this, "SSAO", false);
		dh.create_gui_tree_node(this, "Depth Halos", false);
		dm.create_gui_tree_node(this, "Depth Masking", false);
		ol.create_gui_tree_node(this, "Outline", false);
	}
};

#include <cgv/base/register.h>

#include "lib_begin.h"

/// register a newly created post processor object
extern CGV_API cgv::base::object_registration<post_processor> post_processor_reg("post processor reg");
