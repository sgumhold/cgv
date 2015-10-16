#pragma once

#include <string>
#include <cgv/data/ref_ptr.h>

#include "lib_begin.h"

namespace cgv {
	namespace os {

/** base class for all sockets */
class CGV_API socket : public data::ref_counted
{
protected:
	static bool show_debug_output;
	static int  nr_of_sockets;
	static bool begin();
	static void end();
	friend class socket_server;
	friend class socket_select;
	/// hides constructor from user
	socket();
	/// construct from existing socket identifier
	socket(size_t _id);
	/// store platform dependent reference to socket
	size_t user_data;
	/// store the last error
	mutable std::string last_error;
	/// convenience function to set last error and print debug info. The method always returns false.
	bool set_last_error(const char* location, const std::string& text = "") const;
public:
	/// enables or disables (default) debug output for all socket commands
	static void enable_debug_output(bool enable = true);
	/// virtual destructor
	virtual ~socket();
	/// returns the last error
	std::string get_last_error() const;
	/// return whether data has arrived
	bool is_data_pending() const;
	/// return the number of data bytes that have been arrived at the socket or -1 if socket is not connected
	int get_nr_of_arrived_bytes() const;
	/// receive data up to the next newline excluding the newline char
	std::string receive_line();
	/// receive all pending data or if nr_of_bytes is larger than 0, exactly nr_of_bytes
	std::string receive_data(unsigned int nr_of_bytes = 0);
	/// extends line by newline and send as data
	bool send_line(const std::string& content);
	/// send the data in the string
	bool send_data(const std::string&);
	/// close the socket
	bool close();
};

/// reference counted pointer to socket
typedef data::ref_ptr<socket> socket_ptr;

class CGV_API socket_client;

/// reference counted pointer to socket_client
typedef data::ref_ptr<socket_client,true> socket_client_ptr;

/// client socket
class CGV_API socket_client : public socket 
{
protected:
	friend CGV_API socket_client_ptr create_socket_client();
	/// hide from direct use
	socket_client();
public:
	/// connect to given port of given host, if the socket is already connected, close this connection first
	bool connect(const std::string& host, int port);
};

/// this is the only way to create a socket_client as a reference counted pointer
extern CGV_API socket_client_ptr create_socket_client();


class CGV_API socket_server;

/// reference counted pointer to socket_server
typedef data::ref_ptr<socket_server,true> socket_server_ptr;

/// socket server allows to be connected to
class CGV_API socket_server : public socket 
{
protected:
	friend CGV_API socket_server_ptr create_socket_server();
	/// hide from direct use
	socket_server();
public:
	/// bind and listen to given port with a queue for pending connections of the given size
	bool bind_and_listen(int port, int max_nr_pending_connections);
	/// if no connection is pending, block thread and wait for next incoming connection
	socket_ptr wait_for_connection();
	/// check if a new connection is in the connection queue and return this or an empty connection pointer
	socket_ptr check_for_connection();
};

/// this is the only way to create a socket_server as a reference counted pointer
extern CGV_API socket_server_ptr create_socket_server();

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<cgv::os::socket_ptr>;
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<cgv::os::socket_client_ptr>;
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<cgv::os::socket_server_ptr>;
#endif

	}
}

#include <cgv/config/lib_end.h>
