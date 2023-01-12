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

	//
	// value
	//
	void process(const json::impl::json_value& value) {
		if (top().filter()) return;

		std::string path = top().getPath(lastKey);
		if (matchPathFilter(path)) {
			// filtered
		} else {
			std::string line = path + " " + value.to_string();
			std::cout << line << std::endl;
		}
	}

	//
	// container
	//
	void process(json::impl::json_container& container) {
		const std::string& path = container.getPath();
		if (matchPathFilter(path)) container.setFilter();
	}
};

}


bool json::dump(std::istream& in) {
	json_dump sax;

	sax.addPathFilter("**Id");
	sax.addPathFilter("**/range/**");
	sax.addPathFilter("**/is*");
	sax.addPathFilter("**/loc/includedFrom/**");
	sax.addPathFilter("**/loc/offset");
	sax.addPathFilter("**/loc/line");
	sax.addPathFilter("**/loc/col");
	sax.addPathFilter("**/loc/tokLen");
	sax.addPathFilter("**/loc/spellingLoc/**");
	sax.addPathFilter("**/loc/expansionLoc/**");
	sax.addPathFilter("**/definitionData/**");
	sax.addPathFilter("**/bases/**");

	return sax.parse(in);
}
