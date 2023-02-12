#include <deque>
#include <cgv/base/register.h>
#include <cgv/signal/rebind.h>
#include <cgv/signal/signal.h>
#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/os/web_server.h>
#include <cgv/os/socket.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::os;
using namespace cgv::signal;

// function to generate a html page
void process_request(http_request* r);

/** example of a sever with two ways of communicating with the main thread:
    - by a signal named "event"
	- through a thread safe fifo accessed with the methods add_event and extract_event
*/
class my_server : public web_server_thread
{
protected:
	/**@name thread safe fifo based communication */
	//@{
	/// fifo used to store events from the http requests
	std::deque<int> fifo;
	/// mutex used to protext the fifo
	mutex m;
	/// thread safe adding of an event to the event fifo
	void add_event(int i) {
		m.lock();
		fifo.push_back(i);
		m.unlock();
	}
public:
	/// check whether the event fifo is empty
	bool empty() const { 
		return fifo.empty(); 
	}
	/// thread safe extraction of event from the event fifo, returns -1 if no event available
	int extract_element() {
		m.lock();
		int i=-1;
		if (!fifo.empty()) {
			i = fifo.front();
			fifo.pop_front();
		}
		m.unlock();
		return i;
	}
	//@}

	/// signal triggered by a http request
	signal<std::string> event;
	/// construct with port as argument
	my_server(unsigned int _port) : web_server_thread(_port) {}
	/// overloaded to handle http requests
	void handle_request(http_request& request)
	{
		/// generate to be returned html page and status
		process_request(&request);
		/// print request path to console
		std::cout << "request: " << request.request.c_str();
		std::cout.flush();
		/// analyze path and generate events
		if (request.path == "/red")
			add_event(0);
		else if (request.path == "/blue")
			add_event(1);
		/// lock main gui thread to allow calls relevant to the gui 
		get_gui_driver()->lock();
		/// emit event signal
		event(request.path);
		/// wake up the main gui thread to respond instantly to http request
		get_gui_driver()->wake();
		/// unlock gui thread
		get_gui_driver()->unlock();
	}
};

struct post_thread : public thread
{
	std::string address;
	std::string request;
	int port;
	post_thread(const std::string& _a, int _p, const std::string& _r)
	{
		address = _a;
		port = _p;
		request = _r;
	}
	void run()
	{
		std::cout << "server: " << address.c_str() << std::endl;
		socket_client_ptr scp = create_socket_client();
		if (!scp->connect(address, port)) {
			std::cout << "connection failed: " << scp->get_last_error().c_str() << std::endl;
			return;
		}
		std::cout << "connected" << std::endl;
		scp->send_line(request);
		scp->send_line("");
		// receive lines till a new line
		unsigned int content_length = 0;
		do {
			std::string line = scp->receive_line();
			if (line.empty())
				return;
			if (line == "\n")
				break;
			std::cout << line.c_str();
			if (line.substr(0,15) == "Content-Length:")
				content_length = atoi(line.substr(15).c_str());
			std::cout.flush();
		} while (1);

		if (content_length > 0) {
			std::string content = scp->receive_data(content_length);
			std::cout << "RECEIVED_CONTENT:\n_____________________________\n" << content.c_str() << std::endl;
		}
	}
};


/// simple gui and drawable controlled by my_server
class web_server_test : 
	public node,
	public provider,
	public drawable
{
public:
	/// pointer to the server
	my_server *server;
	/// to be used port number
	unsigned int port;
	/// a trigger event is registered in the event handler for the http request
	trigger trig;

	/// color index is identical to last event
	int color_idx;
	/// last path of http request to be shown in gui
	std::string path;
	/// address of external server
	std::string address;
	///
	std::string my_request;

	/// pointer to the gui buttons used to activate and deactive them
	button_ptr start_button, stop_button;

	/// default constructor
	web_server_test() : node("web server test")
	{
		//cgv::os::socket::enable_debug_output();
		port = 80;
		color_idx = 0;
		server = 0;
		address = "localhost";
		my_request = "GET / HTTP/1.1";
		connect(trig.shoot, this, &web_server_test::trigger_callback);
	}
	/// destruct server if it is still running
	~web_server_test() 
	{
		if (server)
			delete server;
	}

	/// this is called in the main gui thread once after each http request 
	void trigger_callback(double,double)
	{
		// update the path view in the gui
		find_view(path)->update();
		if (server) {
			// read out all events from the event fifo of my server
			while (!server->empty()) {
				color_idx = server->extract_element();
				// ensure redraw of the 3d view
				post_redraw();
			}
		}
	}
	/// called in the http request thread
	void on_event(std::string _path)
	{
		// store path in my own member
		path = _path;
		// activate trigger to be called in the main event
		trig.schedule_one_shot(0);
	}
	/// start button callback creates a server and connects the event signal
	void on_start()
	{
		// configure gui
		find_control(port)->set("active", false);
		start_button->set("active",false);
		stop_button->set("active",true);
		// create server
		server = new my_server(port);
		// connect event signal
		connect(server->event, this, &web_server_test::on_event);
		// start server thread
		server->start();
	}
	/// stop the server again
	void on_stop()
	{
		// destruct server, what deletes socket and terminates server thread
		delete server;
		server = 0;
		// configure gui
		find_control(port)->set("active", true);
		start_button->set("active",true);
		stop_button->set("active",false);
	}
	void on_post()
	{
		post_thread* pt = new post_thread(address,80,my_request);
		pt->start(true);
	}
	/// construct simple gui
	void create_gui()
	{
		add_view("path", path);
		add_control("port", port);
		start_button = add_button("start server");
		connect_copy(start_button->click, rebind(this, &web_server_test::on_start));
		stop_button = add_button("stop server", "active=false");
		connect_copy(stop_button->click, rebind(this, &web_server_test::on_stop));
		add_control("http address", address);
		add_control("request", my_request);
		connect_copy(add_button("post request")->click,
			rebind(this, &web_server_test::on_post));
	}
	/// draw a colored cube
	void draw(context& ctx)
	{
		glColor3f(color_idx == 0 ? 1.0f : 0.0f, 0, color_idx == 1 ? 1.0f : 0.0f);
		ctx.tesselate_unit_cube();
	}
	/// return type name
	std::string get_type_name() const 
	{ 
		return "web_server_test"; 
	}
};



/// function to generate a html page for the request
void process_request(http_request* r) 
{
	std::string title;
	std::string body;
	std::string bgcolor="#ffffff";
	std::string links =
		  "<p><a href='/red'>red</a> "
		  "<br><a href='/blue'>blue</a> "
		  "<br><a href='/'>back</a> "
	//		  "<br><a href='/form'>form</a> "
	//		  "<br><a href='/auth'>authentication example</a> [use <b>adp</b> as username and <b>gmbh</b> as password"
	//		  "<br><a href='/header'>show some HTTP header details</a> "
		  ;

	  if(r->path == "/") {
		title = "Web Server Example";
		body  = "<h1>Web Server Example</h1>"
				"I wonder what you're going to click"  + links;
	  }
	  else if (r->path == "/red") {
		bgcolor = "#ff0000";
		title   = "You chose to do something";
		body    = "<h1>You chose red</h1>" + links;
	  }
	  else if (r->path == "/blue") {
		bgcolor = "#4444ff";
		title   = "You chose blue";
		body    = "<h1>Blue</h1>" + links;
	  }
	  else if (r->path == "/form") {
		title   = "Fill a form";

		body    = "<h1>Fill a form</h1>";
		body   += "<form action='/form'>"
				  "<table>"
				  "<tr><td>Field 1</td><td><input name=field_1></td></tr>"
				  "<tr><td>Field 2</td><td><input name=field_2></td></tr>"
				  "<tr><td>Field 3</td><td><input name=field_3></td></tr>"
				  "</table>"
				  "<input type=submit></form>";


		for (std::map<std::string, std::string>::const_iterator i = r->params.begin();
			 i != r->params.end();
			 i++) {

		  body += "<br>" + i->first + " = " + i->second;
		}


		body += "<hr>" + links;

	  }
	  else if (r->path == "/auth") {
		if (r->authentication_given) {
		  if (r->username == "adp" && r->password == "gmbh") {
			 body = "<h1>Successfully authenticated</h1>" + links;
		  }
		  else {
			 body = "<h1>Wrong username or password</h1>" + links;
			 r->auth_realm = "Private Stuff";
		  }
		}
		else {
		  r->auth_realm = "Private Stuff";
		}
	  }
	  else if (r->path == "/header") {
		title   = "some HTTP header details";
		body    = std::string ("<table>")                                   +
				  "<tr><td>Accept:</td><td>"          + r->accept          + "</td></tr>" +
				  "<tr><td>Accept-Encoding:</td><td>" + r->accept_encoding + "</td></tr>" +
				  "<tr><td>Accept-Language:</td><td>" + r->accept_language + "</td></tr>" +
				  "<tr><td>User-Agent:</td><td>"      + r->user_agent      + "</td></tr>" +
				  "</table>"                                                +
				  links;
	  }
	  else {
		r->status = "404 Not Found";
		title      = "Wrong URL";
		body       = "<h1>Wrong URL</h1>";
		body      += "Path is : &gt;" + r->path + "&lt;"; 
	  }

	  r->answer  = "<html><head><title>";
	  r->answer += title;
	  r->answer += "</title></head><body bgcolor='" + bgcolor + "'>";
	  r->answer += body;
	  r->answer += "</body></html>";
}

/// register web server test
factory_registration<web_server_test> web_server_test_fac("New/Demo/Web Server", 'W', true);

