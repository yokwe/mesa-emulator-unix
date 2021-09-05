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
// DataStream.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("cr-data");

#include "DataStream.h"


//
// myBase <= myPosition <= myLimit
//

void Courier::DataStream::position(int newValue) {
	if (myBase <= newValue && newValue <= myLimit) {
		myPosition = newValue;
	} {
		logger.error("Exceed range");
		logger.error("  newValue = %5d", newValue);
		logger.error("  base     = %5d", myBase);
		logger.error("  position = %5d", myPosition);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void Courier::DataStream::limit(int newValue) {
	if (myBase <= newValue && myPosition <= newValue) {
		myLimit = newValue;
		myData->resize(myLimit);
	} else {
		logger.error("Exceed range");
		logger.error("  newValue = %5d", newValue);
		logger.error("  base     = %5d", myBase);
		logger.error("  position = %5d", myPosition);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void Courier::DataStream::setBase(int newValue) {
	if (0 <= newValue && newValue <= myLimit) {
		myBase     = newValue;
		myPosition = newValue;
	} else {
		logger.error("Exceed range");
		logger.error("  newValue = %5d", newValue);
		logger.error("  base     = %5d", myBase);
		logger.error("  position = %5d", myPosition);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}


void Courier::DataStream::copyFrom(int len, const quint8* value) {
	QByteArray& data(*myData);

	// copy from data to myData + myBase
	myData->reserve(myBase + len);
	for(int i = 0; i < len; i++) {
		data[myBase + i] = value[i];
	}
	// reset position and limit
	myPosition = myBase;
	limit(myBase + len);
}


//
// read
//
void Courier::DataStream::read8(const int index, quint8& value) const {
	const int readSize = 1;
	if ((index + readSize) <= myLimit) {
		QByteArray& data(*myData);

		value = data[index];
	} else {
		logger.error("Exceed range");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void Courier::DataStream::read16(const int index, quint16& value) const {
	const int readSize = 2;
	if ((index + readSize) <= myLimit) {
		QByteArray& data(*myData);

		value = (data[index + 0] << 8) | (data[index + 1] << 0);
	} else {
		logger.error("Exceed range");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void Courier::DataStream::read32(const int index, quint32& value) const {
	const int readSize = 4;
	if ((index + readSize) <= myLimit) {
		QByteArray& data(*myData);

		value = (data[index + 0] << 24) | (data[index + 1] << 16) |
				(data[index + 2] <<  8) | (data[index + 3] <<  0);
	} else {
		logger.error("Exceed range");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void Courier::DataStream::read48(const int index, quint64& value) const {
	const int readSize = 6;
	if ((index + readSize) <= myLimit) {
		QByteArray& data(*myData);

		value = ((quint64)data[index + 0] << 40) | ((quint64)data[index + 1] << 32) |
				((quint64)data[index + 2] << 24) | ((quint64)data[index + 3] << 16) |
				((quint64)data[index + 4] <<  8) | ((quint64)data[index + 5] <<  0);
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}
void Courier::DataStream::read(const int index, const int readSize, quint8* value) const {
	if ((index + readSize) <= myLimit) {
		QByteArray& data(*myData);

		for(int i = 0; i < readSize; i++) {
			value[i] = data[index + i];
		}
	} else {
		logger.error("Exceed limit");
		logger.error("  index    = %5d", index);
		logger.error("  readSize = %5d", readSize);
		logger.error("  limit    = %5d", myLimit);
		ERROR();
	}
}


//
// write
//
void Courier::DataStream::write8(const int index, quint8 value) {
	QByteArray& data(*myData);

	data[index + 0] = value;
}
void Courier::DataStream::write16(const int index, quint16 value) {
	QByteArray& data(*myData);

	data[index + 0] = (quint8)(value >> 8);
	data[index + 1] = (quint8)(value >> 0);
}
void Courier::DataStream::write32(const int index, quint32 value) {
	QByteArray& data(*myData);

	data[index + 0] = (quint8)(value >> 24);
	data[index + 1] = (quint8)(value >> 16);
	data[index + 2] = (quint8)(value >>  8);
	data[index + 3] = (quint8)(value >>  0);
}
void Courier::DataStream::write48(const int index, quint64 value) {
	QByteArray& data(*myData);

	data[index + 0] = (quint8)(value >> 40);
	data[index + 1] = (quint8)(value >> 32);
	data[index + 2] = (quint8)(value >> 24);
	data[index + 3] = (quint8)(value >> 16);
	data[index + 4] = (quint8)(value >>  8);
	data[index + 5] = (quint8)(value >>  0);
}
void Courier::DataStream::write(const int index, const int writeSize, const quint8* value) {
	QByteArray& data(*myData);

	for(int i = 0; i < writeSize; i++) {
		data[index + i] = value[i];
	}
}


//
// operator << for primitive type
//
Courier::DataStream& Courier::DataStream::operator << (const DataStream& value) {
	QByteArray& data(*myData);

	int writeSize = value.limit() - value.base();
	write(writeSize, data.constData() + value.base());
	return *this;
}


//
// operator >> for primitive type
//
Courier::DataStream& Courier::DataStream::operator >> (DataStream& value) {
	QByteArray& data(*myData);

	int readSize = value.limit() - value.base();
	read(readSize, data.data() + value.base());
	return *this;
}

