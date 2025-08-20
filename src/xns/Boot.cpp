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
// Boot.cpp
//

#include "../util/Util.h"
static const util::Logger logger(__FILE__);

#include "Boot.h"


//
// XNS::Boot::Type
//
NameMap::Map<uint16_t> XNS::Boot::Type::nameMap(NameMap::toString16u, {
	{SIMPLE_REQUEST, "SIMPLE_REQUEST"},
	{SIMPLE_DATA,    "SIMPLE_DATA"},
	{SPP_REQUEST,    "SPP_REQUEST"},
});


//
// XNS::Boot::SimpleRequest
//
std::string XNS::Boot::SimpleRequest::toString() const {
	return bootFileNumber.toHexaDecimalString();
}
void XNS::Boot::SimpleRequest::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, bootFileNumber);
}
void XNS::Boot::SimpleRequest::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, bootFileNumber);
}


//
// XNS::Boot::SimpleData
//
std::string XNS::Boot::SimpleData::toString() const {
	return std::string("%1 %2 %3").arg(bootFileNumber.toHexaDecimalString()).arg(packetNumber).arg(block.toString());
}
void XNS::Boot::SimpleData::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, bootFileNumber);
	FROM_BYTE_BUFFER(bb, packetNumber);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Boot::SimpleData::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, bootFileNumber);
	TO_BYTE_BUFFER(bb, packetNumber);
	TO_BYTE_BUFFER(bb, block);
}


//
// XNS::Boot::SPPRequest
//
std::string XNS::Boot::SPPRequest::toString() const {
	return std::string("%1 %2").arg(bootFileNumber.toHexaDecimalString()).arg(connectionID, 4, 16, QChar('0'));
}
void XNS::Boot::SPPRequest::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, bootFileNumber);
	FROM_BYTE_BUFFER(bb, connectionID);
}
void XNS::Boot::SPPRequest::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, bootFileNumber);
	TO_BYTE_BUFFER(bb, connectionID);
}

void XNS::Boot::get(SimpleRequest& newValue) const {
	if (type == Type::SIMPLE_REQUEST) {
		newValue = std::get<SimpleRequest>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Boot::get(SimpleData&    newValue) const {
	if (type == Type::SIMPLE_DATA) {
		newValue = std::get<SimpleData>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Boot::get(SPPRequest&    newValue) const {
	if (type == Type::SPP_REQUEST) {
		newValue = std::get<SPPRequest>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}

void XNS::Boot::set(const SimpleRequest& newValue) {
	if (type == Type::SIMPLE_REQUEST) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Boot::set(const SimpleData&    newValue) {
	if (type == Type::SIMPLE_DATA) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Boot::set(const SPPRequest&    newValue) {
	if (type == Type::SPP_REQUEST) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
std::string XNS::Boot::toString() const {
	if (type == Type::SIMPLE_REQUEST) {
		SimpleRequest newValue;
		get(newValue);
		return std::string("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == Type::SIMPLE_DATA) {
		SimpleData newValue;
		get(newValue);
		return std::string("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else if (type == Type::SPP_REQUEST) {
		SPPRequest newValue;
		get(newValue);
		return std::string("%1 %2").arg(type.toString()).arg(newValue.toString());
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Boot::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, type);

	if (type == Type::SIMPLE_REQUEST) {
		SimpleRequest newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == Type::SIMPLE_DATA) {
		SimpleData newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else if (type == Type::SPP_REQUEST) {
		SPPRequest newValue;
		FROM_BYTE_BUFFER(bb, newValue);
		set(newValue);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}
void XNS::Boot::toByteBuffer(ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, type);

	if (type == Type::SIMPLE_REQUEST) {
		SimpleRequest newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == Type::SIMPLE_DATA) {
		SimpleData newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else if (type == Type::SPP_REQUEST) {
		SPPRequest newValue;
		get(newValue);
		TO_BYTE_BUFFER(bb, newValue);
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", type.toString());
		ERROR();
	}
}



