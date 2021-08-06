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

#ifndef UTIL_H__
#define UTIL_H__

#include <log4cpp/Category.hh>

#include <signal.h>

#include <QtCore>
#include <QtGlobal>


#define DEBUG_TRACE() logger.debug("****  TRACE  %-20s %5d %s", __FUNCTION__, __LINE__, __FILE__)

class Error {
public:
	const char *func;
	const char *file;
	const int   line;

	Error(const char *func_, const char *file_, const int line_) : func(func_), file(file_), line(line_) {}
};

#define ERROR() { logger.fatal("ERROR %s %d %s", __FILE__, __LINE__, __FUNCTION__); logBackTrace(); throw Error(__FUNCTION__, __FILE__, __LINE__); }

class Abort {
public:
	const char *func;
	const char *file;
	const int line;

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


template<typename T>
auto std_sprintf_convert_(T&& value) {
    if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value) {
    	// std::string => const char*
        return (value).c_str();
    } else if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, QString>::value) {
    	// QTString => const char*
    	return (value).toUtf8().constData();
    } else {
    	// otherwise
    	return std::forward<T>(value);
    }
}

template <typename ... Args>
std::string std_sprintf_(const char* format, Args&& ... args) {
    int len = std::snprintf(nullptr, 0, format, args ...);
    char buf[(int)len + 1];
    std::snprintf(buf, len + 1, format, args ...);
    return std::string(buf);
}

template<typename ... Args>
std::string std_sprintf(const char* format, Args&& ... args) {
    return std_sprintf_(format, std_sprintf_convert_(std::forward<Args>(args)) ...);
}

class Logger {
public:
	static Logger getLogger(const char *name);

	static void pushPriority(log4cpp::Priority::Value newValue);
	static void popPriority();

    template<typename... Args> void debug(const char* format, Args&& ... args) const {
    	category->debug(format, std_sprintf_convert_(std::forward<Args>(args)) ...);
    }
    template<typename... Args> void info(const char* format, Args&& ... args) const {
     	category->info(format, std_sprintf_convert_(std::forward<Args>(args)) ...);
     }
    template<typename... Args> void warn(const char* format, Args&& ... args) const {
     	category->warn(format, std_sprintf_convert_(std::forward<Args>(args)) ...);
     }
    template<typename... Args> void error(const char* format, Args&& ... args) const {
    	category->error(format, std_sprintf_convert_(std::forward<Args>(args)) ...);
    }
    template<typename... Args> void fatal(const char* format, Args&& ... args) const {
    	category->fatal(format, std_sprintf_convert_(std::forward<Args>(args)) ...);
    }

private:
	log4cpp::Category* category;
	Logger(log4cpp::Category* category_) : category(category_) {}
};

int toIntMesaNumber(const QString& string);

// convert to utf8
#define TO_CSTRING(e) (e).toUtf8().constData()


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

#endif
