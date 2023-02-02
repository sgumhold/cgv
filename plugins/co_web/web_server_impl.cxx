#include <cgv/os/web_server.h>
#include <3rd/webserver/webserver.h>

void request_function(webserver::http_request* r)
{
	cgv::os::web_server* ws = (cgv::os::web_server*) r->user_data;
	ws->handle_request(*r);
}

#ifdef CGV_OS_WEB_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

struct CGV_API web_server_provider_impl : public cgv::os::web_server_provider
{
	void start_web_server(cgv::os::web_server* instance);
	void stop_web_server(cgv::os::web_server* instance);
};

#include <cgv/config/lib_end.h>

void web_server_provider_impl::start_web_server(cgv::os::web_server* instance)
{
	webserver* w = new webserver(instance);
	ref_user_data(instance) = w;
	w->start(instance->get_port(), request_function);
}

void web_server_provider_impl::stop_web_server(cgv::os::web_server* instance)
{
	webserver* w = (webserver*)ref_user_data(instance);
	w->stop();
	delete w;
}

cgv::os::web_server_provider_registration<web_server_provider_impl> web_server_impl_registration;


