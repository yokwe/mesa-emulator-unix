//
// JSON
//

#pragma once

#include <string>
#include <vector>

namespace json {


//
// data
//
class token_t {
public:
	enum Type {
		ITEM, ENTER, LEAVE
	};

	const std::string path;
	const Type        type;
	const std::string name;
	const bool        arrayFlag;
	const std::string value;

	// copy constructor
	token_t(const token_t& that) :
		path(that.path), type(that.type), name(that.name),
		arrayFlag(that.arrayFlag), value(that.value) {}
	// move constructor
	token_t(token_t&& that) noexcept :
		path(std::move(that.path)), type(that.type), name(std::move(that.name)),
		arrayFlag(that.arrayFlag), value(std::move(that.value)) {}

	// copy constructor
	token_t(const token_t& that, const std::string& path_) :
		path(path_), type(that.type), name(that.name),
		arrayFlag(that.arrayFlag), value(that.value) {}
	// move constructor
	token_t(token_t&& that, const std::string& path_) noexcept :
		path(path_), type(that.type), name(std::move(that.name)),
		arrayFlag(that.arrayFlag), value(std::move(that.value)) {}


	static token_t item(const std::string& path, const std::string& name, const std::string& value) {
		return token_t(path, Type::ITEM, name, value);
	}
	static token_t enter(const std::string& path, const std::string& name, bool isArray) {
		return token_t(path, Type::ENTER, name, isArray);
	}
	static token_t leave(const std::string& path, const std::string& name) {
		return token_t(path, Type::LEAVE, name);
	}

private:
	token_t(const std::string& path_, Type type_, const std::string& name_, const std::string& value_) :
		path(path_), type(type_), name(name_), arrayFlag(false), value(value_) {}
	token_t(const std::string& path_, Type type_, const std::string& name_, bool isArray) :
		path(path_), type(type_), name(name_), arrayFlag(isArray), value() {}
	token_t(const std::string& path_, Type type_, const std::string& name_) :
		path(path_), type(type_), name(name_), arrayFlag(false), value() {}
};


typedef std::vector<const token_t> token_list_t;
typedef std::vector<std::string>   string_list_t;


class handler_t {
public:
	virtual ~handler_t() {}

	// life cycle event
	virtual void start() = 0;
	virtual void stop()  = 0;

	// data event
	virtual void item (const token_t& token) = 0;
	virtual void enter(const token_t& token) = 0;
	virtual void leave(const token_t& token) = 0;
};


//
// utility function
//
std::string glob_to_regex(std::string glob);

// return true if token_list contains no item
bool empty_item(const token_list_t& token_list);

bool contains(const token_list_t& token_list, const std::string& path, const std::string& value);

token_list_t update_path(const token_list_t& token_list);

//
// parse function
//
void parse(std::istream&       in, handler_t *handler);

void parse(const token_list_t& in, handler_t *handler);


}
