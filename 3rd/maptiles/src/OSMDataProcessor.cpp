#include "OSMDataProcessor.h"

#include <tinyxml2.h>
#include <mapbox/earcut.hpp>

#include <algorithm>
#include <sstream>


OSMDataProcessor::OSMDataProcessor(OSMDataLoader& loader)
{
	Parse(loader.GetResponse());
    ExtractTileGeometry();
}

// Extracts the OSMNodes and OSMWays from an overpass api response and stores them
void OSMDataProcessor::Parse(std::string& response)
{
	tinyxml2::XMLDocument doc;
	doc.Parse(response.c_str());

    tinyxml2::XMLElement* element = doc.FirstChildElement("osm");

    for (tinyxml2::XMLElement* OSMElement = element->FirstChildElement(); OSMElement != NULL; OSMElement = OSMElement->NextSiblingElement()) {
        const char* elementName = OSMElement->Name();

        if (strcmp(elementName, "node") == 0) {
            // Extract node attributes and child elements
            unsigned long long nodeId = std::stoull(OSMElement->Attribute("id"));
            double nodeLat = std::stod(OSMElement->Attribute("lat"));
            double nodeLon = std::stod(OSMElement->Attribute("lon"));

            OSMNode node(nodeId, nodeLat, nodeLon);
            m_nodes.insert(std::pair<unsigned long long, OSMNode>(nodeId, node));
        }

        else if (strcmp(elementName, "way") == 0) {
            // Extract way attributes and child elements
            unsigned long long wayId = std::stoull(OSMElement->Attribute("id"));

            OSMWay way(wayId);

            // Collect all the node references which will be used to extract the vectex positions
            tinyxml2::XMLElement* nodeRef = OSMElement->FirstChildElement("nd");
            while ((nodeRef != NULL) && (strcmp(nodeRef->Name(), "nd") == 0))
            {
                way.m_nodeReferences.push_back(std::stoull(nodeRef->Attribute("ref")));
                nodeRef = nodeRef->NextSiblingElement();
            }

            if (way.m_nodeReferences.front() == way.m_nodeReferences.back()) way.m_type = OSMWay::WayType::closed;
            else OSMWay::WayType::open;

            // Collect all the tags
            tinyxml2::XMLElement* tag = OSMElement->FirstChildElement("tag");
            while ((tag != NULL) && (strcmp(tag->Name(), "tag") == 0))
            {
                way.m_tags.insert(std::pair<std::string, std::string>(tag->Attribute("k"), tag->Attribute("v")));
                tag = tag->NextSiblingElement();
            }

            m_ways.push_back(std::move(way));
        }

        else if (strcmp(elementName, "relation") == 0) {
            // Extract relation attributes and child elements
            const char* relationId = OSMElement->Attribute("id");
            // Maybe use relations in the future
        }
    }
}

// Extracts all the geometry of the given tile
void OSMDataProcessor::ExtractTileGeometry()
{

    for (auto& way : m_ways)
    {
        // Extract geometry for ways which have the tag "building"
        if (way.m_tags.find("building") != way.m_tags.end())
        {
            ExtractBuildingGeometry(way);
        }
    }
}

// Contructs 3D geometry for OSMWays with the tag 'building'
void OSMDataProcessor::ExtractBuildingGeometry(OSMWay& way)
{
    // first check if the node references are oriented clockwise
    // based on: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double sum = 0;
    for (int i = 0; i < way.m_nodeReferences.size() - 1; i++)
    {
        OSMNode& e1 = m_nodes[way.m_nodeReferences[i]];
        OSMNode& e2 = m_nodes[way.m_nodeReferences[i + 1]];
        sum += (e1.m_lat - e2.m_lat) * (e1.m_lon + e2.m_lon);
    }

    // reverse the order if it is counter-clockwise
    if (sum < 0)
        std::reverse(way.m_nodeReferences.begin(), way.m_nodeReferences.end());

    // find the number of levels in the building. We use this information to estimate the height of the building
    // set default value to 2 levels
    int levels = 2;
    if (way.m_tags.find("building:levels") != way.m_tags.end())
    {
        try {
            levels = std::stoi(way.m_tags["building:levels"]);
        }
        catch(...)
        {
            levels = 2;
        }
    }

    //Extract building:colour and roof:color
    glm::dvec3 buildingColour = { 1.0, 1.0, 1.0 };
    glm::dvec3 roofColour = { 1.0, 1.0, 1.0 };
    
    if (way.m_tags.find("building:colour") != way.m_tags.end())
    {
        std::string result = way.m_tags["building:colour"];
        try
        {
            if (result[0] == '#')
            {
                std::string colour = result.substr(1);

                int hexColor = std::stoi(colour, nullptr, 16);


                int red = (hexColor >> 16) & (0xFF);
                int green = (hexColor >> 8) & (0xFF);
                int blue = (hexColor) & (0xFF);

                buildingColour = { red / 255.0, green / 255.0, blue / 255.0 };
            }
            else
            {
                //std::cout << result << "\n";
            }
        }
        catch (...)
        {
            buildingColour = { 1.0, 1.0, 1.0 };
        }
    }
    
    if (way.m_tags.find("roof:colour") != way.m_tags.end())
    {
        std::string result = way.m_tags["roof:colour"];
        try 
        {
            if (result[0] == '#')
            {
                std::string colour = result.substr(1);

                int hexColor = std::stoi(colour, nullptr, 16);


                int red = (hexColor >> 16) & (0xFF);
                int green = (hexColor >> 8) & (0xFF);
                int blue = (hexColor) & (0xFF);

                roofColour = { red / 255.0, green / 255.0, blue / 255.0 };
            }
            else
            {
                //std::cout << result << "\n";
            }
        }
        catch (...)
        {
            roofColour = { 1.0, 1.0, 1.0 };
        }
    }

    // we assume that height of each level is 4 meters
    double height = levels * 4;

    // First we construct geometry for the vertical walls
    for (int i = 0; i < way.m_nodeReferences.size() - 1; i++)
    {
        // for every edge, we construct one vertical wall of the building
        //  v1'------v2'    -
        //  |       / |     |
        //  |      /  |     |
        //  |     /   |     |
        //  |    /    |   height
        //  |   /     |     |
        //  |  /      |     |
        //  | /       |     |
        //  v1-------v2     -
        // the two triangles that make this wall are v1v2v2' and v1v2'v1'

        OSMNode& v1 = m_nodes[way.m_nodeReferences[i]];
        OSMNode& v2 = m_nodes[way.m_nodeReferences[i + 1]];

        // The mesh will will store the vertex location as (lat, lon, altitude)
        // altitude will be in meters
        
        m_mesh.push_back({ v1.m_lat, v1.m_lon, 0 });
        m_mesh.push_back(buildingColour);
        m_mesh.push_back({ v2.m_lat, v2.m_lon, 0 });
        m_mesh.push_back(buildingColour);
        m_mesh.push_back({ v2.m_lat, v2.m_lon, height });
        m_mesh.push_back(buildingColour);

        m_mesh.push_back({ v1.m_lat, v1.m_lon, 0 });
        m_mesh.push_back(buildingColour);
        m_mesh.push_back({ v2.m_lat, v2.m_lon, height });
        m_mesh.push_back(buildingColour);
        m_mesh.push_back({ v1.m_lat, v1.m_lon, height });
        m_mesh.push_back(buildingColour);
        
    }

    // Next we construct the geometry for the roof using the ear-clipping algorithm
    using Point = std::array<double, 2>;
    std::vector<std::vector<Point>> polygon;

    polygon.resize(2);

    for (int i = 0; i < way.m_nodeReferences.size() - 1; i++)
    {
        auto& node = m_nodes[way.m_nodeReferences[i]];
        polygon[0].push_back({ node.m_lat, node.m_lon });
    }

    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);

    std::vector<glm::dvec3> colours;
    colours.push_back({ 0.2, 0.8, 0.8 });
    colours.push_back({ 0.8, 0.2, 0.8 });
    colours.push_back({ 0.8, 0.8, 0.2 });
    colours.push_back({ 0.2, 0.2, 0.8 });
    colours.push_back({ 0.8, 0.2, 0.2 });
    colours.push_back({ 0.2, 0.8, 0.2 });
    colours.push_back({ 0.5, 0.5, 0.5 });
    colours.push_back({ 0.1, 0.1, 0.1 });
    colours.push_back({ 0.9, 0.9, 0.9 });
    colours.push_back({ 0.4, 0.6, 0.6 });
    colours.push_back({ 0.3, 0.5, 0.7 });

    //int i = 0;
    for (auto index : indices)
    {
        glm::dvec2 vertex = { polygon[0][index][0], polygon[0][index][1] };
        m_mesh.push_back({ vertex.x, vertex.y, height });
        m_mesh.push_back(roofColour);
        //m_mesh.push_back(colours[((int)i / 3)%colours.size()]);
        //i++;
    }
}

