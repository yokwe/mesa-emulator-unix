/*******************************************************************************
 * Copyright (c) 2024, Yasuhiro Hasegawa
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


//
// Util.cpp
//

#include <filesystem>
#include <stack>
#include <map>
#include <utility>

#include <execinfo.h>
#include <cxxabi.h>

#include <log4cxx/logger.h>
#include <log4cxx/level.h>


#include "Util.h"


static const util::Logger logger(__FILE__);


util::Logger::Logger(const char* name) : myLogger(log4cxx::Logger::getLogger(std::filesystem::path(name).stem().string())) {}

// DEBUG, INFO, WARN, ERROR, FATAL
static std::map<util::Logger::Level, log4cxx::LevelPtr> levelMap = {
	{util::Logger::Level::DEBUG, log4cxx::Level::getDebug()},
	{util::Logger::Level::INFO,  log4cxx::Level::getInfo()},
	{util::Logger::Level::WARN,  log4cxx::Level::getWarn()},
	{util::Logger::Level::ERROR, log4cxx::Level::getError()},
	{util::Logger::Level::FATAL, log4cxx::Level::getFatal()},
	{util::Logger::Level::OFF  , log4cxx::Level::getOff()},
};
static log4cxx::LevelPtr toLevelPtr(util::Logger::Level level) {
	auto it = levelMap.find(level);
	if (it != levelMap.end()) {
		// found
		return it->second;
	} else {
		logger.fatal("Unexpected");
		ERROR();
	}
}

static std::stack<log4cxx::LevelPtr> stack;

void util::Logger::pushLevel(Level newLevel) {
	auto rootLogger = log4cxx::Logger::getRootLogger();
 	stack.push(rootLogger->getLevel());
	rootLogger->setLevel(toLevelPtr(newLevel));
}
void util::Logger::popLevel() {
	if (stack.empty()) {
		ERROR();
	}
	auto rootLogger = log4cxx::Logger::getRootLogger();
	rootLogger->setLevel(stack.top());
	stack.pop();
}


static std::string demangle(const char* mangled) {
	char buffer[512];
	size_t length(sizeof(buffer));
	int status(0);

	char* demangled = __cxxabiv1::__cxa_demangle(mangled, buffer, &length, &status);
	if (status != 0) {
		logger.warn("demange %d %s", status, mangled);
	}

	std::string ret(demangled);
	return ret;
}

void util::logBackTrace() {
	const int BUFFER_SIZE = 100;
	void *buffer[BUFFER_SIZE];

	// get void*'s for all entries on the stack
	int size = backtrace(buffer, BUFFER_SIZE);

	char **msg = backtrace_symbols(buffer, size);

	// print out all the frames
	for(int i = 0; i < size; i++) {
		std::string line(msg[i]);
		auto pos = line.find("_Z");
		if (pos == std::string::npos) {
			// contains no mangled name
			logger.debug("%3d %s", i, msg[i]);
		} else {
			// contains mangled name
			std::string left(line.substr(0, pos));
			std::string middle;
			for(; pos < line.size(); pos++) {
				char c = line.at(pos);
				if (std::isalnum(c) || c == '_') {
					middle += c;
					continue;
				}
				break;
			}
			std::string right(line.substr(pos));
			middle = demangle(middle.c_str());
			logger.debug("%3d %s%s%s", i, left, middle, right);
		}
	}
}


static void signalHandler(int signum) {
	logger.fatal("Error: signal %d:\n", signum);
	util::logBackTrace();
	exit(1);
}

void util::setSignalHandler(int signum) {
	signal(signum, signalHandler);
}

