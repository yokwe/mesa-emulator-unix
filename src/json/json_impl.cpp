//
//
//


#include <vector>
#include <regex>

#include "json.h"
#include "json_impl.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json-impl");

namespace json {
namespace impl {


//
// json_value
//
const std::string json_value::NULL_STRING  = "NULL";
const std::string json_value::TRUE_STRING  = "TRUE";
const std::string json_value::FALSE_STRING = "FALSE";


//
// json_sax
//
bool json_sax::binary(nlohmann::json::binary_t& /*val*/) {
	ERROR();
	return true;
}
bool json_sax::parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) {
	logger.error("parser_error");
	logger.error("  position = %ld  last token = %s  ex = %s", position, last_token, ex.what());
	ERROR();
	return false;
}


}
}


//
// json_dump
//

// to hide detail, use unnamed namespace
namespace {
//
// json_dum dump dot form json with filter
//
class json_dump : public json::impl::json_sax {
public:
	std::vector<std::regex> pathFilters;
	bool skipFlag = false;
	std::regex  skipPath;
	std::string skipValue;

	void addPathFilter(std::string glob) {
		logger.info("pathFilter %s", glob);
		std::string regex = json::glob_to_regex(glob);
		std::regex re(regex);
		pathFilters.push_back(re);
	}
	bool matchPathFilter(const std::string& path) {
		for(auto re: pathFilters) {
			if (std::regex_match(path, re)) return true;
		}
		return false;
	}
	void skipUntil(const std::string& path, const std::string& value) {
		logger.info("skipUntil %s %s", path, value);
		skipFlag  = true;
		std::string regex = json::glob_to_regex(path);
		std::regex re(regex);
		skipPath  = re;
		skipValue = value;
	}
	//
	// value
	//
	bool process(const std::string& path, const std::string& name, const json::impl::json_value& json_value) {
		(void)name; // avoid unused parameter
		const std::string& value = json_value.to_string();

		if (skipFlag && value == skipValue && std::regex_match(path, skipPath)) {
			skipFlag = false;
		}
		if (skipFlag) return false;

		if (top().filter()) return false;

		if (matchPathFilter(path)) return false;

		std::cout << path << " " << value << std::endl;
		return true;
	}

	//
	// container
	//
	void process(const std::string& path, const std::string& name, json::impl::json_container& json_container) {
		(void)name; // avoid unused parameter
		if (skipFlag) return;

		if (matchPathFilter(path)) json_container.setFilter();
	}
};

}


int json::impl::dump(std::istream& in) {
	json_dump sax;

	sax.skipUntil("/inner/*/loc/file", "src/main/a.cpp");

	sax.addPathFilter("**/range/**");
	sax.addPathFilter("**/loc/*");
	sax.addPathFilter("**/definitionData/**");
	sax.addPathFilter("**/bases/**");
	sax.addPathFilter("**/referencedDecl/**");

	return sax.parse(in);
}
