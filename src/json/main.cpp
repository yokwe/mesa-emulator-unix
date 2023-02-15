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


int main(int, char**) {
	logger.info("START");

	setSignalHandler(SIGSEGV);
	setSignalHandler(SIGILL);
	setSignalHandler(SIGABRT);

#if 1
	{
		auto head = stream::json::json(std::cin);
		logger.info("head %s", demangle(typeid(head).name()));

		auto map = stream::map(&head, [](json::token_t t){return t;});
		logger.info("map %s", demangle(typeid(map).name()));

		auto tail = stream::count(&map);
		logger.info("tail %s", demangle(typeid(tail).name()));

		logger.info("tail %d", tail.process());
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
