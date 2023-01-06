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
// Util.cpp
//

#include <log4cpp/PropertyConfigurator.hh>
#include <execinfo.h>

#include "./Util.h"
static const Logger logger = Logger::getLogger("util");

void logBackTrace() {
	const int BUFFER_SIZE = 100;
	void *buffer[BUFFER_SIZE];

	// get void*'s for all entries on the stack
	int size = backtrace(buffer, BUFFER_SIZE);

	char **msg = backtrace_symbols(buffer, size);

	// print out all the frames to fno
	for(int i = 0; i < size; i++) {
		logger.fatal("%3d %s", i, msg[i]);
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



// Logger
static void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& qString) {
	std::string string = qString.toStdString();

	if (context.line == 0) {
		switch(type) {
		case QtDebugMsg:
			logger.debug(string);
			break;
		case QtInfoMsg:
			logger.info(string);
			break;
		case QtWarningMsg:
			logger.warn(string);
			break;
		case QtCriticalMsg:
			logger.error(string);
			break;
		case QtFatalMsg:
			logger.fatal(string);
			break;
		default:
			ERROR();
		}
	} else {
		switch(type) {
		case QtDebugMsg:
			logger.debug("%s %d %s  %s  %s", context.file, context.line, context.function, context.category, string.c_str());
			break;
		case QtInfoMsg:
			logger.info("%s %d %s  %s  %s", context.file, context.line, context.function, context.category, string.c_str());
			break;
		case QtWarningMsg:
			logger.warn("%s %d %s  %s  %s", context.file, context.line, context.function, context.category, string.c_str());
			break;
		case QtCriticalMsg:
			logger.error("%s %d %s  %s  %s", context.file, context.line, context.function, context.category, string.c_str());
			break;
		case QtFatalMsg:
			logger.fatal("%s %d %s  %s  %s", context.file, context.line, context.function, context.category, string.c_str());
			break;
		default:
			ERROR();
		}
	}
}
QtMessageHandler Logger::getQtMessageHandler() {
	return qtMessageHandler;
}

Logger Logger::getLogger(const char *name) {
	static char *fileName = getenv("LOG_CONFIG");
	if (fileName == nullptr) {
		log4cpp::Category::getRoot().fatal("LOG_CONFIG is nullptr");
		exit(1);
	}

	if (fileName) log4cpp::PropertyConfigurator::configure(fileName);
	if (log4cpp::Category::exists(name)) {
		log4cpp::Category::getRoot().fatal("Duplicate logger name = %s", name);
		exit(1);
	}

	(void)log4cpp::Category::getInstance(name);
	Logger logger(log4cpp::Category::exists(name));
	return logger;
}

static void setLoggerPriority(log4cpp::Priority::Value newValue) {
	for(auto i: *log4cpp::Category::getCurrentCategories()) {
		i->setPriority(newValue);
	}
}

static QStack<log4cpp::Priority::Value> loggerPriorityStack;
void Logger::pushPriority(log4cpp::Priority::Value newValue) {
	loggerPriorityStack.push(log4cpp::Category::getRootPriority());
	setLoggerPriority(newValue);
}
void Logger::popPriority() {
	if (loggerPriorityStack.empty()) return;

	log4cpp::Priority::Value newValue = loggerPriorityStack.pop();
	setLoggerPriority(newValue);
}


int toIntMesaNumber(const QString& string) {
	bool ok;
	quint32 ret;

	if (string.startsWith("0x") || string.startsWith("0X")) {
		ret = string.toInt(&ok, 0); // to handle string starts with 0x, use 0 for base
	} else if (string.endsWith("B")) {
		// MESA style octal number
		ret = string.left(string.length() - 1).toInt(&ok, 8);
	} else if (string.endsWith("H")) {
		// MESA style hexadecimal number
		ret = string.left(string.length() - 1).toInt(&ok, 16);
	} else {
		ret = string.toInt(&ok, 0); // to handle string starts with 0x, use 0 for base
	}

	if (!ok) {
		logger.error("Unexpected");
		logger.error("  string %s!", string.toStdString());
		ERROR();
	}

	return ret;
}

bool startsWith(const std::string_view& string, const std::string_view& literal) {
	const size_t string_size(string.size());
	const size_t literal_size(literal.size());

	if (string_size < literal_size) {
		return false;
	} else {
		std::string_view string_view(string.data(), literal_size);
		return string_view == literal;
	}
}
bool endsWith(const std::string_view& string, const std::string_view& literal) {
	const size_t string_size(string.size());
	const size_t literal_size(literal.size());

	if (string_size < literal_size) {
		return false;
	} else {
		std::string_view string_view(string.data() + (string_size - literal_size), literal_size);
		return string_view == literal;
	}
}
int toIntMesaNumber(const std::string& string) {
	int radix = 0;
	std::string numberString;

	if (startsWith(string, "0x") || startsWith(string, "0X")) {
		// c style hexadecimal
		radix = 16;
		numberString = string.substr(2, string.length() - 2);
	} else if (startsWith(string, "0")) {
		// c style octal
		radix = 8;
		numberString = string.substr(1, string.length() - 1);
	} else if (endsWith(string, "H")) {
		// mesa style hexadecimal
		radix = 16;
		numberString = string.substr(0, string.length() - 1);
	} else if (endsWith(string, "B")) {
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
		int ret = std::stoi(numberString,  &idx, radix);
		if (numberString.length() != idx) {
			logger.error("Unexpect");
			logger.error("  end          = %d", numberString.length());
			logger.error("  idx          = %d \"%s\"", idx, numberString.substr(idx, numberString.length() - idx));
			logger.error("  string       = %d \"%s\"", string.length(), string.c_str());
			logger.error("  numberString = %d \"%s\"", numberString.length(), numberString.c_str());
			ERROR();
		}
		return ret;
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

QString toHexString(int size, const quint8* data) {
	QString ret;

	for(int i = 0; i < size; i++) {
		ret += QString::asprintf("%02X", data[i]);
	}
	return ret;
}


quint16 bitField(quint16 word, int startBit, int stopBit) {
	const int MAX_BIT = 15;

	if (startBit < 0)        ERROR();
	if (stopBit  < 0)        ERROR();
	if (stopBit  < startBit) ERROR();
	if (MAX_BIT  < startBit) ERROR();
	if (MAX_BIT  < stopBit)  ERROR();

	int shift  = MAX_BIT - stopBit;
	int mask   = ((int)(1L << (stopBit - startBit + 1)) - 1) << shift;

	return (quint16)((word & mask) >> shift);
}


class MapInfo {
public:
	int     id;
	QFile   file;
	quint64 size;
	void*   page;

	MapInfo(QString path) : id(count++), file(path), size(0), page(0) {}

	static int count;
};
static QMap<void*, MapInfo*>allMap;
int MapInfo::count = 0;

void* Util::mapFile  (const QString& path, quint32& mapSize) {
	MapInfo* mapInfo = new MapInfo(path);

	if (!mapInfo->file.exists()) {
		logger.fatal("%s  file.exists returns false.  path = %s", __FUNCTION__, path.toStdString());
		ERROR();
	}
	mapInfo->size = mapInfo->file.size();
	mapSize = (quint32)mapInfo->size;

	bool ok = mapInfo->file.open(QIODevice::ReadWrite);
	if (!ok) {
		logger.fatal("file.open returns false.  error = %s", mapInfo->file.errorString().toStdString());
		ERROR();
	}
	mapInfo->page = (void*)mapInfo->file.map(0, mapInfo->size);
	if (mapInfo->page == 0) {
		logger.fatal("file.map returns 0.  error  = %s", mapInfo->file.errorString().toStdString());
		ERROR();
	}

	allMap[mapInfo->page] = mapInfo;
	logger.info("mapFile    %d  size = %8X  path = %s", mapInfo->id, (quint32)mapInfo->size, mapInfo->file.fileName().toStdString());

	return mapInfo->page;
}
void  Util::unmapFile(void* page) {
	if (!allMap.contains(page)) {
		logger.fatal("%s page = %p", __FUNCTION__, page);
		ERROR();
	}
	MapInfo* mapInfo = allMap[page];

	logger.info("unmapFile  %d  size = %8X  path = %s", mapInfo->id, (quint32)mapInfo->size, mapInfo->file.fileName().toStdString());

	if (!mapInfo->file.unmap((uchar*)(mapInfo->page))) {
		logger.fatal("file.unmap returns false.  error = %s", mapInfo->file.errorString().toStdString());
		ERROR();
	}

	mapInfo->file.close();

	delete mapInfo;

	allMap.remove(page);
}

// Time stuff
namespace {
	class QThreadWrpper : public QThread {
	public:
		static void msleep(quint32 msec) {
			QThread::msleep(msec);
		}
	};
}

static int elapsedTimer_needStart = 1;
static QElapsedTimer elapsedTimer;
quint32 Util::getMicroTime() {
	if (elapsedTimer_needStart) {
		elapsedTimer.start();
		elapsedTimer_needStart = 0;
	}
	quint64 time = elapsedTimer.nsecsElapsed();
	// convert from nanoseconds to microseconds
	return (quint32)(time / 1000);
}

quint32 Util::getUnixTime() {
	return QDateTime::currentSecsSinceEpoch();
}
void Util::msleep(quint32 milliSeconds) {
	QThreadWrpper::msleep(milliSeconds);
}

void Util::toBigEndian(quint16* source, quint16* dest, int size) {
	qToBigEndian<quint16>(source, size, dest);
}
void Util::fromBigEndian(quint16* source, quint16* dest, int size) {
	qFromBigEndian<quint16>(source, size, dest);
}

// get build directory from Environment variable BUILD_DIR
const char* getBuildDir() {
	char *buildDir = getenv("BUILD_DIR");
	return buildDir == nullptr ? "." : buildDir;
}

