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
// Courier.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-courier");

#include "Courier.h"


//
// XNS::Courier::ProtocolType
//
NameMap::Map<quint16> XNS::Courier::ProtocolType::nameMap(NameMap::toString16u, {
	{PROTOCOL2,  "PROTOCOL2"},
	{PROTOCOL3,  "PROTOCOL3"},
});


//
// XNS::Courier::MessageType
//
NameMap::Map<quint16> XNS::Courier::MessageType::nameMap(NameMap::toString16u, {
	{CALL,   "CALL"},
	{REJECT, "REJECT"},
	{RETURN, "RETURN"},
	{ABORT,  "ABORT"},
});


//
// XNS::Courier::RejectCode
//
NameMap::Map<quint16> XNS::Courier::RejectCode::nameMap(NameMap::toString16u, {
	{NO_SUCH_PROGRAM,   "NO_SUCH_PROGRAM"},
	{NO_SUCH_VERSION,   "NO_SUCH_VERSION"},
	{NO_SUCH_PROCEDURE, "NO_SUCH_PROCEDURE"},
	{INVALID_ARGUMENTS, "INVALID_ARGUMENTS"},
});


//
// XNS::Courier::ProtocolRange
//
QString XNS::Courier::ProtocolRange::toString() const {
	return QString("%1-%2").arg(low.toString()).arg(high.toString());
}
void XNS::Courier::ProtocolRange::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, low);
	FROM_BYTE_BUFFER(bb, high);
}
void XNS::Courier::ProtocolRange::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, low);
	TO_BYTE_BUFFER(bb, high);
}


//
// XNS::Courier::VersionRange
//
QString XNS::Courier::VersionRange::toString() const {
	return QString("%1-%2").arg((quint16)low).arg((quint16)high);
}
void XNS::Courier::VersionRange::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, low);
	FROM_BYTE_BUFFER(bb, high);
}
void XNS::Courier::VersionRange::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, low);
	TO_BYTE_BUFFER(bb, high);
}


//
// XNS::Courier::Protocol2Body::CallBody
//
QString XNS::Courier::Protocol2Body::CallBody::toString() const {
	return QString("%1 %2-%3-%4").arg((quint16)transaction, 4, 16, QChar('0')).arg((quint16)program).arg((quint16)version).arg((quint16)procedure);
}
void XNS::Courier::Protocol2Body::CallBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, program);
	FROM_BYTE_BUFFER(bb, version);
	FROM_BYTE_BUFFER(bb, procedure);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Courier::Protocol2Body::CallBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, program);
	TO_BYTE_BUFFER(bb, version);
	TO_BYTE_BUFFER(bb, procedure);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Courier::Protocol2Body::RejectBody
//
QString XNS::Courier::Protocol2Body::RejectBody::toString() const {
	return QString("%1-%2").arg((quint16)transaction).arg(reject.toString());
}
void XNS::Courier::Protocol2Body::RejectBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, reject);
}
void XNS::Courier::Protocol2Body::RejectBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, reject);
}


//
// XNS::Courier::Protocol2Body::ReturnBody
//
QString XNS::Courier::Protocol2Body::ReturnBody::toString() const {
	return QString("%1").arg((quint16)transaction);
}
void XNS::Courier::Protocol2Body::ReturnBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Courier::Protocol2Body::ReturnBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Courier::Protocol2Body::AbortBody
//
QString XNS::Courier::Protocol2Body::AbortBody::toString() const {
	return QString("%1-%2").arg((quint16)transaction).arg((quint16)abort);
}
void XNS::Courier::Protocol2Body::AbortBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, abort);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Courier::Protocol2Body::AbortBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, abort);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Courier::Protocol2Body
//
void XNS::Courier::Protocol2Body::get(CallBody&   newValue) const {
	if (type == MessageType::CALL) {
		newValue = std::get<CallBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::get(RejectBody& newValue) const {
	if (type == MessageType::REJECT) {
		newValue = std::get<RejectBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::get(ReturnBody& newValue) const {
	if (type == MessageType::RETURN) {
		newValue = std::get<ReturnBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::get(AbortBody&  newValue) const {
	if (type == MessageType::ABORT) {
		newValue = std::get<AbortBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}

void XNS::Courier::Protocol2Body::set(const CallBody&   newValue) {
	if (type == MessageType::CALL) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::set(const RejectBody& newValue) {
	if (type == MessageType::REJECT) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::set(const ReturnBody& newValue) {
	if (type == MessageType::RETURN) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::set(const AbortBody&  newValue) {
	if (type == MessageType::ABORT) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}

QString XNS::Courier::Protocol2Body::toString() const {
	if (type == MessageType::CALL) {
		CallBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == MessageType::REJECT) {
		RejectBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == MessageType::RETURN) {
		ReturnBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == MessageType::ABORT) {
		AbortBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, type);
	if (type == MessageType::CALL) {
		CallBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == MessageType::REJECT) {
		RejectBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == MessageType::RETURN) {
		ReturnBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == MessageType::ABORT) {
		AbortBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else {
		logger.error("Unexpected");
		logger.error("  type %d", (quint16)type);
		ERROR();
	}
}
void XNS::Courier::Protocol2Body::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	if (type == MessageType::CALL) {
		CallBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == MessageType::REJECT) {
		RejectBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == MessageType::RETURN) {
		ReturnBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == MessageType::ABORT) {
		AbortBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else {
		logger.error("Unexpected");
		logger.error("  type %d", (quint16)type);
		ERROR();
	}
}


//
// XNS::Courier::Protocol3Body::CallBody
//
QString XNS::Courier::Protocol3Body::CallBody::toString() const {
	return QString("%1 %2-%3-%4").arg((quint16)transaction, 4, 16, QChar('0')).arg((quint16)program).arg((quint16)version).arg((quint16)procedure);
}
void XNS::Courier::Protocol3Body::CallBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, program);
	FROM_BYTE_BUFFER(bb, version);
	FROM_BYTE_BUFFER(bb, procedure);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Courier::Protocol3Body::CallBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, program);
	TO_BYTE_BUFFER(bb, version);
	TO_BYTE_BUFFER(bb, procedure);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Courier::Protocol3Body::RejectBody
//
void XNS::Courier::Protocol3Body::RejectBody::get(VersionRange&  newValue)  const {
	if (code == RejectCode::NO_SUCH_VERSION) {
		newValue = std::get<VersionRange>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  code %s", code.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::RejectBody::set(const VersionRange&   newValue) {
	if (code == RejectCode::NO_SUCH_VERSION) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  code %s", code.toString());
		ERROR();
	}
}
QString XNS::Courier::Protocol3Body::RejectBody::toString() const {
	if (code == RejectCode::NO_SUCH_PROGRAM) {
		return QString("%1").arg(code.toString());
	} else if (code == RejectCode::NO_SUCH_VERSION) {
		VersionRange newValue;
		get(newValue);
		return QString("%1-%2").arg(code.toString()).arg(newValue.toString());
	} else if (code == RejectCode::NO_SUCH_PROCEDURE) {
		return QString("%1").arg(code.toString());
	} else if (code == RejectCode::INVALID_ARGUMENTS) {
		return QString("%1").arg(code.toString());
	} else {
		logger.error("Unexpected");
		logger.error("  code %s", code.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::RejectBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, code);

	if (code == RejectCode::NO_SUCH_PROGRAM) {
		//
	} else if (code == RejectCode::NO_SUCH_VERSION) {
		VersionRange versionRange;
		FROM_BYTE_BUFFER(bb, versionRange);
		set(versionRange);
	} else if (code == RejectCode::NO_SUCH_PROCEDURE) {
		//
	} else if (code == RejectCode::INVALID_ARGUMENTS) {
		//
	} else {
		logger.error("Unexpected");
		logger.error("  code %s", code.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::RejectBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, code);

	if (code == RejectCode::NO_SUCH_PROGRAM) {
		//
	} else if (code == RejectCode::NO_SUCH_VERSION) {
		VersionRange versionRange;
		get(versionRange);
		TO_BYTE_BUFFER(bb, versionRange);
	} else if (code == RejectCode::NO_SUCH_PROCEDURE) {
		//
	} else if (code == RejectCode::INVALID_ARGUMENTS) {
		//
	} else {
		logger.error("Unexpected");
		logger.error("  code %s", code.toString());
		ERROR();
	}
}


//
// XNS::Courier::Protocol3Body::ReturnBody
//
QString XNS::Courier::Protocol3Body::ReturnBody::toString() const {
	return QString("%1").arg((quint16)transaction);
}
void XNS::Courier::Protocol3Body::ReturnBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Courier::Protocol3Body::ReturnBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Courier::Protocol3Body::AbortBody
//
QString XNS::Courier::Protocol3Body::AbortBody::toString() const {
	return QString("%1-%2").arg((quint16)transaction).arg((quint16)abort);
}
void XNS::Courier::Protocol3Body::AbortBody::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, transaction);
	FROM_BYTE_BUFFER(bb, abort);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Courier::Protocol3Body::AbortBody::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, transaction);
	TO_BYTE_BUFFER(bb, abort);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Courier::Protocol3Body
//
void XNS::Courier::Protocol3Body::get(CallBody&   newValue) const {
	if (type == MessageType::CALL) {
		newValue = std::get<CallBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::get(RejectBody& newValue) const {
	if (type == MessageType::REJECT) {
		newValue = std::get<RejectBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::get(ReturnBody& newValue) const {
	if (type == MessageType::RETURN) {
		newValue = std::get<ReturnBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::get(AbortBody&  newValue) const {
	if (type == MessageType::ABORT) {
		newValue = std::get<AbortBody>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}

void XNS::Courier::Protocol3Body::set(const CallBody&   newValue) {
	if (type == MessageType::CALL) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::set(const RejectBody& newValue) {
	if (type == MessageType::REJECT) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::set(const ReturnBody& newValue) {
	if (type == MessageType::RETURN) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::set(const AbortBody&  newValue) {
	if (type == MessageType::ABORT) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}

QString XNS::Courier::Protocol3Body::toString() const {
	if (type == MessageType::CALL) {
		CallBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == MessageType::REJECT) {
		RejectBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == MessageType::RETURN) {
		ReturnBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == MessageType::ABORT) {
		AbortBody newValue;
		get(newValue);
		return QString("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, type);
	if (type == MessageType::CALL) {
		CallBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == MessageType::REJECT) {
		RejectBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == MessageType::RETURN) {
		ReturnBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == MessageType::ABORT) {
		AbortBody newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else {
		logger.error("Unexpected");
		logger.error("  type %d", (quint16)type);
		ERROR();
	}
}
void XNS::Courier::Protocol3Body::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	if (type == MessageType::CALL) {
		CallBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == MessageType::REJECT) {
		RejectBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == MessageType::RETURN) {
		ReturnBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == MessageType::ABORT) {
		AbortBody newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else {
		logger.error("Unexpected");
		logger.error("  type %d", (quint16)type);
		ERROR();
	}
}


//
// XNS::Courier::ExpeditedCourier
//
QString XNS::Courier::ExpeditedCourier::toString() const {
//	return QString("%1 %2").arg(range.toString()).arg(body.toString());
	return QString("%1").arg(body.toString());
}
void XNS::Courier::ExpeditedCourier::fromByteBuffer(Buffer& bb) {
	FROM_BYTE_BUFFER(bb, range);
	if (range.low == Courier::ProtocolType::PROTOCOL3 && range.high == Courier::ProtocolType::PROTOCOL3) {
		FROM_BYTE_BUFFER(bb, body);
	} else {
		logger.error("Unexpected");
		logger.error("  range %s", range.toString());
		ERROR();
	}
}
void XNS::Courier::ExpeditedCourier::toByteBuffer  (Buffer& bb) const {
	TO_BYTE_BUFFER(bb, range);
	TO_BYTE_BUFFER(bb, body);
}


