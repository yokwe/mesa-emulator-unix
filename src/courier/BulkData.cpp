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
// BulkData.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("cr-bulk");

#include "BulkData.h"


//
// Courier::BulkData::Identifier
//
QString Courier::BulkData::Identifier::toString() const {
	return QString("%1-%2").arg(host.toString()).arg(QString("%1").arg(hostRelativeIdentifier, 8, 16, QChar('0').toUpper()));
}

// Courier::Base
void Courier::BulkData::Identifier::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, host);
	FROM_BYTE_BUFFER(bb, hostRelativeIdentifier);
}
void Courier::BulkData::Identifier::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, host);
	TO_BYTE_BUFFER(bb, hostRelativeIdentifier);
}


//
// Courier::BulkData::Descriptor::Choice
//
NameMap::Map<quint16> Courier::BulkData::Descriptor::Choice::nameMap(NameMap::toString16X04, {
	{NULL_,     "NULL"},
	{IMMEDIATE, "IMMEDIATE"},
	{PASSIVE,   "PASSIVE"},
	{ACTIVE,    "ACTIVE"}
});


//
//
//
void Courier::BulkData::Descriptor::get(      NetworkIdentifier&  newValue)  const {
	if (choice == Choice::PASSIVE || choice == Choice::ACTIVE) {
		newValue = std::get<NetworkIdentifier>(body);
	} else {
		logger.error("Unexpected");
		logger.error("  choice %s", choice.toString());
		ERROR();
	}
}
void Courier::BulkData::Descriptor::set(const NetworkIdentifier&  newValue) {
	if (choice == Choice::PASSIVE || choice == Choice::ACTIVE) {
		body = newValue;
	} else {
		logger.error("Unexpected");
		logger.error("  type %s", choice.toString());
		ERROR();
	}

}

