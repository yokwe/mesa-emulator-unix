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

#include <cstdio>

#include "json.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("gen_introspection");

#include "stream.h"
#include "stream_util.h"
#include "stream_json.h"

#include "../mesa/Pilot.h"

/*
# macmini2020.lan
clang -Xclang -ast-dump=json -fsyntax-only
	-I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1 \
	-I /Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/include \
	-I /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include \
	-I /Library/Developer/CommandLineTools/usr/include \
	-I /opt/local/include \
	-I /opt/local/libexec/qt6/include \
	-I /opt/local/libexec/qt6/include/QtCore \
    -std=c++17 src/main/main.cpp | LOG_CONFIG=tmp/cmake/macmini2020.lan/run/debug.properties tmp/cmake/macmini2020.lan/build/main/main

# dev-clang
clang -Xclang -ast-dump=json -fsyntax-only
    -I /usr/local/include
    -I /usr/local/include/qt6
    -I /usr/local/include/qt6/QtCore
    -std=c++17 -fPIC src/main/main.cpp | LOG_CONFIG=tmp/cmake/dev-clang/run/debug.properties tmp/cmake/dev-clang/build/main/main
*/

#include "introspection.h"



#define check_error(cond) { \
	if (!(cond)) { \
		int errno_ = errno; \
		logger.error("errno = %d %s", errno_, strerror(errno_)); \
		assert(false); \
	} \
}

struct peek_consumer_t {
	struct context_t {
		std::FILE*  m_file;
		char        m_buffer[1024 * 64];

		context_t(const std::string& path) {
			logger.info("peek_consumer file %s", path);
			m_file = std::fopen(path.c_str(), "w");
			check_error(m_file != NULL)

			{
				int ret = setvbuf(m_file, m_buffer, _IOFBF, sizeof(m_buffer));
				check_error(ret == 0);
			}
		}
		~context_t() {
			{
				int ret = std::fclose(m_file);
				check_error(ret == 0);
			}
		}
	};

	std::shared_ptr<context_t> m_context;

	peek_consumer_t(const std::string& path) {
		m_context = std::make_shared<context_t>(path);
	}

	int count = 0;
	void operator()(const json::token_t& list) {
		(void)list;
//		dump("operator ", list);
	}
};

int main(int argc, char** argv) {
	logger.info("START");

	(void)argc;
	(void)argv;

	{
		auto head    = stream::json::json(std::cin);
		auto splitA  = stream::json::split(&head, "/inner/*");
		auto filterA = stream::json::include_clang_source(&splitA,
			"**/Pilot.h",
			"**/Type.h"
			);
		auto filterB = stream::json::include_ojbect_by_path_value(&filterA, "**/kind",
			"EnumConstantDecl"
		);
		auto filterC = stream::json::exclude_token_by_path(&filterB,
			"**/range/**",
			"**/loc/**",
			"**/id"
		);
		auto filterD = stream::json::exclude_ojbect_by_path_value(&filterC, "**/kind",
			"ImplicitCastExpr",
			"ConstantExpr"
		);

		auto filterZ = stream::json::normalize(&filterD);
		auto expand  = stream::json::expand(&filterZ);
		auto saveZ   = stream::json::file(&expand, "tmp/save-gen.json");
		auto countZ  = stream::count(&saveZ, "countZ");

		stream::null(&countZ);
	}


	logger.info("STOP");
	return 0;
}
