/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
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
static const Logger logger(__FILE__);

#include "ByteBuffer.h"

void ByteBuffer::copyFrom(const ByteBuffer& that) {
	this->myBase     = that.myBase;
	this->myPosition = that.myPosition;
	this->myLimit    = that.myLimit;
	this->myCapacity = that.myCapacity;
	this->myData     = that.myData;
	this->myMarkPos  = that.myMarkPos;
}

ByteBuffer::ByteBuffer() {
	this->myBase     = 0;
	this->myPosition = 0;
	this->myLimit    = 0;
	this->myCapacity = 0;
	this->myData     = nullptr;
	this->myMarkPos  = INVALID_POS;
}
ByteBuffer::~ByteBuffer() {
	//
}

ByteBuffer::ByteBuffer(const ByteBuffer& that) {
	copyFrom(that);
}

ByteBuffer& ByteBuffer::operator =(const ByteBuffer& that) {
	copyFrom(that);
	return *this;
}

ByteBuffer::ByteBuffer(int capacity, uint8_t* data) {
	this->myBase     = 0;
	this->myPosition = 0;
	this->myLimit    = capacity;
	this->myCapacity = capacity;
	this->myData     = data;
	this->myMarkPos  = INVALID_POS;
}


void ByteBuffer::copyFrom(int len, const uint8_t* data) {
	if (len < 0) {
		logger.error("Too large len");
		logger.error("  len      = %5d", len);
		ERROR();
	}
	if (myCapacity < (myBase + len)) {
		logger.error("Exceed capacity");
		logger.error("  capacity = %5d", myCapacity);
		logger.error("  base     = %5d", myBase);
		logger.error("  len      = %5d", len);
		ERROR();
	}
	// clear myData + myBase with zero
	memset(myData + myBase, 0, (size_t)(myCapacity - myBase));
	// copy from data to myData + myBase
	memcpy(myData + myBase, data, (size_t)len);
	// reset position and limit
	myPosition = myBase;
	myLimit    = myBase + len;
}

std::string ByteBuffer::toString(int offset) const {
	return toHexString(limit() - offset, data() + offset);
}

void ByteBuffer::limit(int newValue) {
	if (myBase <= newValue && newValue <= myCapacity) {
		myLimit = newValue;
	} else {
		logger.error("Exceed limit");
		logger.error("  newValue = %5d", newValue);
		logger.error("  base     = %5d", myBase);
		logger.error("  capacity = %5d", myCapacity);
		logBackTrace();
		ERROR();
	}
}

void ByteBuffer::position(int newValue) {
	if (myBase <= newValue && newValue <= myLimit) {
		myPosition = newValue;
	} else {
		logger.error("Exceed limit");
		logger.error("  newValue = %5d", newValue);
		logger.error("  base     = %5d", myBase);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}

void ByteBuffer::mark() {
	if (myMarkPos == INVALID_POS) {
		myMarkPos = myPosition;
	} else {
		logger.error("Unexpected");
		logger.error("  myMarkPos = %d", myMarkPos);
		ERROR();
	}
}
void ByteBuffer::reset() {
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
void ByteBuffer::read8(const int index, uint8_t& value) const {
	const int readSize = 1;
	if ((index + readSize) <= myLimit) {
		const uint8_t* data = myData + index;
		value = data[0];
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::read16(const int index, uint16_t& value) const {
	const int readSize = 2;
	if ((index + readSize) <= myLimit) {
		const uint8_t* data = myData + index;
		value = (data[0] << 8) | (data[1] << 0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::read32(const int index, uint32_t& value) const {
	const int readSize = 4;
	if ((index + readSize) <= myLimit) {
		const uint8_t* data = myData + index;
		value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::read48(const int index, uint64_t& value) const {
	const int readSize = 6;
	if ((index + readSize) <= myLimit) {
		const uint8_t* data = myData + index;
		value = ((uint64_t)data[0] << 40) | ((uint64_t)data[1] << 32) |
				((uint64_t)data[2] << 24) | ((uint64_t)data[3] << 16) |
				((uint64_t)data[4] <<  8) | ((uint64_t)data[5] <<  0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void ByteBuffer::read(const int index, const int readSize, uint8_t* value) const {
	if ((index + readSize) <= myLimit) {
		const uint8_t* data = myData + index;
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
void ByteBuffer::write8(const int index, uint8_t value) {
	const int writeSize = 1;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	uint8_t* data = myData + index;
	data[0] = value;
}
void ByteBuffer::write16(const int index, uint16_t value) {
	const int writeSize = 2;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	uint8_t* data = myData + index;
	data[0] = (uint8_t)(value >> 8);
	data[1] = (uint8_t)(value);
}
void ByteBuffer::write32(const int index, uint32_t value) {
	const int writeSize = 4;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  position = %5d", myPosition);
		logger.error("  limit    = %5d", myLimit);
		logger.error("  capacity = %5d", myCapacity);
		ERROR();
	}
	uint8_t* data = myData + index;
	data[0] = (uint8_t)(value >> 24);
	data[1] = (uint8_t)(value >> 16);
	data[2] = (uint8_t)(value >>  8);
	data[3] = (uint8_t)(value >>  0);
}
void ByteBuffer::write48(const int index, uint64_t value) {
	const int writeSize = 6;
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	uint8_t* data = myData + index;
	data[0] = (uint8_t)(value >> 40);
	data[1] = (uint8_t)(value >> 32);
	data[2] = (uint8_t)(value >> 24);
	data[3] = (uint8_t)(value >> 16);
	data[4] = (uint8_t)(value >>  8);
	data[5] = (uint8_t)(value >>  0);
}
void ByteBuffer::write(const int index, const int writeSize, const uint8_t* value) {
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		ERROR();
	}
	uint8_t* data = myData + index;
	memcpy(data, value, writeSize);
}

void ByteBuffer::writeZero(int n) {
	if (n < 0) ERROR()
	for(int i = 0; i < n; i++) write8(0);
}
