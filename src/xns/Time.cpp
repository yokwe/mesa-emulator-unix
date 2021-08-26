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
NameMap::Map<quint16> XNS::Time::Type::nameMap(NameMap::toString16u, {
	{REQUEST,  "REQUEST"},
	{RESPONSE, "RESPONSE"},
});


//
// XNS::Time::Direction
//
NameMap::Map<quint16> XNS::Time::Direction::nameMap(NameMap::toString16u, {
	{WEST, "WEST"},
	{EAST, "EAST"},
});


//
// XNS::Time::Tolerance
//
NameMap::Map<quint16> XNS::Time::Tolerance::nameMap(NameMap::toString16u, {
	{UNKNOWN, "UNKNOWN"},
	{MILLI,   "MILLI"},
});


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
void XNS::Time::set(const Response& newValue) {
	if (type == Type::RESPONSE) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Time::get(Response& newValue) const {
	if (type == Type::RESPONSE) {
		newValue = std::get<Response>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}

QString XNS::Time::toString() const {
	if (version.isCurrent()) {
		if (type == Type::REQUEST) {
//			return QString("%1 %2").arg(version.toString()).arg(type.toString());
			return QString("%1").arg(type.toString());
		} else if (type == Type::RESPONSE) {
			Response response;
			get(response);
//			return QString("%1 %2 %3").arg(version.toString()).arg(type.toString()).arg(response.toString());
			return QString("%1 %2").arg(type.toString()).arg(response.toString());
		} else {
			logger.error("Unexpected");
			logger.error("  type %s", type.toString());
			ERROR();
		}
	} else {
		logger.error("Unexpected");
		logger.error("  version %s", version.toString());
		ERROR();
	}
}
void XNS::Time::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, version);
	if (version.isCurrent()) {
		FROM_BYTE_BUFFER(bb, type);
		if (type == Type::REQUEST) {
			// do notheng
		} else if (type == Type::RESPONSE) {
			Response response;
			FROM_BYTE_BUFFER(bb, response);
			set(response);
		} else {
			logger.error("Unexpected");
			logger.error("  type %s", type.toString());
			ERROR();
		}
	} else {
		logger.error("Unexpected");
		logger.error("  version %s", version.toString());
		ERROR();
	}
}
void XNS::Time::toByteBuffer  (Buffer& bb) const {
	if (version.isCurrent()) {
		TO_BYTE_BUFFER(bb, version);
		TO_BYTE_BUFFER(bb, type);

		if (type == Type::REQUEST) {
			// to nothing
		} else if (type == Type::RESPONSE) {
			Response response;
			get(response);
			TO_BYTE_BUFFER(bb, response);
		} else {
			logger.error("Unexpected");
			logger.error("  type %s", type.toString());
			ERROR();
		}
	} else {
		logger.error("Unexpected");
		logger.error("  version %s", version.toString());
		ERROR();
	}
}
