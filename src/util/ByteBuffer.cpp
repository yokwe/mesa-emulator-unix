/*******************************************************************************
 * Copyright (c) 2026, Yasuhiro Hasegawa
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

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "ByteBuffer.h"

void ByteBuffer::checkBytePos(uint32_t bytePos) {
    if (bytePos < myByteSize) return;

    logger.error("Unexpected values  %s", __FUNCTION__);
    logger.error("  bytePos  %u", bytePos);
    logger.error("  size     %u", myByteSize);
    ERROR();
}

ByteBuffer ByteBuffer::range(uint32_t wordOffset, uint32_t wordSize) {
    auto bytePos  = wordValueToByteValue(wordOffset);
    auto readSize = wordValueToByteValue(wordSize);
    if (myByteSize < (bytePos + readSize)) {
        // fix readSize
        logger.info("unexpected value  readSize   %d   myByteSize  %d", readSize, myByteSize);
        readSize = myByteSize - bytePos;
    }
    // sanity check
    checkByteRange(bytePos, readSize);
    
    return ByteBuffer(myImpl, myData + bytePos, readSize);
}
