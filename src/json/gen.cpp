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
#include <fstream>

#include "json.h"

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("gen");

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
		std::string m_enum_name;
		int         m_enum_value = 0;

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
	void operator()(const json::token_list_t& list) {
		assert(list.front().is_start_object());

		{
			json::token_list_t level1 = json::copy_object(list, 1);

			auto kind_opt     = json::get_item_by_name(level1, "kind",     0);
			assert(kind_opt.has_value());
			std::string kind = kind_opt->value();
			assert(kind == "EnumConstantDecl");

			auto name_opt     = json::get_item_by_name(level1, "name", 0);
			assert(name_opt.has_value());
			std::string name = name_opt->value();

			auto qualType_opt = json::get_item_by_name(level1, "qualType", 1);
			assert(qualType_opt.has_value());
			std::string qualType = qualType_opt->value();

			if (m_context->m_enum_name != qualType) {
				m_context->m_enum_name = qualType;
				m_context->m_enum_value = -1;
			}

			auto const_expr  = json::get_object_by_name_value(list, "kind", "ConstantExpr");
			std::string value;

			if (const_expr.has_value()) {
				auto value_opt = json::get_item_by_name(const_expr.value(), "value", 0);
				if (value_opt.has_value()) {
					value = value_opt->value();
					m_context->m_enum_value = atoi(value.c_str());
				}
			}
			if (value.empty()) {
				++m_context->m_enum_value;
				value = std::to_string(m_context->m_enum_value);
			}
			logger.info("add(%-40s, %-20s, %4s);\n",  qualType, name, value);
			fprintf(m_context->m_file, "add(%-40s, %-20s, %4s);\n", qualType.c_str(), name.c_str(), value.c_str());
		}
	}
};

int main(int argc, char** argv) {
	logger.info("START");

	(void)argc;
	(void)argv;

	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

	{
		auto head    = stream::json::json(std::cin);
		auto saveA   = stream::json::file(&head, "tmp/save-a.json");
		auto splitA  = stream::json::split(&saveA, "/inner/*");
		auto filterA = stream::json::include_clang_source(&splitA,
			"**/Pilot.h",
			"**/Type.h"
			);
		auto filterB = stream::json::exclude_token_by_path(&filterA,
			"**/range/**",
			"**/loc/**",
			"**/referencedDecl/**",
			"**/definitionData/**"
		);
		auto filterC = stream::json::include_ojbect_by_path_value(&filterB, "**/kind",
			"EnumConstantDecl"
		);
		auto filterD = stream::json::exclude_ojbect_by_path_value(&filterC, "**/kind",
			"IntegerLiteral"
		);
		auto filterZ = stream::json::normalize(&filterD);
		auto expand  = stream::json::expand(&filterZ);
		auto saveZ   = stream::json::file(&expand, "tmp/save-gen.json");
		auto countZ  = stream::count(&saveZ, "countZ");

		auto split2  = stream::json::split(&countZ, "/*");
		peek_consumer_t consumer("src/json/introspection_enum.cpp");
		auto peek    = stream::peek(&split2, consumer);

		stream::null(&peek);
	}

	logger.info("STOP");
	return 0;
}
