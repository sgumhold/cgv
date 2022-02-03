#include "point_cloud.h"
#include <future>
#include <memory>

namespace cgv {
namespace pointcloud {


	// interface definition
class point_cloud_provider : public cgv::render::render_types
{
  public:
	virtual point_cloud get_point_cloud() = 0;
	//virtual std::shared_ptr<std::future<point_cloud>> get_point_cloud_future() = 0;
};

}
}
