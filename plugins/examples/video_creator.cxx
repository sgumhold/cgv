#include <cgv/media/video/video_writer.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/signal/signal.h>
#include <cgv/signal/rebind.h>
#include <cgv/render/drawable.h>
#include <cgv/data/data_format.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::gui;
using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::render;
using namespace cgv::media::video;

class video_creator :
	public base,
	public provider,
	public drawable
{
protected:
	data_format df;
	data_view dv;
	video_writer writer;
	trigger trig;
	std::string file_name;
	int width, height;
	float fps;
	button_ptr record_button, open_button, close_button;
	bool recording;
	int frame_index;
	bool use_timer;
	void timer_event()
	{
		if (use_timer)
			post_redraw();
	}
public:
	video_creator() : writer("avi")
	{
		recording = false;
		frame_index = 0;
		width = height = -1;
		use_timer = true;
		fps = 25;
		connect_copy(trig.shoot, rebind(this, &video_creator::timer_event));
	}
	/// overload to return the type name of this object
	std::string get_type_name() const
	{
		return "video creator";
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const
	{
		return "view/video creator";
	}
	void open_file()
	{
		if (!get_context())
			return;
		if (width == -1) {
			if (height != -1)
				get_context()->resize(get_context()->get_width(),height);
		}
		else {
			if (height == -1)
				get_context()->resize(width, get_context()->get_height());
			else
				get_context()->resize(width, height);
		}
		if (!file_name.empty())
			close_file();
		file_name = file_save_dialog("select video ouput file", video_writer::construct_filter_string());
		if (file_name.empty())
			return;
		df = data_format(get_context()->get_width(),get_context()->get_height(),
			cgv::type::info::TI_UINT8, CF_RGB);

		if (!writer.open(file_name, df, fps, true)) {
			file_name = std::string();
			return;
		}

		dv.~data_view();
		new (&dv) data_view(&df);
		std::cout << "opened file " << file_name.c_str() << " for video ouput\n"
			<< "  at resolution " << df.get_width() << "x" << df.get_height() 
			<< std::endl;

		frame_index = 0;

		record_button->set("active", true);
		close_button->set("active", true);
		open_button->set("active", false);
	}
	void toggle_record()
	{
		if (recording) {
			recording = false;
			close_button->set("active", true);
			record_button->set("label", std::string("record"));
			trig.stop();
		}
		else {
			recording = true;
			close_button->set("active", false);
			record_button->set("label", std::string("pause"));
			trig.schedule_recuring(1.0/25);
		}
	}
	void close_file()
	{
		if (file_name.empty())
			return;
		record_button->set("active", false);
		open_button->set("active", true);
		close_button->set("active", false);
		writer.close();
		std::cout << "closed file " << file_name.c_str() << std::endl;
		file_name = std::string();
		dv = data_view();
	}
	void after_finish(context& ctx)
	{
		if (!recording || ctx.get_render_pass() != RP_MAIN)
			return;
		glReadPixels(0,0,df.get_width(), df.get_height(),
						 GL_RGB,GL_UNSIGNED_BYTE,dv.get_ptr<unsigned char>());
		if (writer.write_frame(dv)) {
			if (++frame_index % 100 == 0)
				std::cout << "wrote " << frame_index << " frames" << std::endl;
		}
		else {
			recording = false;
			close_file();
		}
	}
	/// you must overload this for gui creation
	void create_gui()
	{
		open_button = add_button("open file");
		record_button = add_button("record");
		close_button = add_button("close file");
		record_button->set("active", false);
		close_button->set("active", false);
		connect_copy(open_button->click, rebind(this, &video_creator::open_file));
		connect_copy(record_button->click, rebind(this, &video_creator::toggle_record));
		connect_copy(close_button->click, rebind(this, &video_creator::close_file));
		connect_copy(add_button("list codecs")->click, rebind(this, &video_creator::list_codecs));
		add_control("width", width);
		add_control("height", height);
		add_control("fps", fps);
		add_control("use_timer", use_timer, "check");
	}
	void list_codecs()
	{
		std::vector<std::string> codec_names;
		writer.scan_codecs(codec_names);
		unsigned int i;
		std::cout << "registered codecs:" << std::endl;
		for (i=0; i<codec_names.size(); ++i)
			std::cout << "   " << codec_names[i] << std::endl;
	}
};

#include <cgv/base/register.h>

extern factory_registration<video_creator> video_creator_fac("new/video creater", 'Y', true);

/*
	cgv::media::video::video_writer writer("avi");

	std::string best_codec = writer.get_codec();

	std::vector<std::string> codec_names;
	writer.scan_codecs(codec_names);
	unsigned int i;
	std::cout << "registered codecs:" << std::endl;
	for (i=0; i<codec_names.size(); ++i)
		std::cout << "   " << codec_names[i] << std::endl;

	std::cout << "best codec = " << best_codec.c_str() << std::endl;
	writer.set_codec(best_codec);

	cgv::data::data_format df(120,80,cgv::type::info::TI_UINT8,CF_RGB);
	cgv::data::data_view dv(&df);


	unsigned char* p = dv.get_ptr<unsigned char>();
	unsigned int n = df.get_size()*df.get_entry_size();
	std::fill(p,p+n,0);
	if (writer.open("d:/out.avi", df, 200, true)) {
		std::cout << "opened file with\n   quality = " << writer.get<float>("quality")
			<< "\n   bytes_per_sec = " << writer.get<unsigned int>("bytes_per_sec")
			<< "\n   key_frame_step = " << writer.get<unsigned int>("key_frame_step")
			<< std::endl;
		for (i=0; i<200; ++i) {
			p[i] = 255;
			if (writer.write_frame(dv)) {
				if (i % 20 == 0)
					std::cout << "wrote frame " << i << std::endl;
			}
			else {
				std::cout << "could not write frame " << i << std::endl;
				break;
			}
		}
		if (!writer.close())
			std::cout << "could not close file" << std::endl;
	}
	else 
		std::cout << "could not open file" << std::endl;
*/