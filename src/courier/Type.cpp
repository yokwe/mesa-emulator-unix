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
static const Logger logger = Logger::getLogger("courier");

#include "Type.h"


//
// Courier::BOOLEAN
//
void Courier::BOOLEAN::fromByteBuffer(Buffer& bb) {
	quint16 newValue;
	bb.read16(newValue);
	value(newValue);
}
void Courier::BOOLEAN::toByteBuffer  (Buffer& bb) const {
	bb.write16(value());
}


//
// Courier::STRING
//
void Courier::STRING::fromByteBuffer(Buffer& bb) {
	quint16 length;
	bb.read16(length);
	byteArray.clear();

	for(quint16 i = 0; i < length; i++) {
		quint8 newValue;
		bb.read8(newValue);
		byteArray.append((char)newValue);
	}
	// read padding
	if (length % 2) {
		quint8 newValue;
		bb.read8(newValue);
		(void)newValue;
	}
}
void Courier::STRING::toByteBuffer  (Buffer& bb) const {
	int length = byteArray.length();
	if (MAX_LENGTH < length) {
		logger.error("Unexpected");
		logger.error("  MAX_LENGTH = %d", MAX_LENGTH);
		logger.error("  length     = %d", length);
		ERROR();
	}
	bb.write16((quint16)length);
	for(int i = 0; i < length; i++) {
		bb.write8((quint8)byteArray[i]);
	}
	// write padding
	if (length % 2) {
		bb.write8(0);
	}
}

