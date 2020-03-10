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

namespace rgbd {

	rgbd_spider::rgbd_spider() : capture_thread_running(false),serial(""), mesh_changed(false), capture_thread(nullptr){
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
		static const unsigned streams_avaiable = IS_COLOR | IS_MESH;
		return (~(~is | streams_avaiable)) == 0;
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
			stream_formats.push_back(stream_format(info->textureSizeY, info->textureSizeX, PF_RGB, fps, 24));
		}
		else if (is & IS_MESH) {
			stream_formats.push_back(stream_format(1, 1, PF_POINTS_AND_TRIANGLES, fps, 8));
		}
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
			stream_formats.push_back(color_stream = stream_format(info->textureSizeY, info->textureSizeX, PF_RGB, fps, 24));
		}
		if (is && IS_MESH) {
			stream_formats.push_back(mesh_stream = stream_format(0, 0, PF_POINTS_AND_TRIANGLES, fps, 8));
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
			if (format.pixel_format == PF_RGB) {
				color_stream = format;
			}
			else if (format.pixel_format == PF_POINTS_AND_TRIANGLES) {
				mesh_stream = format;
			}
		}

		if (scanner->createFrameProcessor(&frame_processor) != asdk::ErrorCode_OK) {
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
				frames = new_frame;
				TRef<asdk::IFrameMesh> new_mesh;
				auto ec = frame_processor->reconstructAndTexturizeMesh(&new_mesh, frames);
				if (ec == asdk::ErrorCode_OK) {
					mesh_changed = true;
					mesh = new_mesh;
				}
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

		TRef<asdk::IImage> image = nullptr;
		//lock frames
		std::lock_guard<std::mutex> guard(frames_protection);

		if (frames == nullptr) {
			return false;
		}


		const asdk::TimeStamp* t = frames->getCaptureTimeStamp();

		if (is == IS_COLOR) {
			//capture texture
			if (t->microSeconds == last_color_frame_time.microSeconds && t->seconds == last_color_frame_time.seconds) return false;
			last_color_frame_time = *t;

			static_cast<frame_format&>(frame) = color_stream;
			frame.time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			
			scanner->convertTextureFull(&image, frames->getTexture());
			image->bgr2rgb();

			color_stream.compute_buffer_size();
			if (frame.frame_data.size() != color_stream.buffer_size) {
				frame.frame_data.resize(color_stream.buffer_size);
			}
			memcpy(frame.frame_data.data(), image->getPointer(), color_stream.buffer_size);
			return true;
		}
		else if (is == IS_MESH) {
			if (!mesh_changed) {
				return false;
			}
			//if (t->microSeconds == last_mesh_frame_time.microSeconds && t->seconds == last_mesh_frame_time.seconds) return false;
			//last_mesh_frame_time = *t;

			//get points and triangles
			mesh_changed = false;
			TRef<asdk::IArrayPoint3F> points = mesh->getPoints();
			TRef<asdk::IArrayIndexTriplet> triangles = mesh->getTriangles();
			TRef<asdk::IArrayUVCoordinates> uv_coordinates = mesh->getUVCoordinates();
			if (!points || !triangles || !uv_coordinates) {
				cerr << "rgbd_spider::get_frame : reconstructed mesh is empty!\n";
				return false;
			}

			uint32_t points_size = points->getSize();
			uint32_t uv_cooordinates_size = uv_coordinates->getSize();
			uint32_t triangles_size = triangles->getSize();

			//write frame meta data
			static_cast<frame_format&>(frame) = mesh_stream;
			frame.time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
			frame.height = 1;
			frame.width = 3*sizeof(uint32_t) + points_size*sizeof(asdk::Point3F) + triangles_size *sizeof(asdk::IndexTriplet) + uv_cooordinates_size*sizeof(asdk::UVCoordinates);
			frame.compute_buffer_size();
			frame.pixel_format = PF_POINTS_AND_TRIANGLES;
			if (frame.frame_data.size() != frame.buffer_size) {
				frame.frame_data.resize(frame.buffer_size);
			}
			
			//copy data to frame
			//format point_count|point_0,...,point_N|triangle_count|triangle_triplet_0,...,triangle_triplet_M
			
			//stream begins with the amount of points in the mesh
			memcpy(frame.frame_data.data(), &points_size, sizeof(uint32_t));
			uint32_t offset = sizeof(uint32_t);
			//then the Points follow 
			memcpy(frame.frame_data.data()+offset, points->getPointer(), points_size*sizeof(asdk::Point3F));
			offset += points_size * sizeof(asdk::Point3F);
			//the point data ends before points_size * sizeof(asdk::Point3F)+ sizeof(uint32_t)
			memcpy(frame.frame_data.data()+offset, &triangles_size, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			//copy triangle data
			memcpy(frame.frame_data.data()+offset, triangles->getPointer(), triangles_size * sizeof(asdk::IndexTriplet));
			offset += triangles_size * sizeof(asdk::IndexTriplet);
			//texture information
			memcpy(frame.frame_data.data()+offset, &uv_cooordinates_size, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(frame.frame_data.data() + offset, uv_coordinates->getPointer(), uv_cooordinates_size * sizeof(asdk::UVCoordinates));
			return true;
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