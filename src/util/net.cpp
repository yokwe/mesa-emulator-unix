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


 #include <vector>
 #include <regex>

#include <net/bpf.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <unistd.h>

#include "Util.h"
static const Logger logger(__FILE__);

#include "ByteBuffer.h"
#include "BPF.h"

#include "net.h"

namespace net {

std::vector<Device> getDeviceList() {
    std::vector<Device> list;

	struct ifaddrs *ifap;
	int ret;

	CHECK_SYSCALL(ret, ::getifaddrs(&ifap))

	for(; ifap; ifap = ifap->ifa_next) {
		auto *p = ifap->ifa_addr;
		if (p->sa_family == AF_LINK) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifap->ifa_addr;
			if (sdl->sdl_alen != 0) {
				uint8_t* data = (uint8_t*)sdl->sdl_data;

				// build device
				std::string name;
				// copy name
				for(int i = 0; i < sdl->sdl_nlen; i++) {
					name += (char)data[i];
				}
				// copy address
                uint64_t address = 0;
                for(int i = 0; i < sdl->sdl_alen; i++) {
                    address <<= 8;
                    address |= data[sdl->sdl_nlen + i];
                }
                Device device(name, address);

				list.push_back(device);
//				logger.info("device %s", (std::string)device);
			}
		}
	}

    return list;
}
Device  getDevice(const std::string& name) {
    auto list = getDeviceList();
    for(auto e: list) {
        if (e.name == name) return e;
    }
    logger.error("Unexpected name %s!", name);
    ERROR();
}


class Driver_BPF : public Driver {
public:
    Driver_BPF(Device device) : Driver(device) {}

    void open() {
        bpf.open();
		bpfOpen = true;
        logger.info("bpf.fd         = %d", bpf.fd);
        logger.info("bpf.path       = %s", bpf.path);
        logger.info("bpf.bufferSize = %d", bpf.bufferSize);

        bpf.setInterface(device.name);
        bpf.setPromiscuous();                         // need to promiscuos mode to see all packet
        bpf.setImmediate(1);                   // don't wait unitl read buffer full
        bpf.setHeaderComplete(1);              // need to set 1 for custom source address
        bpf.setReadTimeout(1);               // read time out in unit of secod
//        bpf.setReadFilter(::BPF::PROGRAM_IP);  // receive only IPv4 packet
        bpf.setReadFilter(::BPF::PROGRAM_XNS); // receive only XNS packet

        logger.info("bufferSize     = %d", bpf.getBufferSize());
        logger.info("seeSent        = %d", bpf.getSeeSent());
        logger.info("headerComplete = %d", bpf.getHeaderComplete());
        logger.info("interface      = %s", bpf.getInterface());
        logger.info("nbrb           = %d", bpf.getNonBlockingReadBytes());
        logger.info("timeout        = %d", bpf.getReadTimeout());
        logger.info("buffer         = %p", bpf.buffer);
    }

    void close() {
		if (bpfOpen) {
			bpf.close();
			bpfOpen = false;
		}
    }
    
    // no error checking
    int  select  (std::chrono::microseconds timeout, int& opErrno) {
        return bpf.select(timeout, opErrno);
    }
    int  transmit(uint8_t* data, uint32_t dataLen, int& opErrno) {
        return bpf.transmit(data, dataLen, opErrno);
    }
    int  receive (uint8_t* data, uint32_t dataLen, int& opErrno, uint64_t* milliSecondsSinceEpoch = nullptr) {
        return bpf.receive(data, dataLen, opErrno, milliSecondsSinceEpoch);;
    }
    void discard() {
        bpf.discard();
    }

	const std::vector<ByteBuffer>& read() {
		return bpf.read();
	}
	void write(const ByteBuffer& value) {
		bpf.write(value);
	}

    BPF bpf;
	bool bpfOpen = false;
};

Driver* getDriver(const Device& device) {
    return new Driver_BPF(device);
}


std::string toOctalString(uint64_t address) {
    return std_sprintf("%lob", address);
}
std::string toDecimalString(uint64_t address) {
    auto n = address;
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
std::string toHexaDecimalString(uint64_t address, const std::string& sep) {
    std::string string;
    string = std_sprintf("%02X", ((int)(address >> 40)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(address >> 32)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(address >> 24)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(address >> 16)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(address >>  8)) & 0xFF);
    string += std_sprintf("%s%02X", sep, ((int)(address >>  0)) & 0xFF);
    return string;
}

#define HEXSEP "[-:]?"
uint64_t fromString(const std::string& string) {
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

}