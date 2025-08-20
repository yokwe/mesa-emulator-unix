/*******************************************************************************
 * Copyright (c) 2021, Yasuhiro Hasegawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <nlohmann/json.hpp>

#include <iostream>

using json = nlohmann::json;

#include "../util/Util.h"
static const util::Logger logger(__FILE__);

class sax_event_consumer : public json::json_sax_t {
  public:
	int countNull   = 0;
	int countBool   = 0;
	int countInt    = 0;
	int countUInt   = 0;
	int countFloat  = 0;
	int countString = 0;
	int countArray  = 0;
	int countObject = 0;
    //
    // value
    //
    bool null() override {
    	countNull++;
        return true;
    }

    bool boolean(bool) override {
    	countBool++;
        return true;
    }

    bool number_integer(number_integer_t) override {
    	countInt++;
       return true;
    }

    bool number_unsigned(number_unsigned_t) override {
    	countUInt++;
        return true;
    }

    bool number_float(number_float_t, const string_t&) override {
    	countFloat++;
    	return true;
    }

    bool string(string_t&) override {
    	countString++;
        return true;
    }

    //
    // object
    //
    bool start_object(std::size_t) override {
    	countObject++;
    	return true;
    }

    bool end_object() override {
    	return true;
    }

    //
    // array
    //
    bool start_array(std::size_t) override {
    	countArray++;
    	return true;
    }

    bool end_array() override {
        return true;
    }

    //
    // name of object, array or primitive value
    //
    bool key(string_t&) override {
        return true;
    }

    //
    // binary ???
    //
    bool binary(json::binary_t& /*val*/) override {
    	DEBUG_TRACE();
    	ERROR();
        return true;
    }

    bool parse_error(std::size_t position, const std::string& last_token, const json::exception& ex) override {
    	logger.error("parser_error");
    	logger.error("  position = %ld  last token = %s  ex = %s", position, last_token, ex.what());
    	ERROR();
        return false;
    }
};


int main(int, char**) {
	logger.info("START");
	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	// create a SAX event consumer object
	sax_event_consumer sec;

	// parse JSON
	bool result = json::sax_parse(std::cin, &sec);
	logger.info("parse result = %d", result);

	// output the recorded events
	logger.info("countNull   = %8d", sec.countNull);
	logger.info("countBool   = %8d", sec.countBool);
	logger.info("countInt    = %8d", sec.countInt);
	logger.info("countUInt   = %8d", sec.countUInt);
	logger.info("countFloat  = %8d", sec.countFloat);
	logger.info("countString = %8d", sec.countString);
	logger.info("countObject = %8d", sec.countObject);
	logger.info("countArray  = %8d", sec.countArray);

	logger.info("STOP");
	return 0;
}

/*
# macmini2020.lan
clang -Xclang -ast-dump=json -fsyntax-only -I /opt/local/include -I /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include -I /opt/local/libexec/qt6/include -I /opt/local/libexec/qt6/include/QtCore -std=c++17 src/main/main.cpp | LOG_CONFIG=tmp/cmake/macmini2020.lan/run/debug.properties tmp/cmake/macmini2020.lan/build/main/main

# dev-clang
clang -Xclang -ast-dump=json -fsyntax-only -I /usr/local/include -I /usr/local/include/qt6 -I /usr/local/include/qt6/QtCore -std=c++17 -fPIC src/main/main.cpp | LOG_CONFIG=tmp/cmake/dev-clang/run/debug.properties tmp/cmake/dev-clang/build/main/main
*/
