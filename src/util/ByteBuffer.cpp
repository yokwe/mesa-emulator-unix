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
// ByteBuffer.cpp
//

#include "Util.h"
static const Logger logger = Logger::getLogger("bytebuffer");

#include "ByteBuffer.h"


void ByteBuffer::Buffer::copyFrom(int len, const quint8* data) {
	if (len < 0) {
		logger.error("Too large len");
		logger.error("  len      = %5d", len);
		ERROR();
	}
	if (myCapacity < len) {
		logger.error("Exceed capacity");
		logger.error("  capacity = %5d", myCapacity);
		logger.error("  len      = %5d", len);
		ERROR();
	}
	// clear myData with zero
	memset(myData, 0, (size_t)myCapacity);
	// copy from data to myData
	memcpy(myData, data, (size_t)len);
	// reset position and limit
	myPosition = 0;
	myLimit    = len;
}

QString ByteBuffer::Buffer::toString(int limit) {
	QString ret;
	for(int i = myBase; i < myLimit; i++) {
		ret += QString::asprintf("%02X", myData[i]);
	}
	return ret.left(limit);
}

void ByteBuffer::Buffer::position(int newValue	) {
	if (0 <= newValue && newValue <= myLimit) {
		myPosition = newValue;
	} else {
		logger.error("Exceed limit");
		logger.error("  newValue = %5d", newValue);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}

void ByteBuffer::Buffer::mark() {
	if (myMarkPos == INVALID_POS) {
		myMarkPos = myPosition;
	} else {
		logger.error("Unexpected");
		logger.error("  myMarkPos = %d", myMarkPos);
		ERROR();
	}
}
void ByteBuffer::Buffer::reset() {
	if (myMarkPos == INVALID_POS) {
		logger.error("Unexpected");
		ERROR();
	} else {
		myPosition = myMarkPos;
		myMarkPos = INVALID_POS;
	}
}


//
// ByteBuffer::read
//
void ByteBuffer::Buffer::read8(const int index, quint8& value) const {
	const int readSize = 1;
	if ((index + readSize) <= myLimit) {
		const quint8* data = myData + index;
		value = data[0];
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::Buffer::read16(const int index, quint16& value) const {
	const int readSize = 2;
	if ((index + readSize) <= myLimit) {
		const quint8* data = myData + index;
		value = (data[0] << 8) | (data[1] << 0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::Buffer::read32(const int index, quint32& value) const {
	const int readSize = 4;
	if ((index + readSize) <= myLimit) {
		const quint8* data = myData + index;
		value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::Buffer::read48(const int index, quint64& value) const {
	const int readSize = 6;
	if ((index + readSize) <= myLimit) {
		const quint8* data = myData + index;
		value = ((quint64)data[0] << 40) | ((quint64)data[1] << 32) |
				((quint64)data[2] << 24) | ((quint64)data[3] << 16) |
				((quint64)data[4] <<  8) | ((quint64)data[5] <<  0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::Buffer::read(const int index, const int readSize, quint8* value) const {
	if ((index + readSize) <= myLimit) {
		const quint8* data = myData + index;
		memcpy(value, data, readSize);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}


//
// ByteBuffer::write
//
void ByteBuffer::Buffer::write8(const int index, quint8 value) {
	const int writeSize = 1;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	quint8* data = myData + index;
	data[0] = value;
}
void ByteBuffer::Buffer::write16(const int index, quint16 value) {
	const int writeSize = 2;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	quint8* data = myData + index;
	data[0] = (quint8)(value >> 8);
	data[1] = (quint8)(value);
}
void ByteBuffer::Buffer::write32(const int index, quint32 value) {
	const int writeSize = 4;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  position = %5d", myPosition);
		logger.error("  limit    = %5d", myLimit);
		logger.error("  capacity = %5d", myCapacity);
		ERROR();
	}
	quint8* data = myData + index;
	data[0] = (quint8)(value >> 24);
	data[1] = (quint8)(value >> 16);
	data[2] = (quint8)(value >>  8);
	data[3] = (quint8)(value >>  0);
}
void ByteBuffer::Buffer::write48(const int index, quint64 value) {
	const int writeSize = 6;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	quint8* data = myData + index;
	data[0] = (quint8)(value >> 40);
	data[1] = (quint8)(value >> 32);
	data[2] = (quint8)(value >> 24);
	data[3] = (quint8)(value >> 16);
	data[4] = (quint8)(value >>  8);
	data[5] = (quint8)(value >>  0);
}
void ByteBuffer::Buffer::write(const int index, const int writeSize, const quint8* value) {
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	quint8* data = myData + index;
	memcpy(data, value, writeSize);
}


