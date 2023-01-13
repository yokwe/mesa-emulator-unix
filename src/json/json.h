//
// JSON
//

#pragma once

#include <iostream>

namespace json {
//
// utility function
//
std::string glob_to_regex(std::string glob);


int dump(std::istream& in);

}
