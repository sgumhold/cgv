#pragma once

#include <string>
#include <map>
#include <cgv/os/http_request.h>
#include <cgv/os/socket.h>

#ifdef WEBSERVER_EXPORT
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

class CGV_API webserver 
{
public:
	/// extend request by socket and user data
	struct http_request : public cgv::os::http_request {
		http_request()
		{
			authentication_given = false;
			user_data  = 0;
		}
		/// pointer to the socket used to respond to the request
		cgv::os::socket_ptr sp;
		/// user data passed in the constructor to the webserver
		void* user_data;
	};

    typedef   void (*request_func) (http_request*);
	webserver(void* _user_data);
	void start(unsigned int port_to_listen, request_func);
	void stop();
protected:
	cgv::os::socket_server_ptr ssp;
	cgv::os::socket_ptr sp;
	void* user_data;
	uintptr_t thread_id;
  private:
    static unsigned __stdcall Request(void*);
    static request_func request_func_;
};

#include <cgv/config/lib_end.h>
