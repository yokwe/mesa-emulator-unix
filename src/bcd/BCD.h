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
// BCD.h
//

#pragma once

#include <cstdint>

#include "../util/ByteBuffer.h"


// Stamp: TYPE = RECORD [net, host: [0..377B], time: LONG CARDINAL];
// Null: Stamp = Stamp[net: 0, host: 0, time: 0];
class Stamp : public ByteBuffer::Readable {
    std::string string;
public:
    uint8_t  net;
    uint8_t  host;
    uint32_t time;

    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(net, host, time);

        auto dateString = Util::toString(Util::toUnixTime(time));
        string = isNull() ? "#NULL#" : std_sprintf("%s#%03d#%03d", dateString, net, host);

        return bb;
    }

    bool isNull() {
        return net == 0 && host == 0 && time == 0;
    }

    std::string toString() {
        return string;
    }
};


struct BCD {
    static constexpr uint16_t VersionID = 6103;

	uint16_t versionIdent;
	Stamp    version;
	Stamp    creator;

    ByteBuffer& read(ByteBuffer& bb) {
        bb.read(versionIdent, version, creator);
        return bb;
    }
};