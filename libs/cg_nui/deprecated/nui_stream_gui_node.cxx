#include <cgv/base/base.h>
#include <cgv/math/ftransform.h>
#include "nui_stream_gui_node.h"

namespace cgv {
	namespace nui {
		nui_stream_gui_node::nui_stream_gui_node(const std::string& name) : nui_node(name, SM_UNIFORM)
		{
			frame_grabber = SL::Screen_Capture::CreateCaptureConfiguration([]() {
				auto mons = SL::Screen_Capture::GetMonitors();
				std::cout << "Library is requesting the list of monitors to capture!" << std::endl;
				for (auto& m : mons) {
					std::cout << m << std::endl;
				}
				return mons;
			})
				->onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
				// std::cout << "Difference detected!  " << img.Bounds << std::endl;
				auto r = realcounter.fetch_add(1);
				auto s = std::to_string(r) + std::string("MONITORDIF_") + std::string(".jpg");
				auto size = Width(img) * Height(img) * sizeof(SL::Screen_Capture::ImageBGRA);
				// auto imgbuffer(std::make_unique<unsigned char[]>(size));
				// ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
				// tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
			})
				->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
				auto r = realcounter.fetch_add(1);
				auto s = std::to_string(r) + std::string("MONITORNEW_") + std::string(".jpg");

				// auto imgbuffer(std::make_unique<unsigned char[]>(size));
				// ExtractAndConvertToRGBA(img, imgbuffer.get(), size);
				// tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
				// tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
				if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >=
					1000) {
					std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
					onNewFramecounter = 0;
					onNewFramestart = std::chrono::high_resolution_clock::now();
				}
				onNewFramecounter += 1;
			})
				->onMouseChanged([&](const SL::Screen_Capture::Image* img, const SL::Screen_Capture::MousePoint& mousepoint) {
				auto r = realcounter.fetch_add(1);
				auto s = std::to_string(r) + std::string(" M") + std::string(".png");
				if (img) {/*
					std::cout << "New mouse coordinates  AND NEW Image received."
							  << " x= " << mousepoint.Position.x << " y= " << mousepoint.Position.y << std::endl;
					lodepng::encode(s, (unsigned char*)StartSrc(*img), Width(*img), Height(*img));*/
				}
				else {
					// std::cout << "New mouse coordinates received." << " x= " << point.x << " y= " << point.y << " The
					// mouse image is still the same
					// as the last" << std::endl;
				}
			})
				->start_capturing();

			framgrabber->setFrameChangeInterval(std::chrono::milliseconds(100));
			framgrabber->setMouseChangeInterval(std::chrono::milliseconds(100));

		}
		bool start_grabbing();
		bool stop_grabbing();

	}
}