#pragma once

#include <httplib.h>
//#include <3rd/cpp-httplib/httplib.h>
#include <string>
#include "Config.h"

/// <summary>
/// Class to construct an overpass API query and fetch the response
/// </summary>
class OSMDataLoader
{
public:
	MAPTILES_API OSMDataLoader(double latMin, double lonMin, double latMax, double lonMax);
	MAPTILES_API void FetchOSMWays();
	MAPTILES_API std::string& GetResponse() { return m_result->body; }
	MAPTILES_API int GetErrorStatus() { return (int)m_result.error(); }
	MAPTILES_API int GetHTTPStatus() { return m_result->status; }

private:
	std::string m_url;
	std::string m_query;
	double m_latMin, m_lonMin, m_latMax, m_lonMax;
	httplib::Result m_result;
};