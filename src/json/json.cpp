//
// json.cpp
//

#include <vector>
#include <regex>

#include "json.h"
#include "json_impl.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("json");


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
		std::string regex = json::impl::glob_to_regex(glob);
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
		std::string regex = json::impl::glob_to_regex(path);
		std::regex re(regex);
		skipPath  = re;
		skipValue = value;
	}
	//
	// value
	//
	bool process(const std::string& path, const json::impl::json_value& json_value) {
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
	void process(json::impl::json_container& container) {
		if (skipFlag) return;

		const std::string& path = container.getPath();
		if (matchPathFilter(path)) container.setFilter();
	}
};

}


int json::dump(std::istream& in) {
	json_dump sax;

	sax.skipUntil("/inner/*/loc/file", "src/main/a.cpp");

	sax.addPathFilter("**/range/**");
	sax.addPathFilter("**/loc/*");
	sax.addPathFilter("**/definitionData/**");
	sax.addPathFilter("**/bases/**");
	sax.addPathFilter("**/referencedDecl/**");

	return sax.parse(in);
}
