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
// MesaBuffer.cpp
//

#include "Util.h"
static const Logger logger(__FILE__);

#include "MesaBuffer.h"

void MesaBuffer::pos(uint32_t newValue) {
    if (newValue <= span.size()) {
        myPos = newValue;
    } else {
        logger.error("Unexpected newValue");
        logger.error("  newValue  %u", newValue);
        logger.error("  size      %u", span.size());
        ERROR();
    }
}
uint32_t MesaBuffer::pos() {
    return myPos;
}

uint16_t MesaBuffer::get16() {
    const int readSize = 1;

    if ((myPos + readSize) <= span.size()) {
        uint8_t* data = (uint8_t*)span.data() + (myPos * 2);
        myPos += readSize;
        uint16_t ret = (data[0] << 8) | (data[1] << 0);  // mesa data is big endian
        return ret;
    } else {
        logger.error("Unexpected position  %s", __FUNCTION__);
        logger.error("  pos       %u", myPos);
        logger.error("  readSize  %u", readSize);
        logger.error("  size      %u", span.size());
        ERROR(); 
    }
}
uint32_t MesaBuffer::get32() {
    const int readSize = 2;

    if ((myPos + readSize) <= span.size()) {
        uint8_t* data = (uint8_t*)span.data() + (myPos * 2);
        myPos += readSize;
//      Network order
//		uint32_t ret = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);
//      Mesa Long order  low half, high half
		uint32_t ret = (data[0] << 8) | (data[1] << 0) | (data[2] << 24) | (data[3] << 16);
        return ret;
    } else {
        logger.error("Unexpected position  %s", __FUNCTION__);
        logger.error("  pos       %u", myPos);
        logger.error("  readSize  %u", readSize);
        logger.error("  size      %u", span.size());
        ERROR();
    }
}

