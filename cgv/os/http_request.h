#pragma once

#include <string>
#include <map>

namespace cgv {
	namespace os {

/// structure that contains all input and output parameters of a http request
struct http_request
{
	/// this is the complete request received by the server
	std::string request;
	/**@name information of request split into fields*/
	//@{
	///
	std::string                        method;
	///
	std::string                        path;
	///
	std::map<std::string, std::string> params;
	/** authentication_given is true when the user has entered a username and password.
	    These can then be read from username and password */
	bool authentication_given;
	/// user name of authentification
	std::string username;
	/// password of authentification
	std::string password;
	//@}

	std::string accept;
	std::string accept_language;
	std::string accept_encoding;
	std::string user_agent;

	/**@name return values*/
	//@{
	/** status: used to transmit server's error status, such as
	 - 202 OK (this is set by default)
	 -  404 Not Found 
	 and so on. */
	std::string status;
	/** auth_realm: allows to set the basic realm for an authentication,
	    no need to additionally set status if set */
	std::string auth_realm;
	/// set this member to the html page to be returned
	std::string answer;
};

	}
}