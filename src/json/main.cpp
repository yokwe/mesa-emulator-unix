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
clang -Xclang -ast-dump=json -fsyntax-only
  -I /opt/local/include
  -I /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
  -I /opt/local/libexec/qt6/include
  -I /opt/local/libexec/qt6/include/QtCore
  -std=c++17 src/main/main.cpp | LOG_CONFIG=tmp/cmake/macmini2020.lan/run/debug.properties tmp/cmake/macmini2020.lan/build/main/main

# dev-clang
clang -Xclang -ast-dump=json -fsyntax-only
   -I /usr/local/include
   -I /usr/local/include/qt6
   -I /usr/local/include/qt6/QtCore
   -std=c++17 -fPIC src/main/main.cpp | LOG_CONFIG=tmp/cmake/dev-clang/run/debug.properties tmp/cmake/dev-clang/build/main/main
*/

int main(int argc, char** argv) {
	logger.info("START");

	(void)argc;
	(void)argv;


//	setSignalHandler(SIGSEGV);
//	setSignalHandler(SIGILL);
//	setSignalHandler(SIGABRT);

#if 0
	{
		auto head    = stream::json::json(std::cin);
		auto split   = stream::json::split(&head, "/inner/*");
		auto filterA = stream::json::include_source_file(&split, "src/**");
		auto filterB = stream::json::exclude_builtin_source_file(&filterA);
		while(filterB.has_next()) {
			json::token_list_t list = filterB.next();
			logger.info("====");
			json::dump("AA ", list);
		}
	}
#endif

#if 1
	{
		auto head    = stream::json::json(std::cin);
		auto saveA   = stream::json::file(&head, "tmp/save-a.json");
		auto countA  = stream::count(&saveA, "countA");
		auto split   = stream::json::split(&countA, "/inner/*");
		auto countB  = stream::count(&split, "countB");

		auto filterA = stream::json::include_clang_source(&countB,
			"**/Pilot.h",
			"**/Type.h"
			);
		auto countC  = stream::count(&filterA, "countC");
		auto filterB = stream::json::exclude_clang_builtin_source(&countC);
		auto countD  = stream::count(&filterB, "countD");
		auto filterC = stream::json::exclude_token_by_path(&countD,
			"**/range/**",
			"**/loc/**",
			"**/includedFrom/**",
			"**/definitionData/**",
			"**/referencedDecl/**"
		);
		auto countE  = stream::count(&filterC, "countE");
		auto filterD = stream::json::include_token_by_path(&countE,
			"/source",
			"**/kind",
			"**/name",
			"**/qualType",
			"**/value",
			"**/opcode"
		);
		auto countF  = stream::count(&filterC, "countF");

		auto filterE = stream::json::include_token_list_by_path_value(&countF, "/kind", "EnumDecl");
		auto countG  = stream::count(&filterE, "countG");

		auto expand  = stream::json::expand(&countG);

		auto countZ  = stream::count(&expand, "countZ");
		auto saveZ   = stream::json::file(&countZ, "tmp/save-z.json");
		stream::null(&saveZ);
	}
#endif

#if 0
	{
		assert(argc == 3);
		const char* file_path   = argv[1];
		const char* output_path = argv[2];
		logger.info("file_path   %s", file_path);
		logger.info("output_path %s", output_path);

		auto head    = stream::json::json(std::cin);
		auto countA  = stream::count(&head, "countA");
		auto split   = stream::json::split(&countA, "/inner/*");
		auto filterA = stream::json::include_loc_file(&split, file_path);
		auto countB  = stream::count(&split, "filterA");
		auto peek    = stream::peek(&countB, [](const json::token_list_t& list){
			logger.info("==== list ====");
			json::dump("AA ", list);
			json::element_t element;
			json::list_to_element(list, element);
			logger.info("==== element ====");
			json::dump("BB ", element);
			logger.info("====");
		});

		stream::null(&countB);
	}
#endif

#if 0
	{
		auto head    = stream::json::json(std::cin);
		auto countA  = stream::count(&head, "countA");
		auto saveA   = stream::json::file(&countA, "tmp/save-a.json");
		auto split   = stream::json::split(&saveA, "/inner/*");
		auto expand  = stream::json::expand(&split);

		auto countB  = stream::count(&split, "countB");
		auto filterA = stream::json::include_file(&countB, "src/**/Type.h");
		auto filterB = stream::json::exclude_path(&filterA,
			"**/range/**",
			"**/loc/**",
			"**/includedFrom/**",
			"**/definitionData/**",
			"**/referencedDecl/**"
		);
		auto filterC = stream::json::include_token_by_path(&filterB,
			"**/id",
			"**/kind",
			"**/name",
			"**/qualType",
			"**/value",
			"**/opcode",
			"/loc/file"
		);
		auto countC  = stream::count(&filterC, "countC");
		auto expand  = stream::json::expand(&countC);
		auto countZ  = stream::count(&expand, "countZ");
		auto file    = stream::json::file(&countZ, "tmp/save-z.json");

		stream::null(&file);
	}
#endif

	logger.info("STOP");
	return 0;
}
