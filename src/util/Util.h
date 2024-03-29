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

//
// Util.h
//

#pragma once

#include <log4cpp/Category.hh>

#include <signal.h>
#include <cxxabi.h>
#include <type_traits>
#include <tuple>
#include <string>
#include <functional>

#include <QtCore>
#include <QtGlobal>


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
	constexpr auto is_std_string = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value;
	constexpr auto is_qstring = std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, QString>::value;

	// comment out line below to detect QString
#ifdef STD_SPRINTF_STOP_COMPILE_IF_VALUE_IS_QSTRING
	static_assert(!is_qstring, "value is QString");
#endif
	if constexpr (is_std_string) {
		return (value).c_str();
	} else if constexpr (is_qstring) {
		return (value).toUtf8().constData();
	} else {
		return std::forward<T>(value);
	}
}

template <typename ... Args>
void std_sprintf_(std::string& result, int bufferSize, const char* format, Args&& ... args) {
	char buf[bufferSize];
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
	static Logger getLogger(const char *name);

	static void pushPriority(log4cpp::Priority::Value newValue);
	static void popPriority();

	// To redirect qDebug() qInfo() and qWarning() to log4cpp, call installQtMessageHandler
	static QtMessageHandler getQtMessageHandler();
	static void installQtMessageHandler() {
		qInstallMessageHandler(getQtMessageHandler());
	}

	// std::string
	void debug(const std::string& string) const {
		category->debug(string);
	}
	void info(const std::string& string) const {
		category->info(string);
	}
	void warn(const std::string& string) const{
		category->warn(string);
	}
	void error(const std::string& string) const {
		category->error(string);
	}
	void fatal(const std::string& string) const {
		category->fatal(string);
	}

	// const char*
	void debug(const char* string) const {
		category->debug(std::string(string));
	}
	void info(const char* string) const {
		category->info(std::string(string));
	}
	void warn(const char* string) const {
		category->warn(std::string(string));
	}
	void error(const char* string) const {
		category->error(std::string(string));
	}
	void fatal(const char* string) const {
		category->fatal(std::string(string));
	}


    template<typename... Args> void debug(const char* format, Args&& ... args) const {
    	category->debug(std_sprintf(format, args...));
    }
    template<typename... Args> void info(const char* format, Args&& ... args) const {
     	category->info(std_sprintf(format, args...));
     }
    template<typename... Args> void warn(const char* format, Args&& ... args) const {
     	category->warn(std_sprintf(format, args...));
     }
    template<typename... Args> void error(const char* format, Args&& ... args) const {
    	category->error(std_sprintf(format, args...));
    }
    template<typename... Args> void fatal(const char* format, Args&& ... args) const {
    	category->fatal(std_sprintf(format, args...));
    }

private:
	log4cpp::Category* category;
	Logger(log4cpp::Category* category_) : category(category_) {}
};

int toIntMesaNumber(const QString& string);
int toIntMesaNumber(const std::string& string);

bool startsWith(const std::string_view& string, const std::string_view& literal);
bool endsWith  (const std::string_view& string, const std::string_view& literal);

QString toHexString(int size, const quint8* data);

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
	static QMap<ENUM, QString> map({
#define MAP_ENTRY(m) {ENUM::m, #m},
#define TO_STRING_EPILOGUE \
	}); \
	if (map.contains(value)) { \
		return map[value]; \
	} else { \
		logger.error("Unknown value = %d", (int)value); \
		ERROR(); \
		return QString("%1").arg((int)value); \
	}

// bitFiled is used in symbols
quint16 bitField(quint16 word, int startBit, int stopBit);
__attribute__((always_inline)) static inline quint16 bitField(quint16 word, int startBit) {
	return bitField(word, startBit, startBit);
}

// count number of element in array
#define COUNT_ELEMENT(array) ((sizeof(array)) / (sizeof(array[0])))

// get build directory from Environment variable BUILD_DIR
const char* getBuildDir();

class Util {
public:
	// misc functions
	static quint32 getMicroTime();
	static void    msleep(quint32 milliSeconds);
	static quint32 getUnixTime();

	static void*   mapFile  (const QString& path, quint32& mapSize);
	static void    unmapFile(void* page);

	static void    toBigEndian  (quint16* source, quint16* dest, int size);
	static void    fromBigEndian(quint16* source, quint16* dest, int size);

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
	static const quint32 EPOCH_DIFF = (quint32)2114294400 + (quint32)(731 * 60 * 60 * 24);

	static quint32 toMesaTime(quint32 unixTime) {
		return unixTime + EPOCH_DIFF;
	}
	static quint32 toUnixTime(quint32 mesaTime) {
		return mesaTime - EPOCH_DIFF;
	}
};


//
// trait_function
//
template <typename T>
struct trait_function : trait_function<decltype(&T::operator())> {};
template <typename C, typename R, typename... A>
struct trait_function<R(C::*)(A...) const> {
	enum {type = 100};

	// use enum to define type not constant
	enum {arity = sizeof...(A)};
	// return type
	using ret_type = R;
	// argument type as std::tuple<>
	using arg_type = std::tuple<A...>;
};
template <typename R, typename... A>
struct trait_function<R(*)(A...)> {
	enum {type = 200};

	// use enum to define type not constant
	enum {arity = sizeof...(A)};
	// return type
	using ret_type = R;
	// argument type as std::tuple<>
	using arg_type = std::tuple<A...>;
};
#if 0
using trait = trait_function<T>;
logger.info("trait    %s", demangle(typeid(trait)));
logger.info("type     %d", trait::type);
logger.info("arity    %d", trait::arity);
logger.info("ret_type %s", demangle(typeid(typename trait::ret_type)));
using args0 = typename std::tuple_element<0, typename trait::arg_type>;
logger.info("arg0     %s", demangle(typeid(typename args0::type)));
#endif
