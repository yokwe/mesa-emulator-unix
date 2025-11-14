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

// void ByteBuffer::copyFrom(int len, const uint8_t* data) {
// 	if (len < 0) {
// 		logger.error("Too large len");
// 		logger.error("  len      = %5d", len);
// 		ERROR();
// 	}
// 	if (myCapacity < (myBase + len)) {
// 		logger.error("Exceed capacity");
// 		logger.error("  capacity = %5d", myCapacity);
// 		logger.error("  base     = %5d", myBase);
// 		logger.error("  len      = %5d", len);
// 		ERROR();
// 	}
// 	// clear myData + myBase with zero
// 	memset(myData + myBase, 0, (size_t)(myCapacity - myBase));
// 	// copy from data to myData + myBase
// 	memcpy(myData + myBase, data, (size_t)len);
// 	// reset position and limit
// 	myPosition = myBase;
// 	myLimit    = myBase + len;
// }

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

void ByteBuffer::mark() {
	if (myMarkPos == INVALID_POS) {
		myMarkPos = myPosition;
	} else {
		logger.error("Unexpected");
		logger.error("  myMarkPos = %d", myMarkPos);
		logBackTrace();
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
		logBackTrace();
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
		logBackTrace();
		ERROR();
	}
}
void ByteBuffer::read32(const int index, uint32_t& value) const {
	const int readSize = 4;
	if ((index + readSize) <= myLimit) {
		const uint8_t* data = myData + index;
//      Network order
//		value = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
//      Mesa Long order  low half, high half
		value = (data[0] << 8) | (data[1] << 0) | (data[2] << 24) | (data[3] << 16); // FIXME
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		logBackTrace();
		ERROR();
	}
}

uint8_t  ByteBuffer::get8() {
	const int readSize = 1;
	if ((myPosition + readSize) <= myLimit) {
		const uint8_t* data = myData + myPosition;
		uint8_t value = data[0];
		myPosition += readSize; // increment position
		return value;
	} else {
		logger.error("Exceed limit");
		logger.error("  position = %5d", myPosition);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		logBackTrace();
		ERROR();
	}
}
uint16_t ByteBuffer::get16() {
	const int readSize = 2;
	if ((myPosition + readSize) <= myLimit) {
		const uint8_t* data = myData + myPosition;
		uint16_t value = (data[0] << 8) | (data[1] << 0);
		myPosition += readSize; // increment position
		return value;
	} else {
		logger.error("Exceed limit");
		logger.error("  position = %5d", myPosition);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		logBackTrace();
		ERROR();
	}
}
uint32_t ByteBuffer::get32() {
	const int readSize = 4;
	if ((myPosition + readSize) <= myLimit) {
		const uint8_t* data = myData + myPosition;
//      Mesa Long order  low half, high half
		uint32_t value = (data[0] << 8) | (data[1] << 0) | (data[2] << 24) | (data[3] << 16);
		myPosition += readSize; // increment position
		return value;
	} else {
		logger.error("Exceed limit");
		logger.error("  position = %5d", myPosition);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		logBackTrace();
		ERROR();
	}
}



//
// ByteBuffer::write
//
void ByteBuffer::write(const int index, const int writeSize, const uint8_t* value) {
	if (myCapacity < (index + writeSize)) {
		logger.error("Exceed capacity");
		logger.error("  capacity  = %5d", myCapacity);
		logger.error("  index     = %5d", index);
		logger.error("  writeSize = %5d", writeSize);
		logBackTrace();
		ERROR();
	}
	uint8_t* data = myData + index;
	memcpy(data, value, writeSize);
}
