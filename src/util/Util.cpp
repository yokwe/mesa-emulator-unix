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

#include "./Util.h"

#include <log4cpp/PropertyConfigurator.hh>

#include <execinfo.h>

static log4cpp::Category& logger = Logger::getLogger("util");

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
log4cpp::Category& Logger::getLogger(const char *name) {
	char *fileName = getenv("LOG4CPP_CONFIG");
	if (fileName) log4cpp::PropertyConfigurator::configure(fileName);
	if (log4cpp::Category::exists(name)) {
		log4cpp::Category::getRoot().fatal("Duplicate logger name = %s", name);
		exit(1);
	}
	return log4cpp::Category::getInstance(name);
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


class MapInfo {
public:
	int     id;
	QFile   file;
	quint64 size;
	void*   page;

	MapInfo(QString path) : id(count++), file(path.toLatin1().constData()), size(0), page(0) {}

	static int count;
};
static QMap<void*, MapInfo*>allMap;
int MapInfo::count = 0;

void* Util::mapFile  (const QString& path, quint32& mapSize) {
	MapInfo* mapInfo = new MapInfo(path);

	if (!mapInfo->file.exists()) {
		logger.fatal("%s  file.exists returns false.  path = %s", __FUNCTION__, path.toLatin1().constData());
		ERROR();
	}
	mapInfo->size = mapInfo->file.size();
	mapSize = (quint32)mapInfo->size;

	bool ok = mapInfo->file.open(QIODevice::ReadWrite);
	if (!ok) {
		logger.fatal("file.open returns false.  error = %s", qPrintable(mapInfo->file.errorString()));
		ERROR();
	}
	mapInfo->page = (void*)mapInfo->file.map(0, mapInfo->size);
	if (mapInfo->page == 0) {
		logger.fatal("file.map returns 0.  error  = %s", qPrintable(mapInfo->file.errorString()));
		ERROR();
	}

	allMap[mapInfo->page] = mapInfo;
	logger.info("mapFile    %d  size = %8X  path = %s", mapInfo->id, (quint32)mapInfo->size, qPrintable(mapInfo->file.fileName()));

	return mapInfo->page;
}
void  Util::unmapFile(void* page) {
	if (!allMap.contains(page)) {
		logger.fatal("%s page = %p", __FUNCTION__, page);
		ERROR();
	}
	MapInfo* mapInfo = allMap[page];

	logger.info("unmapFile  %d  size = %8X  path = %s", mapInfo->id, (quint32)mapInfo->size, qPrintable(mapInfo->file.fileName()));

	if (!mapInfo->file.unmap((uchar*)(mapInfo->page))) {
		logger.fatal("file.unmap returns false.  error = %s", qPrintable(mapInfo->file.errorString()));
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

