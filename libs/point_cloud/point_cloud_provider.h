#pragma once
#include "point_cloud.h"
#include <future>
#include <memory>
#include <cgv/signal/signal.h>

namespace cgv {
namespace pointcloud {


	// interface definition
class point_cloud_provider
{
  public:
	virtual cgv::signal::signal<>& new_point_cloud_ready() = 0;
	virtual point_cloud get_point_cloud() = 0;
};

}
}
