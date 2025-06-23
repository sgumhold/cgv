#pragma once

#include "library.h"

#include <CesiumAsync/ITaskProcessor.h>
#include <thread>

class CESIUM_TILES_API SimpleTaskProcessor : public CesiumAsync::ITaskProcessor
{
public:
	void startTask(std::function<void()> f) override
	{
		// Run the task in a background thread
		std::thread([f]() { f(); }).detach();
		
	}
};