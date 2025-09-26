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
// Type.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/ByteBuffer.h"

#include "Type.h"

namespace xns {

void STRING::fromByteBuffer(ByteBuffer& bb) {
	uint16_t length;
	bb.read16(length);

	string.clear();
	for(uint16_t i = 0; i < length; i++) {
		uint8_t newValue;
		bb.read8(newValue);
        string += newValue;
	}
	// read padding
	if (length % 2) {
		uint8_t newValue;
		bb.read8(newValue);
		(void)newValue;
	}
}
void STRING::toByteBuffer  (ByteBuffer& bb) const {
	int length = string.length();
	if (MAX_LENGTH < length) {
		logger.error("Unexpected");
		logger.error("  MAX_LENGTH = %d", MAX_LENGTH);
		logger.error("  length     = %d", length);
		ERROR();
	}
	bb.write16((uint16_t)length);
	for(int i = 0; i < length; i++) {
        auto newValue = string.at(i);
		bb.write8((uint8_t)newValue);
	}
	// write padding
	if (length % 2) {
		bb.write8(0);
	}
}




}