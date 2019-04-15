/*
 * \file: luaT_xml_parser.cpp
 * \brief: Created by hushouguo at Nov 06 2014 17:52:21
 */

#include "define.h"

namespace luaT {

	void luaT_xml_parser(lua_State* L, XmlParser& xmlParser, XmlParser::XML_NODE xmlNode) {
		if (!xmlNode) { return; }
		if (xmlNode != xmlParser.getRootNode()) {
			lua_pushstring(L, (const char*)xmlNode->name());
		}

		lua_newtable(L);
		for (rapidxml::xml_attribute<char> * attr = xmlNode->first_attribute(); attr != NULL; attr = attr->next_attribute()) {
			lua_pushstring(L, (const char*)attr->name());
			lua_pushstring(L, (const char*)attr->value());
			lua_settable(L, -3);
		}

		luaT_xml_parser(L, xmlParser, xmlParser.getChildNode(xmlNode, nullptr));

		if (xmlNode != xmlParser.getRootNode()) {
			lua_settable(L, -3); 
		}

		luaT_xml_parser(L, xmlParser, xmlParser.getNextNode(xmlNode, nullptr));
	}

	bool luaT_xml_parser_decode(lua_State* L, const char* file) {
		XmlParser xmlParser;
		if (!xmlParser.open(file)) {
			return false;
		}

		XmlParser::XML_NODE root = xmlParser.getRootNode();
		CHECK_RETURN(root, false, "not found root node: %s", file);

		luaT_xml_parser(L, xmlParser, root);

		xmlParser.final();
		return true;
	}

	const char* luaT_xml_parser_encode(lua_State* L) {
		return "NOT IMPLEMENT";
	}
}
