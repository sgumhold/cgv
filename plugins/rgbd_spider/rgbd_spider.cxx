#include <iostream>
#include <chrono>
#include <cgv/utils/convert.h>
//spider includes
#include <artec/sdk/capturing/IScanner.h>
#include <artec/sdk/capturing/IArrayScannerId.h>
#include <artec/sdk/capturing/IFrameProcessor.h>
#include <artec/sdk/base/BaseSdkDefines.h>
#include <artec/sdk/base/Log.h>
#include <artec/sdk/base/io/ObjIO.h>
#include <artec/sdk/base/IFrameMesh.h>



#include "rgbd_spider.h"

using namespace std;
using namespace chrono;

namespace asdk {
	using namespace artec::sdk::base;
	using namespace artec::sdk::capturing;
};
using asdk::TRef;
using asdk::TArrayRef;

namespace rgbd {

	rgbd_spider::rgbd_spider() : capture_thread_running(false),serial("") {
	}

	rgbd_spider::~rgbd_spider() {
		if (capture_thread) {
			capture_thread->join();
			delete capture_thread;
		}
	}

	bool rgbd_spider::attach(const std::string& serial)
	{
		if (is_attached()) {
			detach();
		}
		

		asdk::IArrayScannerId* scannerList;

		asdk::ErrorCode ec = asdk::enumerateScanners(&scannerList);

		if (ec != asdk::ErrorCode_OK)
		{
			std::cout << "rgbd_spider_driver: something went wrong\n";
			return 0;
		}
		asdk::ScannerId id;
		//const asdk::ScannerId* id = scanner->getId();
		int list_size = scannerList->getSize();
		//find the camera with the same id
		for (int i = 0; i < scannerList->getSize();++i) {
			asdk::ScannerId &scannerId = scannerList->getPointer()[i];
			if (cgv::utils::wstr2str(scannerId.serial).compare(serial) == 0) {
				asdk::ErrorCode ec = asdk::createScanner(&scanner, &scannerId);
				if (ec != asdk::ErrorCode_OK)
				{
					std::cerr << "rgbd_spider::attach: errors ocurred while creating scanner object for scanner with serial " << serial << endl;
					scanner.release();
					return false;
				}
				this->serial = serial;
				return true;
			}
		}
		return false;
	}

	bool rgbd_spider::is_attached() const
	{
		return serial != "";
	}

	bool rgbd_spider::detach()
	{
		if (this->is_attached()) {
			scanner.release();
			serial = "";
			return true;
		}
		return false;
	}
	
	bool rgbd_spider::check_input_stream_configuration(InputStreams is) const
	{
		static const unsigned streams_avaiable = IS_COLOR;
		return (~(~is | streams_avaiable)) == 0;
	}

	bool rgbd_spider::start_device(InputStreams is, std::vector<stream_format>& stream_formats)
	{
		if (!check_input_stream_configuration(is)) {
			cerr << "rgbd_spider::start_device : invalid stream configuration\n";
			return false;
		}
		if (!is_attached()) {
			cerr << "rgbd_spider::start_device : tried to start unattached device\n";
			return false;
		}
		if (is_running()) {
			return true;
		}
		


		//query stream configuration
		int fps = scanner->getFPS();
		const asdk::ScannerInfo* info = scanner->getInfo();
		
		if (is && IS_COLOR) {
			stream_formats.push_back(color_stream = stream_format(info->textureSizeX, info->textureSizeY, PF_BAYER, fps, 8));
		}
		return this->start_device(stream_formats);
	}

	bool rgbd_spider::start_device(const std::vector<stream_format>& stream_formats)
	{
		if (!is_attached()) {
			cerr << "rgbd_spider::start_device : tried to start unattached device\n";
			return false;
		}
		if (is_running()) {
			return true;
		}

		for (auto format : stream_formats) {
			if (format.pixel_format == PF_BAYER) {
				color_stream = format;
			}
		}

		auto ec = scanner->createFrameProcessor(&frame_processor);
		if (ec != asdk::ErrorCode_OK) {
			cerr << "rgbd_spider::start_device : failed to create the frame processor\n";
		}

		capture_thread = new std::thread(&rgbd_spider::capture_frames,this);

		return true;
	}

	bool rgbd_spider::stop_device()
	{
		capture_thread_running = false;
		capture_thread->join();
		delete capture_thread;
		capture_thread = nullptr;
		return true;
	}

	bool rgbd_spider::is_running() const
	{
		return capture_thread_running;
	}

	void rgbd_spider::capture_frames() {
		if (capture_thread_running) {
			cerr << "rgbd_spider::capture_frames: a capture thread is allready running\n";
			return;
		}
		capture_thread_running = true;
		while (capture_thread_running) {
			if (!is_attached()) {
				cerr << "rgbd_spider::capture_frames: tried capturing frames from an unattached device\n";
			}
			//capture new frame
			TRef<asdk::IFrame> new_frame;
			auto ec = scanner->capture(&new_frame, true);
			if (ec == asdk::ErrorCode_OK) {
				std::lock_guard<std::mutex> guard(frames_protection);
				//copy buffer
				frames.release();
				frames = new_frame;
			}
		}
	}

	bool rgbd_spider::get_frame(InputStreams is, frame_type& frame, int timeOut)
	{
		if (!is_running()) {
			cerr << "rgbd_spider::get_frame called on device that is not running" << endl;
			return false;
		}
		if (!check_input_stream_configuration(is)) {
			cerr << "rgbd_spider::get_frame called with an invalid input stream configuration" << endl;
			return false;
		}
		if (!capture_thread_running) {
			return false;
		}

		const asdk::IImage* image = nullptr;
		//lock frames
		std::lock_guard<std::mutex> guard(frames_protection);

		if (frames == nullptr) {
			return false;
		}

		//IArrayPoint3F* points = frames.get_p

		

		if (is == IS_COLOR) {
			asdk::TimeStamp t = *(frames->getCaptureTimeStamp());
			if (t.microSeconds == last_color_frame_time.microSeconds && t.seconds == last_color_frame_time.seconds) return false;
			image = frames->getTexture();
			last_color_frame_time = t;
		
			static_cast<frame_format&>(frame) = color_stream;
			frame.time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			color_stream.compute_buffer_size();
			if (frame.frame_data.size() != color_stream.buffer_size) {
				frame.frame_data.resize(color_stream.buffer_size);
			}
			auto* point = image->getPointer();
			memcpy(frame.frame_data.data(), image->getPointer(), color_stream.buffer_size);
			return true;
		}
		else if (false) {
			//capture triangle mesh
			TRef<asdk::IFrameMesh> mesh;
			auto ec = frame_processor->reconstructMesh(&mesh,frames);
			if (ec != asdk::ErrorCode_OK) {
				cerr << "rgbd_spider::get_frame : failed to reconstruct mesh\n";
				return false;
			}
			//get point cloud
			asdk::IArrayPoint3F* points = mesh->getPoints();
			size_t points_size = points->getSize();
			asdk::IArrayIndexTriplet* triangles = mesh->getTriangles();
			static_cast<frame_format&>(frame) = mesh_stream;
			//copy pointcloud to frame
			const asdk::TimeStamp* time_stamp = frames->getCaptureTimeStamp();
			//frame.time = ((double)time_stamp->seconds) + ((double)time_stamp->microSeconds * 0.000001);
			frame.time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			color_stream.compute_buffer_size();
			frame.height = 1;
			frame.width = sizeof(size_t) + points->getSize()*sizeof(asdk::IArrayPoint3F) + triangles->getSize()*sizeof(asdk::IArrayIndexTriplet);
			if (frame.frame_data.size() != color_stream.buffer_size) {
				frame.frame_data.resize(color_stream.buffer_size);
			}
			//point_count|point_0,...,point_N|triangle_triplet_0,...,triangle_triplet_N
			memcpy(frame.frame_data.data(), &points_size, sizeof(size_t));
			size_t offset = sizeof(size_t);
			memcpy(frame.frame_data.data()+offset, points, points_size*sizeof(asdk::IArrayPoint3F));
			offset += points_size * sizeof(asdk::IArrayPoint3F);
			memcpy(frame.frame_data.data()+offset, triangles, triangles->getSize() * sizeof(asdk::IArrayIndexTriplet));
		}

		
		return false;
	}

	void rgbd_spider::map_color_to_depth(const frame_type& depth_frame, const frame_type& color_frame, frame_type& warped_color_frame) const
	{

	}

	bool rgbd_spider::map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		return false;
	}

	void rgbd_spider::query_stream_formats(InputStreams is, std::vector<stream_format>& stream_formats) const
	{
		if (!is_attached()) {
			std::cerr << "rgbd_realsense::query_stream_formats:  device is not attached!\n";
			return;
		}

		//query stream configuration
		int fps = scanner->getFPS();
		const asdk::ScannerInfo* info = scanner->getInfo();

		if (is & IS_COLOR) {
			stream_formats.push_back(stream_format(info->textureSizeX, info->textureSizeY, PF_BAYER, fps, 8));
		}
	}


	rgbd_spider_driver::rgbd_spider_driver() {
		
	}

	rgbd_spider_driver::~rgbd_spider_driver() {
		
	}

	unsigned rgbd_spider_driver::get_nr_devices() {
		asdk::IArrayScannerId* scannerList;

		asdk::ErrorCode ec = asdk::enumerateScanners(&scannerList);

		if (ec != asdk::ErrorCode_OK)
		{
			std::cout << "rgbd_spider_driver: something went wrong\n";
			return 0;
		}
		return scannerList->getSize();
	}

	std::string rgbd_spider_driver::get_serial(int i) {
		asdk::IArrayScannerId* scannerList;
		
		asdk::ErrorCode ec = asdk::enumerateScanners(&scannerList);

		if (ec != asdk::ErrorCode_OK)
		{
			std::cout << "rgbd_spider_driver: something went wrong\n";
			return "";
		}
		if (i >= scannerList->getSize()) {
			std::cout << "rgbd_spider_driver::get_serial scannerList index out of bounds\n";
			return "";
		}
		return cgv::utils::wstr2str(scannerList->getPointer()[i].serial);
	}

	rgbd_device* rgbd_spider_driver::create_rgbd_device() {
		return new rgbd_spider();
	}
}
#include "lib_begin.h"

extern CGV_API rgbd::driver_registration<rgbd::rgbd_spider_driver> rgbd_spider_driver_registration("spider_driver");