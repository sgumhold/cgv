#include "stream_vis.h"
#include <cgv/base/register.h>
#include <cgv/utils/file.h>
#include <cgv/gui/application.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/gui_driver.h>

namespace stream_vis {
	stream_vis::stream_vis()
	{
		set_name("stream_vis");
	}
	stream_vis::~stream_vis()
	{
	}
	void generate_gui(cgv::gui::gui_driver_ptr d)
	{
		cgv::gui::window_ptr w = cgv::gui::application::create_window(1280, 768, "cgv 3d viewer");
		w->show();
		cgv::base::register_object(w, "register viewer window");
	}

	void stream_vis::run()
	{
		cgv::render::render_config_ptr rcp = cgv::render::get_render_config();
		if (!rcp->is_property("debug"))
			std::cout << "no debug property ;(" << std::endl;
		//	cgv::base::register_prog_name(argv[0]);
		cgv::signal::connect(cgv::gui::on_gui_driver_registration(), generate_gui);
		cgv::base::disable_registration_event_cleanup();
		cgv::base::enable_permanent_registration();
		cgv::base::enable_registration();
		if (!getenv("CGV_DIR")) {
			std::cerr << "please set CGV_DIR environment variable first!" << std::endl;
			abort();
		}
		std::string cgv_dir = cgv::utils::file::clean_path(getenv("CGV_DIR"));
		std::string shader_config_cmd = "type(shader_config):shader_path='" +
			cgv_dir + "/libs/plot/glsl;" +
			cgv_dir + "/libs/cgv_gl/glsl'";
		cgv::base::process_command("plugin:cg_ext");
		cgv::base::process_command("plugin:cmi_io");
		cgv::base::process_command("plugin:cg_icons");
		cgv::base::process_command("plugin:cg_fltk");
		cgv::base::process_command(shader_config_cmd);
		cgv::base::process_command("plugin:cmf_tt_gl_font");
		//cgv::base::process_command("plugin:crg_stereo_view");
		cgv::base::process_command("plugin:crg_grid");
		//  cgv::base::register_object(console::get_console());
		//	process_config_file(cfg_file_name);
		cgv::base::register_object(cgv::base::group_ptr(this));
		cgv::base::enable_registration_event_cleanup();
		connect(cgv::gui::get_animation_trigger().shoot, this, &stream_vis::timer_event);
		bool res = cgv::gui::application::run();
		cgv::base::unregister_all_objects();
	}
	void stream_vis::timer_event(double t, double dt)
	{
		process_context_changes();
		for (context_ptr cp : context_pool)
			if (cp->is_outofdate()) 
				post_redraw();
	}
	void stream_vis::register_context(context_ptr ctx_ptr)
	{
		lock.lock();
		new_contexts.push_back(ctx_ptr);
		lock.unlock();
	}
	void stream_vis::unregister_context(context_ptr ctx_ptr)
	{
		lock.lock();
		old_contexts.push_back(ctx_ptr);
		lock.unlock();
	}
	void stream_vis::process_context_changes()
	{
		if (!lock.try_lock())
			return;
		for (context_ptr cp : old_contexts) {
			auto iter = std::find(context_pool.begin(), context_pool.end(), cp);
			if (iter == context_pool.end())
				continue;
			cgv::base::unregister_object(cp);
			context_pool.erase(iter);
		}
		old_contexts.clear();
		for (context_ptr cp : new_contexts) {
			auto iter = std::find(context_pool.begin(), context_pool.end(), cp);
			if (iter != context_pool.end())
				continue;
			context_pool.push_back(cp);
			cgv::base::register_object(cp);
		}
		new_contexts.clear();
		lock.unlock();
	}

	void stream_vis::create_gui()
	{
		add_decorator("stream_vis", "heading");

	}

}


/*
#include "streaming_time_series.h"
#include <cgv/utils/advanced_scan.h>
#include <cgv/render/drawable.h>
#include <cgv/base/node.h>
#include <plot/plot2d.h>
#include <plot/plot3d.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>
#include <cgv/render/render_buffer.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/texture.h>
#include <cgv/gui/provider.h>
#include <iostream>
#include <cgv/base/register.h>
#include <cgv/gui/application.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/gui/base_provider_generator.h>
#include <cgv/utils/file.h>
#include <cgv/render/context.h>
#include <cgv/base/console.h>
#include <cgv/os/thread.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/advanced_scan.h>

#include <map>


void stream_value(std::ostream& os, LV_Type t, const LV_Value& v)
{
	switch (t) {
	case LV_Signed: os << reinterpret_cast<const int64_t&>(v[0]); break;
	case LV_Unsigned: os << reinterpret_cast<const uint64_t&>(v[0]); break;
	case LV_Float:os << reinterpret_cast<const double&>(v[0]); break;
	case LV_Bool:os << reinterpret_cast<const bool&>(v[0]) ? "True" : "False"; break;
	}
}

struct LV_Context : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
{
	std::vector<LV_Declaration> inputs;
	std::vector<stream_vis::streaming_time_series*> input_time_series;
	std::vector<unsigned> input_subplots;
	std::vector<LV_Declaration> outputs;
	std::vector<stream_vis::streaming_time_series*> output_time_series;
	std::vector<std::string> triggers;
	std::vector<stream_vis::streaming_time_series*> typed_time_series;

	LV_Type get_input_type(uint16_t index) const { return inputs[index].type; }
	LV_Type get_output_type(uint16_t index) const { return outputs[index].type; }

	/// mapping of input and output indices to unique indices
	uint16_t map_input_to_unique_index(uint16_t input_index) const { return input_index; }
	uint16_t map_output_to_unique_index(uint16_t output_index) const { return (uint16_t)inputs.size() + output_index; }
	bool is_input_index(uint16_t unique_index) const { return unique_index < inputs.size(); }
	bool is_output_index(uint16_t unique_index) const { return unique_index >= inputs.size(); }
	uint16_t map_form_unique_index(uint16_t unique_index) const { return unique_index < inputs.size() ? unique_index : unique_index - (uint16_t)inputs.size(); }

	std::string get_trigger_message(uint16_t index) const { return triggers[index]; }
	LV_Context(
		uint16_t num_inputs, const LV_Declaration* inputs,
		uint16_t num_outputs, const LV_Declaration* outputs,
		uint16_t num_triggers, const char** messages, const char* declarations)
	{
		set_name("LV_Context");
		// store declarations
		std::copy(inputs, inputs + num_inputs, std::back_inserter(this->inputs));
		std::copy(outputs, outputs + num_outputs, std::back_inserter(this->outputs));
		std::copy(messages, messages + num_triggers, std::back_inserter(this->triggers));

		extract_time_series(declarations);
		extract_subplots();
	}
	~LV_Context()
	{
		for (auto& tsp : typed_time_series)
			delete tsp;
	}	
	void extract_time_series(const std::string& declarations)
	{
		std::map<std::string, uint16_t> name2index;
		for (const auto& id : inputs) {
			uint16_t idx = name2index[id.name] = map_input_to_unique_index(uint16_t(&id - &inputs.front()));
			switch (id.type) {
			case LV_Signed:   typed_time_series.push_back(new stream_vis::int_time_series(idx)); break;
			case LV_Unsigned: typed_time_series.push_back(new stream_vis::uint_time_series(idx)); break;
			case LV_Float:    typed_time_series.push_back(new stream_vis::float_time_series(idx)); break;
			case LV_Bool:     typed_time_series.push_back(new stream_vis::bool_time_series(idx)); break;
			}
			typed_time_series.back()->name = std::string("inp ") + id.name;
			input_time_series.push_back(typed_time_series.back());
		}
		for (const auto& od : outputs) {
			uint16_t idx = name2index[od.name] = map_output_to_unique_index(uint16_t(&od - &outputs.front()));
			switch (od.type) {
			case LV_Signed:   typed_time_series.push_back(new stream_vis::int_time_series(idx)); break;
			case LV_Unsigned: typed_time_series.push_back(new stream_vis::uint_time_series(idx)); break;
			case LV_Float:    typed_time_series.push_back(new stream_vis::float_time_series(idx)); break;
			case LV_Bool:     typed_time_series.push_back(new stream_vis::bool_time_series(idx)); break;
			}
			typed_time_series.back()->name = std::string("out ") + od.name;
			output_time_series.push_back(typed_time_series.back());
		}
		std::vector<cgv::utils::line> lines;
		cgv::utils::split_to_lines(declarations, lines);
		for (const auto& l : lines) {
			// skip empty lines
			if (l.empty())
				continue;
			std::vector<cgv::utils::token> tokens;
			cgv::utils::split_to_tokens(l.begin, l.end, tokens, ":;,=", false, "\"'", "\"'");
			if (tokens.size() < 5) {
				std::cout << "skipping declaration line <" << to_string(l) << ">" << std::endl;
				continue;
			}
			if (to_string(tokens[1]) != ":" || to_string(tokens[3]) != "=") {
				std::cout << "expected declaration syntax name:type=n1,...,nd but found line <" << to_string(l) << ">" << std::endl;
				continue;
			}
			std::string name = to_string(tokens[0]);
			std::string type = to_string(tokens[2]);
			std::vector<uint16_t> stream_indices;
			size_t i = 3;
			do {
				++i;
				if (name2index.find(to_string(tokens[i])) != name2index.end())
					stream_indices.push_back(name2index[to_string(tokens[i])]);
				else {
					std::cerr << "could not find name " << to_string(tokens[i]) << " in inputs or outputs" << std::endl;
					stream_indices.push_back(0);
				}
				++i;
			} while (i + 1 < tokens.size() && to_string(tokens[i]) == ",");
			if (type.substr(0, 3) == "vec") {
				if (type.length() != 4) {
					std::cout << "unknown type <" << type << ">" << std::endl;
					continue;
				}
				int dim = type[3] - '0';
				if (stream_indices.size() != dim) {
					std::cout << "composed stream of type <" << type << "> needs to be defined from " << dim << " streams, but found only " << stream_indices.size() << ":\n" << to_string(l) << std::endl;
					continue;
				}
				if (dim < 2 || dim > 4) {
					std::cout << "only support composed vec type of dim 2-4 but got " << dim << ":\n" << to_string(l) << std::endl;
					continue;
				}
				switch (dim) {
				case 2: typed_time_series.push_back(new stream_vis::fvec_time_series<2>(stream_indices[0], stream_indices[1])); break;
				case 3: typed_time_series.push_back(new stream_vis::fvec_time_series<3>(stream_indices[0], stream_indices[1], stream_indices[2])); break;
				case 4: typed_time_series.push_back(new stream_vis::fvec_time_series<4>(stream_indices[0], stream_indices[1], stream_indices[2], stream_indices[3])); break;
				}
				typed_time_series.back()->name = name;
			}
			else if (type == "quat") {
				if (stream_indices.size() != 4) {
					std::cout << "composed stream of type <" << type << "> needs to be defined from 4 streams, but found only " << stream_indices.size() << ":\n" << to_string(l) << std::endl;
					continue;
				}
				typed_time_series.push_back(new stream_vis::quat_time_series(stream_indices[0], stream_indices[1], stream_indices[2], stream_indices[3])); break;
				typed_time_series.back()->name = name;
			}
		}
	}
	void extract_subplots()
	{
		for (auto its : input_time_series) {
			input_subplots.push_back(plot.add_sub_plot(its->name));
		}
	}
	void process_inputs(uint16_t num_inputs, LV_IndexedValue* values, double timestamp)
	{
		int i;
		for (i = 0; i < num_inputs; ++i) {
			values[i].index = map_input_to_unique_index(values[i].index);
		}
		for (auto& tts : typed_time_series)
			tts->extract_from_values(num_inputs, values, timestamp);
		for (int i = 0; i < num_inputs; ++i) {
			uint16_t ii = map_form_unique_index(values[i].index);
			unsigned p1 = input_subplots[ii];
			plot.ref_sub_plot_samples(p1).clear();
			input_time_series[ii]->append_cached_samples(plot.ref_sub_plot_samples(p1));
		}
		plot.adjust_domain_to_data();
	}
	void process_outputs(uint16_t num_outputs, LV_IndexedValue* values, double timestamp)
	{
		for (int i = 0; i < num_outputs; ++i)
			values[i].index = map_output_to_unique_index(values[i].index);
		for (auto& tts : typed_time_series)
			tts->extract_from_values(num_outputs, values, timestamp);
	}
	std::string get_type_name() const
	{
		return "LV_Context";
	}
	void on_set(void* member_ptr)
	{
		if ((member_ptr >= &plot.ref_domain_min()(0) && member_ptr < &plot.ref_domain_min()(0) + plot.get_dim()) ||
			(member_ptr >= &plot.ref_domain_max()(0) && member_ptr < &plot.ref_domain_max()(0) + plot.get_dim())) {
			plot.adjust_tick_marks_to_domain();
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{

		// create GPU objects for offline rendering
		tex.create(ctx, cgv::render::TT_2D, 2048, 1024);
		depth.create(ctx, 2048, 1024);
		fbo.create(ctx, 2048, 1024);
		fbo.attach(ctx, depth);
		fbo.attach(ctx, tex);

		// and init plot
		return plot.init(ctx) && fbo.is_complete(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		// first call init frame of plot
		plot.init_frame(ctx);

		if (!fbo.is_created())
			return;

		// if fbo is created, perform offline rendering with world space in the range [-1,1]� and white background
		fbo.enable(ctx);
		fbo.push_viewport(ctx);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(cgv::math::identity4<double>());
		ctx.push_projection_matrix();
		ctx.set_projection_matrix(cgv::math::identity4<double>());
		glDepthMask(GL_FALSE);
		plot.draw(ctx);
		glDepthMask(GL_TRUE);
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();

		fbo.pop_viewport(ctx);
		fbo.disable(ctx);

		// generate mipmaps in rendered texture and in case of success enable anisotropic filtering
		if (tex.generate_mipmaps(ctx))
			tex.set_min_filter(cgv::render::TF_ANISOTROP, 16.0f);
	}
	void draw(cgv::render::context& ctx)
	{
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::translate4<float>(vec3(0, 1, 0)));
		if (fbo.is_created()) {
			// use default shader with texture support to draw offline rendered plot
			glDisable(GL_CULL_FACE);
			auto& prog = ctx.ref_default_shader_program(true);
			tex.enable(ctx);
			prog.enable(ctx);
			ctx.set_color(rgba(1, 1, 1, 1));
			ctx.push_modelview_matrix();
			// scale down in y-direction according to texture resolution
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(1.0, 0.5, 1.0));
			ctx.tesselate_unit_square();
			ctx.pop_modelview_matrix();
			prog.disable(ctx);
			tex.disable(ctx);
			glEnable(GL_CULL_FACE);
		}
		else
			plot.draw(ctx);
		ctx.pop_modelview_matrix();
	}
	void create_gui()
	{
		plot.create_gui(this, *this);
	}
	// plot that can manage several 2d sub plots
	cgv::plot::plot2d plot;
	// GPU objects for offline 
	cgv::render::texture tex;
	cgv::render::render_buffer depth;
	cgv::render::frame_buffer fbo;
};

LV_Context* LV_create_context(
	uint16_t num_inputs, const LV_Declaration* inputs,
	uint16_t num_outputs, const LV_Declaration* outputs,
	uint16_t num_triggers, const char** messages, const char* declarations)
{
	if (!get_state().gui_thread) {
		ref_state().gui_thread = new cgv_thread();
		ref_state().gui_thread->start(true);
	}
	static char* type_names[] = { "Signed", "Unsigned", "Float", "Bool" };
	if (get_state().debug_start) {
		// print out declaration
		std::cout << "LV_start(\n";
		for (uint16_t i = 0; i < num_inputs; ++i)
			std::cout << "\tinp " << inputs[i].name << ":" << type_names[inputs[i].type] << "\n";
		for (uint16_t o = 0; o < num_outputs; ++o)
			std::cout << "\tout " << outputs[o].name << ":" << type_names[outputs[o].type] << "\n";
		for (uint16_t t = 0; t < num_triggers; ++t)
			std::cout << "\ttrigger '" << messages[t] << "'\n";
		std::cout << ")" << std::endl;
	}
	LV_Context* context = new LV_Context(num_inputs, inputs, num_outputs, outputs, num_triggers, messages, declarations);
	ref_state().gui_thread->register_object(context);
	return context;
}

void LV_drop_context(LV_Context* context)
{
	delete context;
}

/// Indicates that new input values arrived
void LV_inputs(LV_Context* context, uint16_t num_inputs, LV_IndexedValue* values, double timestamp)
{
	if (get_state().debug_inputs) {
		std::cout << "LV_inputs(" << timestamp << ":\n";
		for (uint16_t i = 0; i < num_inputs; ++i) {
			std::cout << "\t" << values[i].index << "=";
			stream_value(std::cout, context->get_input_type(values[i].index), values[i].value);
			std::cout << "\n";
		}
		std::cout << ")" << std::endl;
	}
	context->process_inputs(num_inputs, values, timestamp);
}

/// Indicates that new output values were computed
void LV_outputs(LV_Context* context, uint16_t num_outputs, LV_IndexedValue* values, double timestamp, bool periodic)
{
	if (get_state().debug_outputs) {
		std::cout << "LV_outputs(" << timestamp << ":";
		if (periodic)
			std::cout << "periodic";
		std::cout << "\n";

		for (uint16_t o = 0; o < num_outputs; ++o) {
			std::cout << "\t" << values[o].index << "=";
			stream_value(std::cout, context->get_output_type(values[o].index), values[o].value);
			std::cout << "\n";
		}
		std::cout << ")" << std::endl;
	}
	context->process_outputs(num_outputs, values, timestamp);
}
/// indicates that triggers have been triggered
void LV_triggers(LV_Context* context, uint16_t num_triggers, uint16_t* triggers, double timestamp)
{
	if (get_state().debug_triggers) {
		std::cout << "LV_triggers(" << timestamp << ":\n";
		for (uint16_t t = 0; t < num_triggers; ++t)
			std::cout << "\ttrigger " << triggers[t] << " sais '" << context->get_trigger_message(triggers[t]) << "'\n";
		std::cout << ")" << std::endl;
	}
}

*/