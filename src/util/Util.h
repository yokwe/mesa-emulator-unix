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
// Util.h
//

#pragma once

#include <chrono>
#include <string>
#include <cstdint>
#include <set>
#include <source_location>

#include <log4cxx/logger.h>


//
// helper template for Logger
//
template <typename ... Args>
void std_sprintf_(std::string& result, int bufferSize, const char* format, Args&& ... args) {
	char* buf = (char*)alloca(bufferSize);
	int ret = std::snprintf(buf, bufferSize, format, args ...);
	if (bufferSize <= ret) {
		// failure
		// + 1 for trailing null character
		std_sprintf_(result, ret + 1, format, args ...);
	} else {
		// success
		result += buf;
	}
}
template<typename T>
auto std_sprintf_convert_(T&& value) {
	constexpr auto is_std_string = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;

	// handle std::string properly
	if constexpr (is_std_string) {
		return (value).c_str();
	} else {
		return std::forward<T>(value);
	}
}

#define STD_SPRINTF_DEFAULT_BUFFER_SIZE 512

template<typename ... Args>
std::string std_sprintf(const char* format, Args&& ... args) {
	std::string result;
    std_sprintf_(result, STD_SPRINTF_DEFAULT_BUFFER_SIZE, format, std_sprintf_convert_(std::forward<Args>(args)) ...);
    return result;
}

std::string std_vsprintf(const char* format, va_list ap);

class Logger {
public:
	Logger(const char* name);
	Logger(const Logger& that) : myLogger(that.myLogger) {}

	enum class Level {
		DEBUG, INFO, WARN, ERROR, FATAL, OFF,
	};

	static void pushLevel(Level newValue = Level::OFF);
	static void popLevel();

	// std::string
	void debug(const std::string& string) const {
		myLogger->debug(string);
	}
	void info(const std::string& string) const {
		myLogger->info(string);
	}
	void warn(const std::string& string) const{
		myLogger->warn(string);
	}
	void error(const std::string& string) const {
		myLogger->error(string);
	}
	void fatal(const std::string& string) const {
		myLogger->fatal(string);
	}

	// const char*
	void debug(const char* string) const {
		myLogger->debug(std::string(string));
	}
	void info(const char* string) const {
		myLogger->info(std::string(string));
	}
	void warn(const char* string) const {
		myLogger->warn(std::string(string));
	}
	void error(const char* string) const {
		myLogger->error(std::string(string));
	}
	void fatal(const char* string) const {
		myLogger->fatal(std::string(string));
	}


    template<typename... Args> void debug(const char* format, Args&& ... args) const {
    	myLogger->debug(std_sprintf(format, args...));
    }
    template<typename... Args> void info(const char* format, Args&& ... args) const {
    	myLogger->info(std_sprintf(format, args...));
     }
    template<typename... Args> void warn(const char* format, Args&& ... args) const {
    	myLogger->warn(std_sprintf(format, args...));
     }
    template<typename... Args> void error(const char* format, Args&& ... args) const {
    	myLogger->error(std_sprintf(format, args...));
    }
    template<typename... Args> void fatal(const char* format, Args&& ... args) const {
    	myLogger->fatal(std_sprintf(format, args...));
    }

	static void stopMessageUntilMP(uint16_t newValue) {
		stopMessageUntilMPSet.insert(newValue);
		Logger::pushLevel(Logger::Level::FATAL);
	}
	static void mp_observer(uint16_t mp);
private:
    log4cxx::LoggerPtr myLogger;
	static std::set<uint16_t> stopMessageUntilMPSet;
};

class LogSourceLocation {
private:
	static const char* to_simple_path(const char* path) {
		auto pos = strstr(path, "/src");
		return (pos == NULL) ? path : pos + 1;
	}
public:
	static std::string toString(std::source_location location, const char* prefix = "") {
		return std_sprintf("%s%5d  %s  --  %s", prefix, location.line(), to_simple_path(location.file_name()), location.function_name());
	}
	static void debug(const Logger& logger, std::source_location location, const char* prefix = "") {
		logger.debug(toString(location, prefix));
	}
	static void info(const Logger& logger, std::source_location location, const char* prefix = "") {
		logger.info(toString(location, prefix));
	}
	static void warn(const Logger& logger, std::source_location location, const char* prefix = "") {
		logger.warn(toString(location, prefix));
	}
	static void error(const Logger& logger, std::source_location location, const char* prefix = "") {
		logger.error(toString(location, prefix));
	}
	static void fatal(const Logger& logger, std::source_location location, const char* prefix = "") {
		logger.fatal(toString(location, prefix));
	}

	static void trace(const Logger& logger, std::source_location location = std::source_location::current()) {
		debug(logger, location, "**** TRACE ");
	}
};


#define DEBUG_TRACE() { LogSourceLocation::trace(logger); }


struct ErrorError {
	const std::source_location location;
	ErrorError(std::source_location location_ = std::source_location::current()) : location(location_) {}
};

#define ERROR() { ErrorError errorError; LogSourceLocation::fatal(logger, errorError.location, "ERROR  "); throw errorError; }

class Abort {
public:
	const std::source_location location;
	Abort(std::source_location location_ = std::source_location::current()) : location(location_) {}
};
#define ERROR_Abort() { throw Abort(); }

class RequestReschedule {
public:
	const std::source_location location;
	RequestReschedule(std::source_location location_ = std::source_location::current()) : location(location_) {}
};
#define ERROR_RequestReschedule() { throw RequestReschedule(); }

void logBackTrace();
void setSignalHandler(int signum = SIGSEGV);

std::string demangle(const char* mangled);

int32_t toIntMesaNumber(const std::string& string);

std::string toHexString(int size, const uint8_t* data);
std::vector<uint8_t> fromHexString(const std::string& string);

// https://stackoverflow.com/questions/7276826/format-number-with-commas-in-c
template<class T>
std::string formatWithCommas(T value) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

std::string readFile(const std::string& path);

// https://stackoverflow.com/questions/18361638/converting-steady-clocktime-point-to-time-t
std::chrono::system_clock::time_point to_system_clock(std::chrono::steady_clock::time_point steady_time);


// convert to utf8
#define TO_CSTRING(e) (e).toUtf8().constData()

// helper macro for syscall
#define LOG_ERRNO(errNo)               { logger.error( "errno = %d  \"%s\"", errNo, strerror(errNo)); }
#define LOG_SYSCALL(retVar, syscall)   { retVar = syscall; if (retVar < 0) { int errNo = errno; logger.error("%s = %d", #syscall, retVar);  LOG_ERRNO(errNo)         } }
#define CHECK_SYSCALL(retVar, syscall) { retVar = syscall; if (retVar < 0) { int errNo = errno; logger.error("%s = %d", #syscall, retVar);  LOG_ERRNO(errNo) ERROR() } }

#define LOG_SYSCALL2(retVar, errNo, syscall)   { retVar = syscall; errNo = errno; if (retVar < 0) { logger.error("%s = %d", #syscall, retVar);  LOG_ERRNO(errNo)         } }
#define CHECK_SYSCALL2(retVar, errNo, syscall) { retVar = syscall; errNo = errno; if (retVar < 0) { logger.error("%s = %d", #syscall, retVar);  LOG_ERRNO(errNo) ERROR() } }

// Helper macro to make toString for enum class
#define TO_STRING_PROLOGUE(e) \
	typedef e ENUM; \
	static std::map<ENUM, std::string> map({
#define MAP_ENTRY(m) {ENUM::m, #m},
#define TO_STRING_EPILOGUE \
	}); \
	if (map.contains(value)) { \
		return map[value]; \
	} else { \
		logger.error("Unknown value = %d", (int)value); \
		ERROR(); \
		return std_sprintf("%d", (int)value); \
	}

// bitFiled is used in symbols
uint16_t bitField(uint16_t word, int startBit, int stopBit);
inline uint16_t bitField(uint16_t word, int startBit) {
	return bitField(word, startBit, startBit);
}

// count number of element in array
#define COUNT_ELEMENT(array) ((sizeof(array)) / (sizeof(array[0])))

// get build directory from Environment variable BUILD_DIR
inline const char* getBuildDir() {
	return BUILD_DIR;
}

class Util {
public:
	// Time duraion for one second
	static constexpr std::chrono::milliseconds ONE_SECOND = std::chrono::seconds(1);

	// misc functions
	static uint64_t getSecondsSinceEpoch();

	static uint32_t getUnixTime() {
		return (uint32_t)getSecondsSinceEpoch();
	}
	static uint32_t getMesaTime() {
		return toMesaTime(getUnixTime());
	}

	static void*   mapFile  (const std::string& path, uint32_t& mapSize);
	static void    unmapFile(void* page);

	static void    byteswap  (uint16_t* source, uint16_t* dest, int size);

	//From System.mesa
	//-- Time of day
	//
	//GreenwichMeanTime: TYPE = RECORD [LONG CARDINAL];
	//-- A greenwich mean time t represents the time which is t-gmtEpoch seconds after
	//-- midnight, 1 January 1968, the time chosen as the epoch or beginning of the Pilot
	//-- time standard.  Within the range in which they overlap, the Alto and Pilot time
	//-- standards assign identical bit patterns, but the Pilot standard runs an additional
	//-- 67 years before overflowing.
	//-- Greenwich mean times should be compared directly only for equality; to find which of
	//-- two gmt's comes first, apply SecondsSinceEpoch to each and compare the result.  If t2
	//-- is a gmt known to occur after t1, then t2-t1 is the seconds between t1 and t2.  If t
	//-- is a gmt, then System.GreenwichMeanTime[t+60] is the gmt one minute after t.
	//gmtEpoch: GreenwichMeanTime = [2114294400];
	//-- = (67 years * 365 days + 16 leap days) * 24 hours * 60 minutes * 60 seconds
	//GetGreenwichMeanTime: PROCEDURE RETURNS [gmt: GreenwichMeanTime];

	// Unix Time Epoch  1970-01-01 00:00:00
	// Mesa Time Epoch  1968-01-01 00:00:00
	//   Difference between above 2 date is 731 days.
	static const uint32_t EPOCH_DIFF = (uint32_t)2114294400 + (uint32_t)(731 * 60 * 60 * 24);

	static uint32_t toMesaTime(uint32_t unixTime) {
		return unixTime + EPOCH_DIFF;
	}
	static uint32_t toUnixTime(uint32_t mesaTime) {
		return mesaTime - EPOCH_DIFF;
	}
};
