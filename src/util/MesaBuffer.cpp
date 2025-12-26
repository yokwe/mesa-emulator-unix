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

void MesaBuffer::bytePos(uint32_t newValue) {
    if (newValue <= myData.size()) {
        myPos = newValue;
    } else {
        logger.error("Unexpected newValue");
        logger.error("  newValue  %u", newValue);
        logger.error("  size      %u", myData.size());
        ERROR();
    }
}

uint16_t MesaBuffer::get16() {
    const int readSize = 2;

    if ((myPos + readSize) <= myData.size()) {
        uint16_t ret = (myData[myPos + 0] << 8) | (myData[myPos + 1] << 0);  // mesa use big endian
        myPos += readSize;
        return ret;
    } else {
        logger.error("Unexpected position  %s", __FUNCTION__);
        logger.error("  pos       %u", myPos);
        logger.error("  readSize  %u", readSize);
        logger.error("  size      %u", myData.size());
        ERROR(); 
    }
}
uint32_t MesaBuffer::get32() {
    const int readSize = 4;

    if ((myPos + readSize) <= myData.size()) {
//      Network order
//		uint32_t ret = (span[myPos + 0] << 24) | (span[myPos + 1] << 16) | (span[myPos + 2] << 8) | (span[myPos + 3] << 0);
//      Mesa Long order  low half, high half
		uint32_t ret = (myData[myPos + 0] << 8) | (myData[myPos + 1] << 0) | (myData[myPos + 2] << 24) | (myData[myPos + 3] << 16);
        myPos += readSize;
        return ret;
    } else {
        logger.error("Unexpected position  %s", __FUNCTION__);
        logger.error("  pos       %u", myPos);
        logger.error("  readSize  %u", readSize);
        logger.error("  size      %u", myData.size());
        ERROR();
    }
}

