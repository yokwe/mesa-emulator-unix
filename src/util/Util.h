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

#include <string>
#include <cstdint>

#include <alloca.h>

#include <log4cxx/logger.h>


#define DEBUG_TRACE() logger.debug("****  TRACE  %-20s %5d %s", __FUNCTION__, __LINE__, __FILE__)

class ErrorError {
public:
	const char *func;
	const char *file;
	const int   line;

	ErrorError(const char *func_, const char *file_, const int line_) : func(func_), file(file_), line(line_) {}
};

#define ERROR() { logger.fatal("ERROR %s %d %s", __FILE__, __LINE__, __FUNCTION__); logBackTrace(); throw ErrorError(__FUNCTION__, __FILE__, __LINE__); }

class Abort {
public:
	const char *func;
	const char *file;
	const int   line;

	Abort(const char *func_, const char *file_, const int line_) : func(func_), file(file_), line(line_) {}
};
#define ERROR_Abort() throw Abort(__FUNCTION__, __FILE__, __LINE__)

class RequestReschedule {
public:
	const char *func;
	const char *file;
	const int line;

	RequestReschedule(const char *func_, const char *file_, const int line_) : func(func_), file(file_), line(line_) {}
};
#define ERROR_RequestReschedule() throw RequestReschedule(__FUNCTION__, __FILE__, __LINE__)

void logBackTrace();
void setSignalHandler(int signum = SIGSEGV);

std::string demangle(const char* mangled);


template<typename T>
auto std_sprintf_convert_(T&& value) {
	constexpr auto is_std_string     = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;

	// handle std::string properly
	if constexpr (is_std_string) {
		return (value).c_str();
	} else {
		return std::forward<T>(value);
	}
}

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

#define STD_SPRINTF_DEFAULT_BUFFER_SIZE 512

template<typename ... Args>
std::string std_sprintf(const char* format, Args&& ... args) {
	std::string result;
    std_sprintf_(result, STD_SPRINTF_DEFAULT_BUFFER_SIZE, format, std_sprintf_convert_(std::forward<Args>(args)) ...);
    return result;
}

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

private:
    log4cxx::LoggerPtr myLogger;
};

int32_t toIntMesaNumber(const std::string& string);

std::string toHexString(int size, const uint8_t* data);

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
		return std::string("%1").arg((int)value); \
	}

// bitFiled is used in symbols
uint16_t bitField(uint16_t word, int startBit, int stopBit);
__attribute__((always_inline)) static inline uint16_t bitField(uint16_t word, int startBit) {
	return bitField(word, startBit, startBit);
}

// count number of element in array
#define COUNT_ELEMENT(array) ((sizeof(array)) / (sizeof(array[0])))

// get build directory from Environment variable BUILD_DIR
const char* getBuildDir();

class Util {
public:
	// Time duraion for one second
	static constexpr std::chrono::milliseconds ONE_SECOND = std::chrono::seconds(1);

	// misc functions
	static uint64_t getSecondsFromEpoch();
	static uint64_t getMilliSecondsFromEpoch();
	static uint64_t getMicroSecondsFromEpoch();

	static uint32_t getMicroTime() {
		return (uint32_t)getMicroSecondsFromEpoch();
	}
	static uint32_t getUnixTime() {
		return (uint32_t)getSecondsFromEpoch();
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
