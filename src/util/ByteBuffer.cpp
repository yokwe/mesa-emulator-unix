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

#include "Util.h"
static const Logger logger(__FILE__);

#include "ByteBuffer.h"


// valid range of myBytePos   is [0..myByteCapacity)
// valid range of myByteLimit is [myBytePos..myByteCapacity)

void ByteBuffer::checkBeforeRead(uint32_t byteSize) {
    auto newBytePos = myBytePos + byteSize;
    if (newBytePos <= myByteLimit) return;

    logger.error("Unexpected value  %s", __FUNCTION__);
    logger.error("  byteSize  %u", byteSize);
    logger.error("  name      %s", myImpl->myName);
    logger.error("  data      %p", myData);
    logger.error("  capacity  %u", myByteCapacity);
    logger.error("  pos       %u", myBytePos);
    logger.error("  limit     %u", myByteLimit);
    ERROR()
}
void ByteBuffer::checkBeforeWrite(uint32_t byteSize) {
    auto newBytePos = myBytePos + byteSize;
    if (newBytePos <= myByteCapacity) return;

    logger.error("Unexpected value  %s", __FUNCTION__);
    logger.error("  byteSize  %u", byteSize);
    logger.error("  name      %s", myImpl->myName);
    logger.error("  data      %p", myData);
    logger.error("  capacity  %u", myByteCapacity);
    logger.error("  pos       %u", myBytePos);
    logger.error("  limit     %u", myByteLimit);
    ERROR()
}

ByteBuffer ByteBuffer::range(uint32_t wordOffset, uint32_t wordSize) const {
    auto bytePos  = wordValueToByteValue(wordOffset);
    auto readSize = wordValueToByteValue(wordSize);
    if (myByteCapacity < (bytePos + readSize)) {
        // fix readSize
        auto newReadSize = myByteCapacity - bytePos;
//        logger.warn("%s  Adjust readSize from %d to %d", __FUNCTION__, readSize, newReadSize);
        readSize = newReadSize;
    }    
    return ByteBuffer(myImpl, myData + bytePos, readSize, readSize);
}

void ByteBuffer::mark() {
    if (myByteMark != BAD_MARK) ERROR()
    myByteMark = myBytePos;
}
void ByteBuffer::reset() {
    if (myByteMark == BAD_MARK) ERROR()
    myBytePos = myByteMark;
    myByteMark = BAD_MARK;
}
