#include "library.h"

// CGV Framework core
#include "cgv/math/fvec.h"
#include "cgv/render/context.h"
#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>
#include <cgv/render/stereo_view.h>
#include <cgv/render/shader_program.h>

#include "cesium_tiles/TileManager.h"
#include "cesium_tiles/TileRenderer.h"

class cesium_tiles : public cgv::base::node,
					 public cgv::render::drawable,
					 public cgv::gui::provider,
					 public cgv::gui::event_handler
{
protected:
	TileManager tileManager;
	TileRenderer tileRenderer;
	cgv::render::shader_program _elevationShader;
	cgv::render::stereo_view* _camera = nullptr;


public:
	cesium_tiles();

	virtual ~cesium_tiles();

	virtual bool init(cgv::render::context& ctx) override;
	virtual void init_frame(cgv::render::context& ctx) override;
	virtual void clear(cgv::render::context& ctx) override;
	virtual void draw(cgv::render::context& ctx) override;

	std::string get_type_name() const override;
	virtual void stream_help(std::ostream& os) override;

	virtual bool handle(cgv::gui::event& e) override;
	virtual void on_set(void* member_ptr) override;
	virtual void create_gui() override;
};

cgv::base::object_registration<cesium_tiles> cesium_tiles_object("");