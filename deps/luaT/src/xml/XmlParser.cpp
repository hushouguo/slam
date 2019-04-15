/*
 * \file: XmlParser.cpp
 * \brief: Created by hushouguo at 09:54:48 Jan 22 2018
 */

#include "define.h"

namespace luaT {
	XmlParser::XmlParser() {
	}

	XmlParser::~XmlParser() {
	}

	bool XmlParser::open(std::string xmlfile) {
		return this->open(xmlfile.c_str());
	}

	bool XmlParser::open(const char* xmlfile) {
		try {
			rapidxml::file<> fileDoc(xmlfile);
			doc.parse<0>(fileDoc.data());
		}
		catch (std::exception& e) {
			Error("xmlParser exception: %s, filename: %s", e.what(), xmlfile);
			return false;
		}

		//file content: doc.name()

		return true;
	}

	void XmlParser::final() {
	}

	XmlParser::XML_NODE XmlParser::getRootNode() {
		return doc.first_node();
	}

	XmlParser::XML_NODE XmlParser::getChildNode(XML_NODE xmlNode, const char* name) {
		if (name != nullptr) {
			return xmlNode->first_node(name);
		}
		return xmlNode->first_node();
	}


	XmlParser::XML_NODE XmlParser::getNextNode(XML_NODE xmlNode, const char* name) {
		if (name != nullptr) {
			return xmlNode->next_sibling(name);
		}
		return xmlNode->next_sibling();
	}

	const char* XmlParser::getValueByString(XML_NODE xmlNode, const char* name, const char* defaultValue) {
		rapidxml::xml_attribute<char> * attr = nullptr;
		if (name != nullptr) {
			attr = xmlNode->first_attribute(name);
		}
		else {
			attr = xmlNode->first_attribute();
		}
		return attr == nullptr ? defaultValue : attr->value();
	}

	const uint32_t XmlParser::getValueByInteger(XML_NODE xmlNode, const char* name, uint32_t defaultValue) {
		rapidxml::xml_attribute<char> * attr = nullptr;
		if (name != nullptr) {
			attr = xmlNode->first_attribute(name);
		}
		else {
			attr = xmlNode->first_attribute();
		}
		return attr == nullptr ? defaultValue : strtoul((char*)attr->value(), (char**)nullptr, 10);
	}

	void XmlParser::getValues(XML_NODE xmlNode, std::map<std::string, std::string> values) {
		for (rapidxml::xml_attribute<char> * attr = xmlNode->first_attribute(); attr != NULL; attr = attr->next_attribute()) {
			values.insert(std::make_pair((const char*)attr->name(), (const char*)attr->value()));
		}
	}

#if 0
	void XmlParser::makeRegistry(Registry* registry, const char* prefix) {
		this->makeRegistry(registry, this->getRootNode(), prefix);
	}

	void XmlParser::makeRegistry(Registry* registry, XML_NODE xmlNode, std::string name) {
		if (!xmlNode) { 
			return; 
		}

		for (rapidxml::xml_attribute<char> * attr = xmlNode->first_attribute(); attr; attr = attr->next_attribute()) {
			//if (xmlNode == this->getRootNode()) {
			if (name.length() > 0) {
				char buf[1024];
				snprintf(buf, sizeof(buf), "%s.%s.%s", name.c_str(), xmlNode->name(), attr->name());
				if (!registry->set(buf, attr->value())) {
					Error("duplicate key: %s", buf);
				}
			}
			else {
				char buf[1024];
				snprintf(buf, sizeof(buf), "%s.%s", xmlNode->name(), attr->name());
				if (!registry->set(buf, attr->value())) {
					Error("duplicate key: %s", buf);
				}
			}
		}

		if (xmlNode == this->getRootNode()) {
			this->makeRegistry(registry, this->getChildNode(xmlNode), "");
		}
		else {
			if (name.length() > 0) {
				this->makeRegistry(registry, this->getChildNode(xmlNode), name + "." + xmlNode->name());
			}
			else {
				this->makeRegistry(registry, this->getChildNode(xmlNode), xmlNode->name());
			}
		}

		this->makeRegistry(registry, this->getNextNode(xmlNode), name);
	}
#endif

	void XmlParser::dump() {
		dump(getRootNode(), 0);
	}

	void XmlParser::dump(XML_NODE xmlNode, int depth) {
		if (!xmlNode) { 
			return; 
		}

		for (int i = 0; i < depth; ++i) {
			Trace("\t");
		}

		Trace("<%s", xmlNode->name());

		for (rapidxml::xml_attribute<char> * attr = xmlNode->first_attribute(); attr; attr = attr->next_attribute()) {
			Trace(" %s=\"%s\"", attr->name(), attr->value());
		}

		if (!getChildNode(xmlNode)) {
			Trace(" />");
		}
		else {
			Trace(">");
		}

		dump(getChildNode(xmlNode), depth + 1);

		if (getChildNode(xmlNode)) {
			for (int i = 0; i < depth; ++i) {
				Trace("\t");
			}
			Trace("</%s>", xmlNode->name());
		}

		dump(getNextNode(xmlNode), depth);
	}	
}
