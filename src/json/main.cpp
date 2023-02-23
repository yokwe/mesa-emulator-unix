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

#include "json.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("main");

#include "stream.h"
#include "stream_util.h"
#include "stream_json.h"

/*
# macmini2020.lan
clang -Xclang -ast-dump=json -fsyntax-only -I /opt/local/include -I /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include -I /opt/local/libexec/qt6/include -I /opt/local/libexec/qt6/include/QtCore -std=c++17 src/main/main.cpp | LOG_CONFIG=tmp/cmake/macmini2020.lan/run/debug.properties tmp/cmake/macmini2020.lan/build/main/main

# dev-clang
clang -Xclang -ast-dump=json -fsyntax-only -I /usr/local/include -I /usr/local/include/qt6 -I /usr/local/include/qt6/QtCore -std=c++17 -fPIC src/main/main.cpp | LOG_CONFIG=tmp/cmake/dev-clang/run/debug.properties tmp/cmake/dev-clang/build/main/main
*/

int main(int, char**) {
	logger.info("START");

	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

#if 0
	{
		auto head   = stream::json::json(std::cin);
		auto file   = stream::json::file(&head, "tmp/a.json");
//		auto peek   = stream::peek(&file, [](auto t){(void)t;});
		stream::null(&file);
	}
#endif

#if 1
	{
		auto head   = stream::json::json(std::cin);
//		logger.info("head %s", demangle(typeid(head).name()));
//		while(head.has_next()) head.next();

		auto countA  = stream::count(&head, "countA");
		auto map     = stream::map(&countA, [](auto t){return t;});
		auto split   = stream::json::split(&map, "/inner/*");
		auto countB  = stream::count(&split, "countB");
		auto filterA = stream::json::include_loc_file(&countB, "src/main/a.cpp");
//		auto filterA = stream::json::include_path_value(&countB, "/kind", "EnumDecl");
		auto countC  = stream::count(&filterA, "countC");
		auto expand  = stream::json::expand(&countC);
		auto countD  = stream::count(&expand, "countD");
		auto filterB = stream::json::exclude_path(&countD,
				"**/range/**",
				"**/loc/**",
				"**/includedFrom/**",
				"**/definitionData/**",
				"**/referencedDecl/**"
		);
		auto countE  = stream::count(&filterB, "countE");
		//auto filterC = stream::filter(&countE, [](auto t){return t.name() != "id";});
		//auto filterC = stream::json::exclude_path(&countE, "**/abc");

		auto dump    = stream::peek(&countE, [](auto token){(void)token; /*json::dump("PEEK ", token);*/ });

		auto filterD = stream::json::include_path(&dump, "**/kind", "**/name", "**/qualType", "**/value", "**/opcode");
		auto countF  = stream::count(&filterD, "countF");

		auto file    = stream::json::file(&countF, "tmp/a.json");

		stream::null(&file);
		//logger.info("stream::count() %d", stream::count(&dump));

//		logger.info("tail %s %s", tail.name(), demangle(typeid(tail).name()));
	}
#endif

#if 0
	{
		auto head   = stream::vector({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
		auto countA = stream::count(&head, "countA");
		auto filter = stream::filter(&countA, [](int a){return a < 6;});
		auto add    = stream::map(&filter,    [](int a){return (long)a + 1000;});
		auto sub    = stream::map(&add,       [](long a){return a - 1000;});
		auto countB = stream::count(&sub, "countB");
		auto sum    = stream::sum(&countB);

		logger.info("sum %s", std::to_string(sum.process()));
	}
#endif

#if 0
	{
		stream::json::json_t json(std::cin);
		auto countA  = stream::count(&json, "countA");
		auto splitA  = stream::json::split(&countA, "/inner/*");
		// auto splitA = stream::token_list(&countA, "/inner/*");
		// auto_splitA = stream::token::token_list(&countA, "/inner/*");

		auto filterA = stream::json::include_path_value(&splitA, "/kind", "EnumDecl");
		// auto filterA = stream::include_token_list(&splitA, "/kind", "EnumDecl");
		// auto filterA = stream:::token_list::include(&splitA, "/kind", "EnumDecl");

		auto peekA   = stream::peek(&filterA, [](json::token_list_t){});

		auto countB  = stream::count(&peekA, "countB");
		auto expandA = stream::json::expand(&countB);
		// auto expandA = stream::expand_token_list(&countB);
		// auto expandA = stream::token_list::expand(&countB);
		
		auto filterB = stream::json::exclude_path(&expandA,
			{
				"**/range/**",
				"**/loc/**",
				"**/includedFrom/**",
				"**/range",
				"**/loc",
				"**/includedFrom"
			}
		);
		// auto filterB = stream::exclude_token_path(&expandA, ...);
		// auto filgerB = stream::token::exclude_path(&expandA, ...);

		//auto peekB   = stream::peek(&filterB,   [](json::token_t t){json::dump_item("BB ", t);});

		class {
		public:
			void start() {
				logger.info("my_callback start");
			}
			void stop() {
				logger.info("my_callback stop");
			}
			void data(json::token_t newValue) {
				json::dump_item("CC ", newValue);
			}
		} my_callback;
		auto peekB = stream::tee(&filterB, my_callback);


		auto count   = stream::count(&peekB);
		logger.info("count %s", std::to_string(count.process()));
	}
#endif

#if 0
	{
		stream::json_t json(std::cin);
		auto countA  = stream::filter(&json, [](json::token_t t){dump("AA ", t);return true;});
		auto count   = stream::count(&countA);
		logger.info("count %s", std::to_string(count.process()));
	}
#endif

	logger.info("STOP");
	return 0;
}
