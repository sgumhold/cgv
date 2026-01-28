#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/media/transfer_function.h>
#include <cgv/render/color_scale_adapter.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/arrow_render_data.h>
#include <cgv_gl/volume_renderer.h>
#include <cgv_gpgpu/texture_differentiate.h>

/// This example illustrates how to compute gradients of a 3D volume using the gpgpu::texture_differentiate algorithm.
class gradients : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
public:
	gradients() : cgv::base::node("Gradients") {
		volume_texture.set_min_filter(cgv::render::TF_LINEAR_MIPMAP_LINEAR);
		volume_texture.set_mag_filter(cgv::render::TF_LINEAR);
		volume_texture.set_wrap_s(cgv::render::TW_CLAMP_TO_BORDER);
		volume_texture.set_wrap_t(cgv::render::TW_CLAMP_TO_BORDER);
		volume_texture.set_wrap_r(cgv::render::TW_CLAMP_TO_BORDER);
		volume_texture.set_border_color(0.0f, 0.0f, 0.0f, 0.0f);
		
		gradient_texture.set_min_filter(cgv::render::TF_LINEAR);
		gradient_texture.set_mag_filter(cgv::render::TF_LINEAR);
		gradient_texture.set_wrap_s(cgv::render::TW_CLAMP_TO_EDGE);
		gradient_texture.set_wrap_t(cgv::render::TW_CLAMP_TO_EDGE);
		gradient_texture.set_wrap_r(cgv::render::TW_CLAMP_TO_EDGE);

		// an extra depth texture is used to enable mixing of opaque geometry and the volume
		depth_texture.set_min_filter(cgv::render::TF_NEAREST);
		depth_texture.set_mag_filter(cgv::render::TF_NEAREST);

		arrows.style.length_scale = 0.05f;
		arrows.style.radius_relative_to_length = 0.05f;
	}

	std::string get_type_name() const {
		return "gradients";
	}

	void stream_stats(std::ostream& os) {
		os << "gradients: volume_resolution=" << volume_resolution[0] << "x" << volume_resolution[1] << "x" << volume_resolution[2] << std::endl;
	}

	void on_set(void* member_ptr) {
		if(member_ptr == &shape_type) {
			if(get_context())
				create_test_volume(*get_context());
		}

		if(member_ptr == &gradient_operator || member_ptr == &wrap_mode) {
			if(get_context()) {
				cgv::render::context& ctx = *get_context();
				initialize_gradient_kernel(ctx);
				calculate_gradient_texture(ctx);
			}
		}

		post_redraw();
		update_member(member_ptr);
	}

	bool init(cgv::render::context& ctx) {
		cgv::render::ref_volume_renderer(ctx, 1);

		if(!arrows.init(ctx)) {
			std::cout << "Error: could not initialize arrow render data" << std::endl;
			return false;
		}

		if(!initialize_gradient_kernel(ctx)) {
			std::cout << "Error: could not initialize GPU texture differentiation algorithm" << std::endl;
			return false;
		}
		
		transfer_function->set_color_points({
			{ 0.0f, cgv::rgb(0.3f, 0.3f, 1.0f) },
			{ 1.0f, cgv::rgb(1.0f, 0.3f, 0.3f) }
		});
		transfer_function->set_opacity_points({
			{ 0.0f, 0.0f },
			{ 1.0f, 1.0f },
		});
		color_scale_adapter.set_color_scale(transfer_function);
		
		create_test_volume(ctx);
		return true;
	}

	void clear(cgv::render::context& ctx) {
		cgv::render::ref_volume_renderer(ctx, -1);
		arrows.destruct(ctx);
		gradient_kernel.destruct(ctx);
		color_scale_adapter.destruct(ctx);
	}

	void init_frame(cgv::render::context& ctx) {
		if(depth_texture.is_created() && (ctx.get_width() != depth_texture.get_width() || ctx.get_height() != depth_texture.get_height()))
			depth_texture.destruct(ctx);

		if(!depth_texture.is_created())
			depth_texture.create(ctx, cgv::render::TT_2D, ctx.get_width(), ctx.get_height());
	}

	void draw(cgv::render::context& ctx) {
		if(show_gradients)
			arrows.render(ctx);
	}

	void after_finish(cgv::render::context& ctx) {
		// copy the contents of the depth buffer from the opaque geometry into the extra depth texture
		depth_texture.replace_from_buffer(ctx, 0, 0, 0, 0, ctx.get_width(), ctx.get_height());

		if(show_volume) {
			cgv::render::volume_renderer& volume_renderer = cgv::render::ref_volume_renderer(ctx);
			volume_renderer.set_render_style(volume_style);
			volume_renderer.set_volume_texture(&volume_texture);

			volume_renderer.set_transfer_function_texture(&color_scale_adapter.get_texture(ctx));

			volume_renderer.set_depth_texture(&depth_texture);
			volume_renderer.set_bounding_box(volume_bounding_box);
			volume_renderer.transform_to_bounding_box(true);
			volume_renderer.render(ctx, 0, 0);
		}
	}

	void create_gui() {
		add_decorator("Gradients", "heading", "level=2");

		add_member_control(this, "Shape", shape_type, "dropdown", "enums='Ellipsoid,Cuboid'");

		add_member_control(this, "Show volume", show_volume, "check");
		add_member_control(this, "Show gradients", show_gradients, "check");
		add_member_control(this, "Wrap mode", wrap_mode, "dropdown", "enums='Repeat, Mirrored repeat, Clamp to edge, Clamp to border'");
		add_member_control(this, "Gradient operator", gradient_operator, "dropdown", "enums='Forward difference,Backward difference,Central difference,Sobel'");
		
		if(begin_tree_node("Volume style", volume_style, false)) {
			align("\a");
			add_gui("volume_style", volume_style);
			align("\b");
			end_tree_node(volume_style);
		}

		if(begin_tree_node("Arrow style", arrows.style, false)) {
			align("\a");
			add_gui("arrow_style", arrows.style);
			align("\b");
			end_tree_node(arrows.style);
		}
	}

private:
	/// shape of volume content
	enum class ShapeType {
		kEllipsoid,
		kCuboid
	};

	bool initialize_gradient_kernel(cgv::render::context& ctx) {
		return gradient_kernel.init(
			ctx,
			cgv::render::TextureType::TT_3D,
			sl::ImageFormatLayoutQualifier::k_r32f,
			wrap_mode,
			gradient_operator,
			cgv::gpgpu::DifferentiationOutput::kDerivative);
	}

	void create_test_volume(cgv::render::context& ctx) {
		volume_resolution = cgv::ivec3(96, 64, 29); // odd-shaped resolution to show the shader is not limited to nice power-of-two resolutions

		// destruct previous textures
		volume_texture.destruct(ctx);
		gradient_texture.destruct(ctx);
		
		const cgv::vec3 half_volume_resolution = 0.5f * cgv::vec3(volume_resolution);

		// compute volume data
		std::vector<float> vol_data(volume_resolution[0] * volume_resolution[1] * volume_resolution[2], 0.0f);
		cgv::ivec3 voxel = { 0 };
		unsigned i = 0;
		for(voxel[2] = 0; voxel[2] < volume_resolution[2]; ++voxel[2]) {
			for(voxel[1] = 0; voxel[1] < volume_resolution[1]; ++voxel[1]) {
				for(voxel[0] = 0; voxel[0] < volume_resolution[0]; ++voxel[0], ++i) {
					cgv::vec3 position = cgv::vec3(voxel) / half_volume_resolution - 1.0f;
					if(shape_type == ShapeType::kEllipsoid) {
						float distance = length(position);
						if(distance >= 0.5f && distance <= 1.0f && position[1] < 0.4f)
							vol_data[i] = 1.0f;
					} else {
						if(position[0] > -0.5f && position[0] < 0.5f && position[1] > -0.5f && position[1] < 0.5f && position[2] > -0.5f && position[2] < 0.5f)
							vol_data[i] = 1.0f;
					}
				}
			}
		}

		// transfer volume data into volume texture and compute mipmaps
		cgv::data::data_format vol_df(volume_resolution[0], volume_resolution[1], volume_resolution[2], cgv::type::info::TypeId::TI_FLT32, cgv::data::ComponentFormat::CF_R);
		cgv::data::const_data_view vol_dv(&vol_df, vol_data.data());
		volume_texture.create(ctx, vol_dv, 0);
		volume_texture.generate_mipmaps(ctx);

		// update the volume bounding box to later scale the rendering accordingly
		// the bounding box is centered around the origin and is scaled so that the minimum extent is 1
		unsigned min_resolution = cgv::math::min_value(volume_resolution);
		const cgv::vec3 extent = cgv::vec3(volume_resolution) / cgv::vec3(static_cast<float>(min_resolution));
		cgv::vec3 min = -0.5f * extent;
		volume_bounding_box.ref_min_pnt() = min;
		volume_bounding_box.ref_max_pnt() = min + extent;

		// make sure gradient texture is recomputed
		calculate_gradient_texture(ctx);
	}

	void calculate_gradient_texture(cgv::render::context& ctx) {
		// compute gradient texture with gpgpu differentiation algorithm that internally uses a compute shader
		if(!gradient_texture.is_created())
			gradient_texture.create(ctx, cgv::render::TT_3D, volume_resolution[0], volume_resolution[1], volume_resolution[2]);

		if(!gradient_kernel.is_initialized()) {
			std::cout << "Error: GPU differentiation algorithm is not initialized" << std::endl;
		}

		if(gradient_kernel.dispatch(ctx, volume_texture, gradient_texture))
			create_arrows(ctx);

		create_arrows(ctx);
	}

	void create_arrows(cgv::render::context& ctx) {
		// read texture into memory
		cgv::data::data_view dv;
		if(!gradient_texture.copy(ctx, dv))
			std::cout << "Error: could not read back gradient texture" << std::endl;

		cgv::vec4* gradient_data = dv.get_ptr<cgv::vec4>();

		// construct geometry data for arrow rendering
		arrows.clear();

		const cgv::vec3 half_volume_resolution = 0.5f * cgv::vec3(volume_resolution);

		const float voxel_size = 1.0f / static_cast<float>(cgv::math::min_value(volume_resolution));

		// Loop over all voxels and place arrows to visualize the gradients
		cgv::ivec3 voxel = { 0 };
		unsigned i = 0;
		for(voxel[2] = 0; voxel[2] < volume_resolution[2]; ++voxel[2]) {
			for(voxel[1] = 0; voxel[1] < volume_resolution[1]; ++voxel[1]) {
				for(voxel[0] = 0; voxel[0] < volume_resolution[0]; ++voxel[0], ++i) {
					cgv::vec3 gradient = cgv::vec3(gradient_data[i]);
					if(gradient.sqr_length() > 0.0f) {
						cgv::vec3 position = 0.5f * volume_bounding_box.get_extent() * (cgv::vec3(voxel) / half_volume_resolution - 1.0f) + 0.5f * voxel_size;
						cgv::vec3 normal = -normalize(gradient);
						cgv::rgb color = 0.5f * cgv::rgb(normal[0], normal[1], normal[2]) + 0.5f;
						arrows.add(position, color, normal);
					}
				}
			}
		}
	}

	ShapeType shape_type = ShapeType::kEllipsoid;
	
	/// voxel resolution of the volume in all three dimensions
	cgv::box3 volume_bounding_box = { cgv::vec3(0.0f), cgv::vec3(1.0f) };
	cgv::ivec3 volume_resolution = { 0 };
	/// whether to show the volume
	bool show_volume = true;
	/// whether to show the gradients
	bool show_gradients = true;

	cgv::gpgpu::texture_differentiate gradient_kernel;
	cgv::gpgpu::WrapMode wrap_mode = cgv::gpgpu::WrapMode::kClampToBorder;
	cgv::gpgpu::DifferentiationOperator gradient_operator = cgv::gpgpu::DifferentiationOperator::kSobel;

	// Render members
	cgv::render::arrow_render_data<> arrows;
	cgv::render::volume_render_style volume_style;
	std::shared_ptr<cgv::media::transfer_function> transfer_function = std::make_shared<cgv::media::transfer_function>();
	cgv::render::color_scale_adapter color_scale_adapter;
	cgv::render::texture volume_texture = cgv::render::texture("flt32[R]");
	cgv::render::texture gradient_texture = cgv::render::texture("flt32[R,G,B,A]");
	cgv::render::texture depth_texture = cgv::render::texture("[D]");
};

#include <cgv/base/register.h>

/// register a factory to create new gradients instances
cgv::base::factory_registration<gradients> gradients_fac("New/GPGPU/Gradients");
