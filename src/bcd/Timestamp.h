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
// Timestamp.h
//

#pragma once

#include <cstdint>
#include <string>

#include "../util/ByteBuffer.h"
#include "../util/Util.h"


// TimeStamp: TYPE = RECORD [net, host: [0..377B], time: LONG CARDINAL];
// Null: TimeStamp = TimeStamp[net: 0, host: 0, time: 0];
class Timestamp : public ByteBuffer::Readable, public HasToString {
    uint8_t  net;
    uint8_t  host;
    uint32_t time;
public:
    static Timestamp getNull();

    Timestamp(uint8_t net_, uint8_t host_, uint32_t time_) :  net(net_), host(host_), time(time_) {}
    Timestamp() : net(0), host(0), time(0) {}

    ByteBuffer& read(ByteBuffer& bb) override {
        bb.read(net, host, time);
        return bb;
    }
    std::string toString() const override;
    
    bool isNull() const {
        return net == 0 && host == 0 && time == 0;
    }

    inline auto operator<=>(const Timestamp& that) const {
        auto ret = this->time <=> that.time;
        if (ret == std::strong_ordering::equal) ret = this->net <=> that.net;
        if (ret == std::strong_ordering::equal) ret = this->host <=> that.host;
        return ret;
    }
    inline auto operator==(const Timestamp& that) const {
        return this->time == that.time && this->net == that.net && this->host == that.host;
    }
};
