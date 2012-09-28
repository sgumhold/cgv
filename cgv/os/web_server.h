#pragma once

#include "http_request.h"
#include "thread.h"

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// simple interface for a web server
class CGV_API web_server 
{
protected:
	unsigned int port;
	void* user_data;
	friend class web_server_provider;
public:
	/// create a web server that listens to the given port
	web_server(unsigned int _port = 80);
	/// reimplement to handle requests
	virtual void handle_request(http_request& request) = 0;
	/// start the web server (does never return)
	void start();
	/// can only be called from a different thread
	void stop();
	/// return the port to which the web server listens
	unsigned int get_port() const;
};

/// web server interface that runs in its own thread
class CGV_API web_server_thread : public thread, public web_server
{
public:
	/// create a web server that listens to the given port
	web_server_thread(unsigned int _port = 8080);
	/// calls the stop method of the web_server
	~web_server_thread();
	/// start the web server in a separate thread
	void start();
	/// reimplements the run method that simply starts the web server
	void run();
};

/// interface for a html_server provider
class CGV_API web_server_provider
{
protected:
	void*& ref_user_data(web_server* instance) const;
public:
	virtual void start_web_server(web_server* instance) = 0;
	virtual void stop_web_server(web_server* instance) = 0;
};

/// method to register a web server plugin
extern CGV_API void register_web_server_provider(web_server_provider* wsp);

/// template to facilitate registeration of a web server provider
template <class T>
struct web_server_provider_registration
{
	web_server_provider_registration() {
		register_web_server_provider(new T());
	}
};

	}
}

#include <cgv/config/lib_end.h>