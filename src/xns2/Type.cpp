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

#include <string>
#include <regex>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/ByteBuffer.h"

#include "Type.h"

#include "Echo.h"
#include "Error.h"
#include "Ethernet.h"
#include "IDP.h"
#include "PEX.h"
#include "RIP.h"
#include "SPP.h"
#include "Time.h"

namespace xns {

void initialize() {
    xns::echo::initialize();
    xns::error::initialize();
    xns::ethernet::initialize();
    xns::idp::initialize();
    xns::pex::initialize();
    xns::rip::initialize();
    xns::spp::initialize();
    xns::time::initialize();
}

void dumpFormatList() {
    {
        auto list = xns::BaseNumber<uint8_t>::getFormatList();
        for(const auto& e: list) {
            logger.info("%8s %-20s  %14d  %s", "uint8_t", e.group, e.value, e.format);
        }
    }
    {
        auto list = xns::BaseNumber<uint16_t>::getFormatList();
        for(const auto& e: list) {
            logger.info("%8s %-20s  %14d  %s", "uint16_t", e.group, e.value, e.format);
        }
    }
    {
        auto list = xns::BaseNumber<uint32_t>::getFormatList();
        for(const auto& e: list) {
            logger.info("%8s %-20s  %14d  %s", "uint32_t", e.group, e.value, e.format);
        }
    }
    {
        auto list = xns::BaseNumber<uint64_t>::getFormatList();
        for(const auto& e: list) {
            logger.info("%8s %-20s  0x%012lX  %s", "uint64_t", e.group, e.value, e.format);
        }
    }

}

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


std::string host::toOctalString(uint64_t value) {
    return std_sprintf("%lob", value);
}
std::string host::toDecimalString(uint64_t value) {
    auto n = value;
    std::string string;
    for(;;) {
        if (n == 0) break;
        auto quotient  = n / 1000;
        auto remainder = (int)(n % 1000);
//        string += std_sprintf("-%03d", remainder);
        string.insert(0, std_sprintf("-%03d", remainder));
        n = quotient;
    }
    return string.substr(1);
}
std::string host::toHexaDecimalString(uint64_t value, const std::string& sep) {
    std::string string;
    string = std_sprintf("%02X", ((int)(value >> 40)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(value >> 32)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(value >> 24)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(value >> 16)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(value >>  8)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(value >>  0)) & 0xFF);
    return string;
}

#define HEXSEP "[-:]?"
uint64_t host::fromString(const std::string& string) {
    static std::regex dec4("([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3})");
    static std::regex dec5("([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3})");
    static std::regex hex6("([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])" HEXSEP "([0-9A-Fa-f][0-9A-Fa-f])");
    static std::regex oct("([0-7]+)b");

    {
        std::smatch m;
        if (std::regex_match(string, m, dec4)) {
            uint64_t ret = 0;
            ret += std::stoi(m[1]);
            ret *= 1000;
            ret += std::stoi(m[2]);
            ret *= 1000;
            ret += std::stoi(m[3]);
            ret *= 1000;
            ret += std::stoi(m[4]);
            return ret;
        }
    }
    {
        std::smatch m;
        if (std::regex_match(string, m, dec5)) {
            uint64_t ret = 0;
            ret += std::stoi(m[1]);
            ret *= 1000;
            ret += std::stoi(m[2]);
            ret *= 1000;
            ret += std::stoi(m[3]);
            ret *= 1000;
            ret += std::stoi(m[4]);
            ret *= 1000;
            ret += std::stoi(m[5]);
            return ret;
        }
    }
    {
        std::smatch m;
        if (std::regex_match(string, m, hex6)) {
            uint64_t ret = 0;
            ret += std::stoi(m[1], nullptr, 16);
            ret <<= 8;
            ret += std::stoi(m[2], nullptr, 16);
            ret <<= 8;
            ret += std::stoi(m[3], nullptr, 16);
            ret<<= 8;
            ret += std::stoi(m[4], nullptr, 16);
            ret <<= 8;
            ret += std::stoi(m[5], nullptr, 16);
            ret <<= 8;
            ret += std::stoi(m[6], nullptr, 16);
            return ret;
        }
    }
    {
        std::smatch m;
        if (std::regex_match(string, m, oct)) {
            return std::stol(m[1], nullptr, 8);
        }
    }

    logger.error("Unexpected");
    logger.error("  string = %s!", string);
    ERROR();
}


UINT48 Host::BROADCAST = Host{0xFFFF'FFFF'FFFFL, "BROADCAST"};
UINT48 Host::UNKNOWN   = Host{0x0000'0000'0000L, "UNKNOWN"};
UINT48 Host::BFN_GVWIN = Host{0x0000'aa00'0e60L, "BFN_GVWIN"};

UINT32 Net::ALL     = Net(0xFFFF'FFFF, "ALL");
UINT32 Net::UNKNOWN = Net(0x0000'0000, "UNKNOWN");

}