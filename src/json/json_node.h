//
//
//

#pragma once

#include <string>
#include <vector>

namespace json {
namespace node {


class node_t;

class item_t {
	std::string path;
	std::string valueString;
	node_t*     valueNode;
	bool        valueNodeFree;

public:
	item_t(const std::string& path_, const std::string& value_) :
		path(path_), valueString(value_), valueNode(nullptr), valueNodeFree(false) {}
	item_t(const std::string& path_, node_t* value_) :
		path(path_), valueString(""), valueNode(value_), valueNodeFree(true) {}
	item_t(const std::string& path_, node_t& value_) :
		path(path_), valueString(""), valueNode(&value_), valueNodeFree(false) {}

	// copy constructor
	item_t(const item_t& that) :
		path(that.path), valueString(that.valueString),
		valueNode(that.valueNode), valueNodeFree(that.valueNodeFree) {}

	// move constructor
	item_t(item_t&& that) noexcept :
		path(std::move(that.path)), valueString(std::move(that.valueString)),
		valueNode(that.valueNode), valueNodeFree(that.valueNodeFree) {}


	~item_t();

	std::string& getPath() {
		return path;
	}

	bool isNode() {
		return valueNode != nullptr;
	}

	std::string& getString();
	node_t&      getNode();
};


class node_t {
public:
	std::string         path;
	std::vector<item_t> entries;

	node_t(const std::string& path_) : path(path_) {}

	// copy constructor
	node_t(const node_t& that) : path(that.path), entries(that.entries) {}

	// move constructor
	node_t(node_t&& that) noexcept :
		path(std::move(that.path)) {
		std::move(that.entries.begin(), that.entries.end(), std::back_inserter(entries));
	}


	void add(const std::string& path_, const std::string& value_) {
		item_t item(path_, value_);
		entries.push_back(item);
	}
	void add(const std::string& path_, node_t* value_) {
		item_t item(path_, value_);
		entries.push_back(item);
	}
	void add(const std::string& path_, node_t& value_) {
		item_t item(path_, value_);
		entries.push_back(item);
	}
};


}
}
