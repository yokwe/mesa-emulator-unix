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
// net.h
//

#pragma once

#include <string>
#include <cstdint>

#include "ByteBuffer.h"

#include "../util/Util.h"

namespace net {

class Device {
public:
    const std::string name;
    const uint64_t    address;

    Device(const std::string& name_, uint64_t address_) : name(name_), address(address_) {}
    operator std::string() const noexcept {
        uint16_t w1, w2, w3;
        getAddress(w1, w2, w3);
        std::string string = std_sprintf("{%04x-%04x-%04x  %s}", w1, w2, w3, name);
        return string;
    }
    void getAddress(uint16_t& word1, uint16_t& word2, uint16_t& word3) const {
        word1 = (uint16_t)(address >> 32);
        word2 = (uint16_t)(address >> 16);
        word3 = (uint16_t)(address >>  0);
    }
};

class Driver {
public:
    const Device device;

    virtual void open()  = 0;
    virtual void close() = 0;

    // no error checking
    virtual int  select  (uint32_t timeout, int& opErrno) = 0;
    virtual int  transmit(uint8_t* data, uint32_t dataLen, int& opErrno) = 0;
    virtual int  receive (uint8_t* data, uint32_t dataLen, int& opErrno, uint64_t* milliSecondsSinceEpoch = nullptr) = 0;
    virtual void discard() = 0;

    // packet base functions
    virtual const std::vector<ByteBuffer>& read() = 0;
    virtual void write(const ByteBuffer& value) = 0;

    Driver(const Device& device_) : device(device_.name, device_.address) {}
    virtual ~Driver() {}
};

std::vector<Device> getDeviceList();

Device  getDevice(const std::string& name);
Driver* getDriver(const Device& device);

}