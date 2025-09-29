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
 // Error.h
 //

#pragma once

#include "Type.h"

namespace xns::error {

void initialize();

class ErrorNumber : public UINT16 {
    static inline const char* group = "xns::error::ErrorNumber";
    ErrorNumber(uint16_t value_, const char* name_) : UINT16(group, value_, name_) {}
public:
    ErrorNumber() : UINT16() {}

    void fromByteBuffer(ByteBuffer& bb) {
        fromByteBufferGroup(group, bb);
    }
    
	static UINT16 UNSPEC;               // An unspecified error is detected at destination
	static UINT16 BAD_CHECKSUM;         // The checksum is incorrect or the packet has some other serious inconsistency detected at destination
	static UINT16 NO_SOCKET;            // The specified socket does not exist at the specified destination host
	static UINT16 RESOURCE_LIMIT;       // The destination cannot accept the packet due to resource limitations
	static UINT16 LISTEN_REJECT;        //
	static UINT16 INVALID_PACKET_TYPE;  //
	static UINT16 PROTOCOL_VIOLATION;   //

	static UINT16 UNSPECIFIED_IN_ROUTE; // An unspecified error occurred before reaching destination
	static UINT16 INCONSISTENT;         // The checksum is incorrect, or the packet has some other serious inconsistency before reaching destination
	static UINT16 CANT_GET_THERE;       // The destination host cannot be reached from here.
	static UINT16 EXCESS_HOPS;          // The packet has passed through 15 internet routes without reaching its destination.
	static UINT16 TOO_BIG;              // The packet is too large to be forwarded through some intermediate network.
			                            // The Error Parameter field contains the length of the largest packet that can be accommodated.
	static UINT16 CONGESTION_WARNING;   //
	static UINT16 CONGESTION_DISCARD;   //
};

struct Error {
    ErrorNumber errorNumber;
    UINT16      errorParameter;
    BLOCK       block;

    // this <= ByteBuffer
    void fromByteBuffer(ByteBuffer& bb) {
        errorNumber.fromByteBuffer(bb);
        errorParameter.fromByteBuffer(bb);
        block.fromByteBuffer(bb);
    }

    // ByteBuffer <= this
    void toByteBuffer(ByteBuffer& bb) const {
        errorNumber.toByteBuffer(bb);
        errorParameter.toByteBuffer(bb);
        block.toByteBuffer(bb);
    }
};

}
