#include "webserver.h"
#include <ctime>
// #include <process.h>
#include <iostream>
#include <string>
#include <map>
#include <sstream>


#include "UrlHelper.h"
#include "base64.h"


#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(disable:4996)
#endif

using namespace cgv::os;

webserver::request_func webserver::request_func_=0;

unsigned webserver::Request(void* _ws) {
	webserver* ws = (webserver*) _ws;
    
	socket_ptr sp = ws->sp;
  
	std::string line = sp->receive_line();
	if (line.empty()) {
		return 1;
	}

	http_request req;
	req.request = line;
	req.user_data = ws->user_data;
	if (line.find("GET") == 0) {
		req.method ="GET";
	}
	else if (line.find("POST") == 0) {
		req.method ="POST";
	}

	std::string path;
	std::map<std::string, std::string> params;

	size_t posStartPath = line.find_first_not_of(" ",3);

	SplitGetReq(line.substr(posStartPath), path, params);

	req.status     = "202 OK";
	req.sp= sp;
	req.path       = path;
	req.params     = params;

	static const std::string authorization   = "Authorization: Basic ";
	static const std::string accept          = "Accept: "             ;
	static const std::string accept_language = "Accept-Language: "    ;
	static const std::string accept_encoding = "Accept-Encoding: "    ;
	static const std::string user_agent      = "User-Agent: "         ;

	while(1) {
		line=sp->receive_line();
		//std::cout << "RECEIVED:" << line.c_str();
		std::cout.flush();

		if (line.empty()) break;

		unsigned int pos_cr_lf = line.find_first_of("\x0a\x0d");
		if (pos_cr_lf == 0) break;

		line = line.substr(0,pos_cr_lf);

		if (line.substr(0, authorization.size()) == authorization) {
			req.authentication_given = true;
			std::string encoded = line.substr(authorization.size());
			std::string decoded = base64_decode(encoded);

			unsigned int pos_colon = decoded.find(":");

			req.username = decoded.substr(0, pos_colon);
			req.password = decoded.substr(pos_colon+1 );
		}
		else if (line.substr(0, accept.size()) == accept) {
			req.accept = line.substr(accept.size());
		}
		else if (line.substr(0, accept_language.size()) == accept_language) {
			req.accept_language = line.substr(accept_language.size());
		}
		else if (line.substr(0, accept_encoding.size()) == accept_encoding) {
			req.accept_encoding = line.substr(accept_encoding.size());
		}
		else if (line.substr(0, user_agent.size()) == user_agent) {
			req.user_agent = line.substr(user_agent.size());
		}
	}

	request_func_(&req);

	std::stringstream str_str;
	str_str << req.answer.size();

	time_t ltime;
	time(&ltime);
	tm* gmt = gmtime(&ltime);

	static std::string const serverName = "cgv web server";

	char* asctime_remove_nl = asctime(gmt);
	asctime_remove_nl[24] = 0;

	sp->send_data("HTTP/1.1 ");

	if (!req.auth_realm.empty() ) {
		sp->send_line("401 Unauthorized");
		sp->send_data("WWW-Authenticate: Basic Realm=\"");
		sp->send_data(req.auth_realm);
		sp->send_line("\"");
	}
	else {
		sp->send_line(req.status);
	}
	sp->send_line(std::string("Date: ") + asctime_remove_nl + " GMT");
	sp->send_line(std::string("Server: ") +serverName);
	sp->send_line("Connection: close");
	sp->send_line("Content-Type: text/html; charset=ISO-8859-1");
	sp->send_line("Content-Length: " + str_str.str());
	sp->send_line("");
	sp->send_line(req.answer);
	sp->close();

	ws->thread_id = 0;
	return 0;
}

webserver::webserver(void* _user_data) {
	user_data = _user_data;
	thread_id = 0;
}

void webserver::start(unsigned int port_to_listen, request_func r)
{
	ssp = create_socket_server();
	ssp->bind_and_listen(port_to_listen,5);
	request_func_ = r;
	while (1) {
		socket_ptr _sp = ssp->wait_for_connection();
		if (_sp) {
			sp = _sp;
			unsigned ret;
			thread_id = _beginthreadex(0,0,Request,(void*) this,0,&ret);
		}
		else {
			return;
		}
	}
}

void webserver::stop()
{
	if (thread_id)
		_endthreadex(thread_id);
	sp.clear();
}

