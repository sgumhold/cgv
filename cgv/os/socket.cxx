#include <errno.h>
#ifdef WIN32
#pragma warning(disable:4996)
#include <WinSock2.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#endif

#include <algorithm>
#include <iostream>
#include "socket.h"
#include "mutex.h"

#ifndef WIN32
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define closesocket(s) ::close(s)
#endif

#ifdef WIN32
		typedef int socklen_t;
#endif


using namespace std;

namespace cgv {
	namespace os {

mutex& ref_show_mutex()
{
	static mutex sm;
	return sm;
}

int socket::nr_of_sockets= 0;
bool socket::show_debug_output = false;

/// enables or disables (default) debug output for all socket commands
void socket::enable_debug_output(bool enable)
{
	show_debug_output = enable;
}


bool socket::begin() 
{
	if (!nr_of_sockets) {
#ifdef WIN32
		WSADATA info;
		if (WSAStartup(MAKEWORD(2,0), &info)) {
			if (show_debug_output) {
				ref_show_mutex().lock();
				std::cerr << "could not start up windows socket dll" << std::endl;
				ref_show_mutex().unlock();
			}
			return false;
		}
		else
#endif
			if (show_debug_output) {
				ref_show_mutex().lock();
				std::cout << "successfully start up windows socket dll" << std::endl;
				ref_show_mutex().unlock();
			}
	}
	++nr_of_sockets;
	return true;
}

void socket::end() 
{
	if (--nr_of_sockets == 0) {
#ifdef WIN32
		WSACleanup();
#endif
		if (show_debug_output) {
			ref_show_mutex().lock();
			std::cout << "cleaned up windows socket dll" << std::endl;
			ref_show_mutex().unlock();
		}
	}

}

socket::socket() : user_data(0) 
{
}

/// construct from existing socket identifier
socket::socket(unsigned int _id) : user_data(_id)
{
}

bool socket::set_last_error(const char* location, const std::string& text) const
{
	if (text.empty()) {
		char buffer[1024];
		socklen_t len;
		getsockopt(user_data, SOL_SOCKET, SO_ERROR, buffer, &len);
		last_error = buffer;
	}
	else
		last_error = text;
	if (show_debug_output) {
		ref_show_mutex().lock();
		std::cerr << "socket error: ";
		if (user_data)
			std::cerr << '(' << user_data << ") ";
		std::cerr << location << " - " << last_error.c_str();
		ref_show_mutex().unlock();
	}
	return false;
}

socket::~socket() 
{
	if (user_data)
		close();
}

/// returns the last error
std::string socket::get_last_error() const
{
	return last_error;
}

/// return whether data has arrived
bool socket::is_data_pending() const
{
	fd_set set;
	FD_ZERO(&set);
	FD_SET(user_data, &set);
	timeval t;
	timerclear(&t);
	return select(0, &set, 0, 0, &t) == 1;
}

/// return the number of data bytes that have been arrived at the socket
int socket::get_nr_of_arrived_bytes() const
{
	if (!user_data) {
		set_last_error("get_nr_of_arrived_bytes", "socket not connected");
		return -1;
	}
	unsigned long arg;
#ifdef WIN32
	if (ioctlsocket(user_data, FIONREAD, &arg) != 0) {
		set_last_error("get_nr_of_arrived_bytes");
		return -1;
	}
	last_error.clear();
#else
	arg = -1;
	std::cerr << "get_nr_of_arrived_bytes not implemented!!!" << std::endl;
#endif
	return arg;
}

std::string socket::receive_data(unsigned int nr_of_bytes) 
{
	std::string ret;
	char buf[1024];
	last_error.clear();

	if (nr_of_bytes == 0) {
		while (is_data_pending()) {
			int received_nr_of_bytes = recv (user_data, buf, 1024, 0);
			if (received_nr_of_bytes <= 0) {
				set_last_error("receive_data", received_nr_of_bytes == SOCKET_ERROR ? "" : "connection closed");
				break;
			}
			ret += std::string(buf,received_nr_of_bytes);
		}
	}
	else {
		while (nr_of_bytes > 0) {
			int received_nr_of_bytes = recv(user_data, buf, min(1024, (int)nr_of_bytes), 0);
			if (received_nr_of_bytes <= 0) {
				set_last_error("receive_data", received_nr_of_bytes == SOCKET_ERROR ? "" : "connection closed");
				break;
			}
			ret += std::string(buf,received_nr_of_bytes);
			nr_of_bytes -= received_nr_of_bytes;
		}

	}
	return ret;
}

std::string socket::receive_line() 
{
	std::string ret;
	while (true) {
		char r;
		int result = recv(user_data, &r, 1, 0);
		if (result <= 0) {
			set_last_error("receive_line", result == SOCKET_ERROR ? "" : "connection closed");
			return "";
		}
		ret += r;
		if (r == '\n') {
			last_error.clear();
			if (show_debug_output) {
				ref_show_mutex().lock();
				std::cout << "received line: " << ret.c_str(); 
				std::cout.flush();
				ref_show_mutex().unlock();
			}
			return ret;
		}
	}
}

bool socket::send_line(const std::string& s) 
{
	return send_data(s+'\n');
}

bool socket::send_data(const std::string& s)
{
	int nr_bytes = s.length();
	const char* buf = s.c_str();
	do {
		int nr_bytes_sent = send(user_data,buf,nr_bytes,0);
		if (nr_bytes_sent <= 0)
			return set_last_error("send_data/line", nr_bytes_sent == SOCKET_ERROR ? "" : "connection closed");
		nr_bytes -= nr_bytes_sent;
		buf += nr_bytes_sent;
	} while (nr_bytes > 0);
	last_error.clear();
	return true;
}

bool socket::close() 
{
#ifdef WIN32
	shutdown(user_data, SD_BOTH);
	int result = closesocket(user_data);
#else
	shutdown(user_data, SHUT_RDWR);
	int result = closesocket(user_data);
#endif
	if (result == 0) {
		if (show_debug_output) {
			ref_show_mutex().lock();
			std::cout << "socket(" << user_data << ") closed successfully" << std::endl;
			ref_show_mutex().unlock();
		}
		last_error.clear();
	}
	else
		set_last_error("close");

	user_data = 0;
	end();
	return result == 0;
}

socket_client::socket_client()
{
}

bool socket_client::connect(const std::string& host, int port)
{
	if (!begin())
		return set_last_error("connect", "could not initialize os specific socket shared library");
	user_data = ::socket(AF_INET,SOCK_STREAM,0);
	if (user_data == INVALID_SOCKET) {
		user_data = 0;
		return set_last_error("connect");
	}
	std::string error;
	hostent *he;
	if ((he = gethostbyname(host.c_str())) == 0)
		return set_last_error("connect");
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((in_addr *)he->h_addr);
	memset(&(addr.sin_zero), 0, 8); 
	if (::connect(user_data, (sockaddr *) &addr, sizeof(sockaddr)))
		return set_last_error("connect");
	if (show_debug_output) {
		ref_show_mutex().lock();
		std::cout << "successfully connected socket_client(" << user_data << ")" << std::endl;
		ref_show_mutex().unlock();
	}
	return true;
}

socket_client_ptr create_socket_client()
{
	return socket_client_ptr(new socket_client());
}

socket_server::socket_server()
{
}

bool socket_server::bind_and_listen(int port, int max_nr_connections) 
{
	if (!begin())
		return set_last_error("bind_and_listen", "could not initialize os specific socket shared library");
	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = PF_INET;             
	sa.sin_port = htons(port);          
	user_data = ::socket(AF_INET, SOCK_STREAM, 0);
	if (user_data == INVALID_SOCKET) {
		user_data = 0;
		return set_last_error("bind_and_listen", "could not create socket");
	}
	/* bind the socket to the internet address */
	if (bind(user_data, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		set_last_error("bind_and_listen");
		closesocket((SOCKET)user_data);
		user_data = 0;
		return false;
	}
	if (listen((SOCKET)user_data, max_nr_connections) != 0)
		return set_last_error("bind_and_listen");
	last_error.clear();
	if (show_debug_output) {
		ref_show_mutex().lock();
		std::cout << "successfully bound socket_server(" << user_data << ") to port " << port << std::endl;
		ref_show_mutex().unlock();
	}
	return true;
}

/// if no connection is pending, block thread and wait for next incoming connection
socket_ptr socket_server::wait_for_connection()
{
	if (!user_data) {
		set_last_error("wait_for_connection", "attempt to wait for connection of socket server that does not listen to port");
		return socket_ptr();
	}
#ifdef WIN32
	u_long arg = 0;
	ioctlsocket(user_data, FIONBIO, &arg);
#else
	int flags = fcntl(user_data, F_GETFD) & (~O_NONBLOCK);
	fcntl(user_data, F_SETFD, flags);
#endif
	SOCKET new_sock = ::accept(user_data, 0, 0);
	if (new_sock == INVALID_SOCKET) {
		set_last_error("wait_for_connection");
		return socket_ptr();
	}
	last_error.clear();
	if (show_debug_output) {
		ref_show_mutex().lock();
		std::cout << "received new connection on socket_server(" << user_data << "): " << new_sock << std::endl;
		ref_show_mutex().unlock();
	}
	begin();
	return socket_ptr(new socket(new_sock));
}

/// check if a new connection is in the connection queue and return this or an empty connection pointer
socket_ptr socket_server::check_for_connection()
{
	if (!user_data) {
		set_last_error("check_for_connection", "attempt to check for connection of socket server that does not listen to port");
		return socket_ptr();
	}
#ifdef WIN32
	u_long arg = 1;
	ioctlsocket(user_data, FIONBIO, &arg);
#else
	fcntl(user_data, F_SETFD, O_NONBLOCK);
#endif
	SOCKET new_sock = ::accept(user_data, 0, 0);
	if (new_sock == INVALID_SOCKET) {
#ifdef WIN32
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
			last_error.clear();
			return socket_ptr();
		}
		set_last_error("check_for_connection");
		return socket_ptr();
	}
	last_error.clear();
	if (show_debug_output) {
		ref_show_mutex().lock();
		std::cout << "received new connection on socket_server(" << user_data << "): " << new_sock << std::endl;
		ref_show_mutex().unlock();
	}
	begin();
	return socket_ptr(new socket(new_sock));
}

/// this is the only way to create a socket_server as a reference counted pointer
socket_server_ptr create_socket_server()
{
	return socket_server_ptr(new socket_server);
}


	}
}

