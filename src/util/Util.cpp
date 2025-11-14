/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
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
#include <vector>
#include <regex>
#include <stack>
#include <map>
#include <utility>
#include <chrono>
#include <iostream>
#include <fstream>
#include <bit>

#include <execinfo.h>
#include <cxxabi.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <log4cxx/logger.h>
#include <log4cxx/level.h>


#include "Util.h"
static const Logger logger(__FILE__);

void logBackTrace() {
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
			logger.fatal("%3d %s", i, msg[i]);
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
			logger.fatal("%3d %s%s%s", i, left, middle, right);
		}
	}
}

static void signalHandler(int signum) {
	logger.fatal("Error: signal %d:\n", signum);
	logBackTrace();
	exit(1);
}

void setSignalHandler(int signum) {
	signal(signum, signalHandler);
}

std::string demangle(const char* mangled) {
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



// Logger
Logger::Logger(const char* name) : myLogger(log4cxx::Logger::getLogger(std::filesystem::path(name).stem().string())) {}

// DEBUG, INFO, WARN, ERROR, FATAL
static std::map<Logger::Level, log4cxx::LevelPtr> levelMap = {
	{Logger::Level::DEBUG, log4cxx::Level::getDebug()},
	{Logger::Level::INFO,  log4cxx::Level::getInfo()},
	{Logger::Level::WARN,  log4cxx::Level::getWarn()},
	{Logger::Level::ERROR, log4cxx::Level::getError()},
	{Logger::Level::FATAL, log4cxx::Level::getFatal()},
	{Logger::Level::OFF  , log4cxx::Level::getOff()},
};
static log4cxx::LevelPtr toLevelPtr(Logger::Level level) {
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

void Logger::pushLevel(Level newLevel) {
	auto rootLogger = log4cxx::Logger::getRootLogger();
 	stack.push(rootLogger->getLevel());
	rootLogger->setLevel(toLevelPtr(newLevel));
}
void Logger::popLevel() {
	if (stack.empty()) {
		ERROR();
	}
	auto rootLogger = log4cxx::Logger::getRootLogger();
	rootLogger->setLevel(stack.top());
	stack.pop();
}

std::set<uint16_t> Logger::stopMessageUntilMPSet;
void Logger::mp_observer(uint16_t mp) {
	if (stopMessageUntilMPSet.contains(mp)) {
		Logger::popLevel();
		logger.info("show message at MP %4d", mp);
		// clear stopMessageUntilMPSet to prevent call twice
		stopMessageUntilMPSet.clear();
	}
}

int32_t toIntMesaNumber(const std::string& string) {
	int radix = 0;
	std::string numberString;
	if (string == "0") return 0;
	
	if (string.starts_with("0x") || string.starts_with("0X")) {
		// c style hexadecimal
		radix = 16;
		numberString = string.substr(2, string.length() - 2);
	} else if (string.starts_with("0")) {
		// c style octal
		radix = 8;
		numberString = string.substr(1, string.length() - 1);
	} else if (string.ends_with("H")) {
		// mesa style hexadecimal
		radix = 16;
		numberString = string.substr(0, string.length() - 1);
	} else if (string.ends_with("B")) {
		// mesa style octal
		radix = 8;
		numberString = string.substr(0, string.length() - 1);
	} else {
		// decimal
		radix = 10;
		numberString = string;
	}

	try {
		size_t idx;
		uint32_t ret = std::stoul(numberString,  &idx, radix);
		if (numberString.length() != idx) {
			logger.error("Unexpect");
			logger.error("  end          = %d", numberString.length());
			logger.error("  idx          = %d \"%s\"", idx, numberString.substr(idx, numberString.length() - idx));
			logger.error("  string       = %d \"%s\"", string.length(), string.c_str());
			logger.error("  numberString = %d \"%s\"", numberString.length(), numberString.c_str());
			ERROR();
		}
		return (int32_t)ret;
	} catch (const std::invalid_argument& e) {
		logger.error("exception");
		logger.error("  name   = %s!", typeid(e).name());
		logger.error("  what   = %s!", e.what());
		logger.error("  string = %s!", string);
		ERROR();
	} catch (const std::out_of_range& e) {
		logger.error("exception");
		logger.error("  name   = %s!", typeid(e).name());
		logger.error("  what   = %s!", e.what());
		logger.error("  string = %s!", string);
		ERROR();
	}
}

std::string toHexString(int size, const uint8_t* data) {
	std::string string;
	for(int i = 0; i < size; i++) {
		string += std_sprintf("%02X",data[i]);
	}
	return string;
}
std::vector<uint8_t> fromHexString(const std::string& string) {
	// sanity check
	static std::regex hex("^([0-9a-fA-F]{2})+$");
	if (!std::regex_match(string, hex)) {
		logger.error("string %s!", string);
		ERROR()
	}

	std::vector<uint8_t> array;
	static std::regex hex2("[0-9a-fA-F]{2}");
	std::sregex_token_iterator begin(string.cbegin(), string.cend(), hex2, 0);
	std::sregex_token_iterator end;
	for(auto& i = begin; i != end; i++) {
		uint8_t value = (uint8_t)std::stoi(i->str(), nullptr, 16);
		array.push_back(value);
	}
	return array;
}


std::string readFile(const std::string& path) {
	std::ifstream ifs(path.c_str());
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	return buffer.str();
}

std::chrono::system_clock::time_point to_system_clock(std::chrono::steady_clock::time_point time_steady) {
	static auto base_system_ = std::chrono::system_clock::now();
	static auto base_steady_ = std::chrono::steady_clock::now();
	return base_system_ + std::chrono::duration_cast<std::chrono::microseconds>(time_steady - base_steady_);
}


uint16_t bitField(uint16_t word, int startBit, int stopBit) {
	const int MAX_BIT = 15;

	if (startBit < 0)        ERROR();
	if (stopBit  < 0)        ERROR();
	if (stopBit  < startBit) ERROR();
	if (MAX_BIT  < startBit) ERROR();
	if (MAX_BIT  < stopBit)  ERROR();

	int shift  = MAX_BIT - stopBit;
	int mask   = ((int)(1L << (stopBit - startBit + 1)) - 1) << shift;

	return (uint16_t)((word & mask) >> shift);
}


class MapInfo {
public:
	int      id;
	std::string path;
	int   	 fd;
	uint64_t size;
	void*    page;

	MapInfo() : id(count++), path(""), fd(0), size(0), page(0) {}

	static int count;
};
static std::map<void*, MapInfo>mapInfoMap;
int MapInfo::count = 0;

void* Util::mapFile  (const std::string& path, uint32_t& mapSize) {
	// sanity check
	if (!std::filesystem::exists(path)) {
		logger.error("unexpected path");
		logger.error("  path  {}", path);
		ERROR();
	}

	MapInfo mapInfo;
	mapInfo.path = path;

	CHECK_SYSCALL(mapInfo.fd, open(path.c_str(), O_RDWR))
	mapInfo.size = std::filesystem::file_size(path);
	mapInfo.page = mmap(nullptr, mapInfo.size, PROT_READ | PROT_WRITE, MAP_SHARED, mapInfo.fd, 0);
	if (mapInfo.page == MAP_FAILED) ERROR()
	
	mapInfoMap.insert({mapInfo.page, mapInfo});
	logger.info("mapFile    %d  size = %8X  path = %s", mapInfo.id, (uint32_t)mapInfo.size, mapInfo.path);

	mapSize = mapInfo.size;
	return mapInfo.page;
}
void  Util::unmapFile(void* page) {
	if (!mapInfoMap.contains(page)) {
		logger.error("unexpected page");
		logger.error("  page  %p", page);
		ERROR()
	}
	const MapInfo& mapInfo = mapInfoMap[page];

	logger.info("unmapFile  %d  size = %8X  path = %s", mapInfo.id, (uint32_t)mapInfo.size, mapInfo.path);

	int ret;
	CHECK_SYSCALL(ret, munmap(mapInfo.page, mapInfo.size))
	CHECK_SYSCALL(ret, close(mapInfo.fd))	

	mapInfoMap.erase(mapInfo.page);
}

// Time stuff
uint64_t Util::getSecondsSinceEpoch() {
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

std::string Util::toString(uint32_t unixTime) {
	time_t temp = unixTime;
    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%d-%02d-%02d %02d:%02d:%02d", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void Util::byteswap(uint16_t* source, uint16_t* dest, int size) {
	for(int i = 0; i < size; i++) {
		dest[i] = std::byteswap(source[i]);
	}
}


void std_vsprintf_(std::string& result, int bufferSize, const char* format, va_list ap) {
	char* buf = (char*)alloca(bufferSize);
	int ret = std::vsnprintf(buf, bufferSize, format, ap);
	if (bufferSize <= ret) {
		// failure
		// + 1 for trailing null character
		std_vsprintf_(result, ret + 1, format, ap);
	} else {
		// success
		result += buf;
	}
}
std::string std_vsprintf(const char* format, va_list ap) {
	std::string result;
	std_vsprintf_(result, STD_SPRINTF_DEFAULT_BUFFER_SIZE, format, ap);
	return result;
}
