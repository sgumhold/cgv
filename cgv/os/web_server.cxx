#include "web_server.h"

namespace cgv {
	namespace os {

web_server_provider*& ref_provider()
{
	static web_server_provider* wsp = 0;
	return wsp;
}

/// create a web server that listens to the given port
web_server::web_server(unsigned int _port)
{
	port = _port;
	user_data = 0;
}

/// create a web server that listens to the given port
web_server_thread::web_server_thread(unsigned int _port) : web_server(_port)
{
}


/// return the port to which the web server listens
unsigned int web_server::get_port() const
{
	return port;
}


void web_server::start()
{
	if (ref_provider()) 
		ref_provider()->start_web_server(this);
	else
		std::cerr << "no web server provider registered, please use the co_web plugin" << std::endl;
}

/// can only be called from a different thread
void web_server::stop()
{
	if (user_data && ref_provider()) 
		ref_provider()->stop_web_server(this);
	else
		std::cerr << "no web server provider registered, please use the co_web plugin" << std::endl;
}


/// start the html server in a separate thread
void web_server_thread::start()
{
	thread::start();
}

/// calls the stop method of the web_server
web_server_thread::~web_server_thread()
{
	thread::kill();
	web_server::stop();
}

/// reimplements the run method that simply starts the web server
void web_server_thread::run()
{
	web_server::start();
}


/// method to register a web server plugin
void register_web_server_provider(web_server_provider* wsp)
{
	ref_provider() = wsp;
}

void*& web_server_provider::ref_user_data(web_server* instance) const
{
	return instance->user_data;
}

	}
}
