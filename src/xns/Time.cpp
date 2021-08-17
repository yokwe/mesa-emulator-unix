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
// Time.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-time");

#include "Time.h"


//
// XNS::Time::XNSTime
//
QString XNS::Time::XNSTime::toString() const {
	qint64 unixTime = toUnixTime(value());
	QDateTime dateTime = QDateTime::fromSecsSinceEpoch(unixTime, Qt::UTC);
	return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}


//
// XNS::Time::Version
//
QString XNS::Time::Version::toString() const {
	return QString::asprintf("%d", value());
}


//
// XNS::Time::Type
//
QString XNS::Time::Type::toString() const {
	if (nameMap.contains(value())) {
		return nameMap[value()];
	} else {
		return QString::asprintf("%d", value());
	}
}
QMap<quint16, QString> XNS::Time::Type::initNameMap() {
	QMap<quint16, QString> ret;
	ret[REQUEST]  = "REQUEST";
	ret[RESPONSE] = "RESPONSE";
	return ret;
}
QMap<quint16, QString> XNS::Time::Type::nameMap  = initNameMap();


//
// XNS::Time::Direction
//
QString XNS::Time::Direction::toString() const {
	if (nameMap.contains(value())) {
		return nameMap[value()];
	} else {
		return QString::asprintf("%d", value());
	}
}
QMap<quint16, QString> XNS::Time::Direction::initNameMap() {
	QMap<quint16, QString> ret;
	ret[WEST] = "WEST";
	ret[EAST] = "EAST";
	return ret;
}
QMap<quint16, QString> XNS::Time::Direction::nameMap  = initNameMap();


//
// XNS::Time::Tolerance
//
QString XNS::Time::Tolerance::toString() const {
	if (nameMap.contains(value())) {
		return nameMap[value()];
	} else {
		return QString::asprintf("%d", value());
	}
}
QMap<quint16, QString> XNS::Time::Tolerance::initNameMap() {
	QMap<quint16, QString> ret;
	ret[UNKNOWN] = "UNKNOWN";
	ret[MILLI]   = "MILLI";
	return ret;
}
QMap<quint16, QString> XNS::Time::Tolerance::nameMap  = initNameMap();


//
// XNS::Time::Response
//
QString XNS::Time::Response::toString() const {
	return QString("%1  %2-%3-%4  %5-%6 %7-%8").
		arg(time.value()).
		arg(offsetDirection.toString()).arg(offsetHours.value()).arg(offsetMinutes.value()).
		arg(dstStart.value()).arg(dstEnd.value()).
		arg(tolerance.toString()).arg(toleranceValue.value());
}

void XNS::Time::Response::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, time);
	FROM_BYTE_BUFFER(bb, offsetDirection);
	FROM_BYTE_BUFFER(bb, offsetHours);
	FROM_BYTE_BUFFER(bb, offsetMinutes);
	FROM_BYTE_BUFFER(bb, dstStart);
	FROM_BYTE_BUFFER(bb, dstEnd);
	FROM_BYTE_BUFFER(bb, tolerance);
	FROM_BYTE_BUFFER(bb, toleranceValue);
}
void XNS::Time::Response::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, time);
	TO_BYTE_BUFFER(bb, offsetDirection);
	TO_BYTE_BUFFER(bb, offsetHours);
	TO_BYTE_BUFFER(bb, offsetMinutes);
	TO_BYTE_BUFFER(bb, dstStart);
	TO_BYTE_BUFFER(bb, dstEnd);
	TO_BYTE_BUFFER(bb, tolerance);
	TO_BYTE_BUFFER(bb, toleranceValue);
}


//
// XNS::Time
//
QString XNS::Time::toString() const {
	if (version.isCurrent()) {
		if (type == Type::REQUEST) {
			return QString("%1 %2 %3").arg(version.toString()).arg(type.toString()).arg(std::get<struct Request>(body).toString());
		} else if (type == Type::RESPONSE) {
			return QString("%1 %2 %3").arg(version.toString()).arg(type.toString()).arg(std::get<struct Response>(body).toString());
		} else {
			logger.error("Unexpected");
			logger.error("  type %d", type.value());
			ERROR();
		}
	} else {
		logger.error("Unexpected");
		logger.error("  version %d", version.value());
		ERROR();
	}
}
void XNS::Time::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, version);
	if (version.isCurrent()) {
		FROM_BYTE_BUFFER(bb, type);
		if (type == Type::REQUEST) {
			struct Request request;
			FROM_BYTE_BUFFER(bb, request);
			body = request;
		} else if (type == Type::RESPONSE) {
			struct Response response;
			FROM_BYTE_BUFFER(bb, response);
			body = response;
		} else {
			logger.error("Unexpected");
			logger.error("  type %d", type.value());
			ERROR();
		}
	} else {
		logger.error("Unexpected");
		logger.error("  version %d", version.value());
		ERROR();
	}
}
void XNS::Time::toByteBuffer  (Buffer& bb) const {
	if (version.isCurrent()) {
		TO_BYTE_BUFFER(bb, version);
		TO_BYTE_BUFFER(bb, type);

		if (type == Type::REQUEST) {
			struct Request request = std::get<struct Request>(body);
			TO_BYTE_BUFFER(bb, request);
		} else if (type == Type::RESPONSE) {
			struct Response response = std::get<struct Response>(body);
			TO_BYTE_BUFFER(bb, response);
		} else {
			logger.error("Unexpected");
			logger.error("  type %d", type.value());
			ERROR();
		}
	} else {
		logger.error("Unexpected");
		logger.error("  version %d", version.value());
		ERROR();
	}
}
