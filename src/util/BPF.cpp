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
// BPF.cpp
//

#include <chrono>
#include <cstring>
#include <deque>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <net/if.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "Util.h"
static const Logger logger(__FILE__);

#include "ByteBuffer.h"

#include "BPF.h"

// copy output from "tcpdump -dd ip"
static struct bpf_insn ip_insn[] = {
	{ 0x28, 0, 0, 0x0000000c },
	{ 0x15, 0, 1, 0x00000800 },
	{ 0x06, 0, 0, 0x00040000 },
	{ 0x06, 0, 0, 0x00000000 },
};
static struct bpf_program program_ip = {
	(u_int)COUNT_ELEMENT(ip_insn),
	ip_insn,
};
const struct bpf_program* BPF::PROGRAM_IP = &program_ip;

// copy output from "tcpcump -dd ether proto xnd"
static struct bpf_insn xns_insn[] = {
	{ 0x28, 0, 0, 0x0000000c },
	{ 0x15, 0, 1, 0x00000600 },
	{ 0x06, 0, 0, 0x00040000 },
	{ 0x06, 0, 0, 0x00000000 },
};
static struct bpf_program program_xns = {
	(u_int)COUNT_ELEMENT(xns_insn),
	xns_insn,
};
const struct bpf_program* BPF::PROGRAM_XNS = &program_xns;


void BPF::open() {
	char tempPath[sizeof("/dev/bpf00") + 1];
	int  tempFD;

	for(int i = 0; i < 256; i++) {
		snprintf(tempPath, sizeof(tempPath), "/dev/bpf%d", i);
//		LOG_SYSCALL(tempFD, ::open(tempPath, O_RDWR));
		tempFD = ::open(tempPath, O_RDWR);
		if (tempFD < 0) continue;
		break;
	}
	if (tempFD <0) ERROR()
	path       = tempPath;
	fd         = tempFD;
	bufferSize = getBufferSize();
	buffer     = new uint8_t[bufferSize];
}
void BPF::close() {
	if (0 <= fd) {
		int ret;
		LOG_SYSCALL(ret, ::close(fd))
		fd = -1;
	}
	delete buffer;
	path.clear();
	fd         = -1;
	bufferSize = -1;
	buffer     = 0;
	readData.clear();
}

const std::deque<ByteBuffer>& BPF::read() {
	int validBufferLen;
	CHECK_SYSCALL(validBufferLen, ::read(fd, buffer, bufferSize))

//	logger.debug("validBufferLen = %d", validBufferLen);
	readData.clear();

	for(int i = 0; i < validBufferLen; ) {
		struct bpf_hdr* p = (struct bpf_hdr*)(buffer + i);
		int     caplen = (int)(p->bh_caplen);
		int     hdrlen = (int)(p->bh_hdrlen);
		uint8_t* data   = buffer + i;

		ByteBuffer element(hdrlen + caplen, data);
		element.setBase(hdrlen);
		readData.push_front(element);

		i += BPF_WORDALIGN(caplen + hdrlen);
	}

	return readData;
}
int BPF::read(ByteBuffer& bb, std::chrono::microseconds timeout, std::chrono::microseconds* timestamp) {
	bb.clear();
	if (readData.empty()) {
		if (getNonBlockingReadBytes() == 0) {
			select(timeout);
		}
		if (getNonBlockingReadBytes() == 0) return 0;
		read();
	}
	const ByteBuffer& bpfData = readData.back();
	// bpfData hase base value
	// copy bpfData[base .. limit) to bb
	bb.write(bpfData.limit() - bpfData.base(), bpfData.data() + bpfData.base());
	bb.flip();
	if (timestamp != nullptr) {
		struct bpf_hdr* p = (struct bpf_hdr*)bb.data();
		struct BPF_TIMEVAL tstamp = p->bh_tstamp;
		int64_t time = (int64_t)(tstamp.tv_sec) * 1000'000 + tstamp.tv_usec; // conver to microsecond
		*timestamp = std::chrono::microseconds(time);
	}
	readData.pop_back();
	return bb.length();
}
int BPF::write(const ByteBuffer& value) {
	int ret;
	LOG_SYSCALL(ret, ::write(fd, value.data(), value.limit()))
	return ret;
}

// for net::Driver
// no error check
int  BPF::select  (std::chrono::microseconds timeout_) {
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	// set tmeval
	auto count = timeout_.count();
	auto factor = 1000 * 1000;   // microseconds per second
	struct timeval timeout;
	timeout.tv_sec  = count / factor; // unit is second
	timeout.tv_usec = count % factor; // unit is microsecond
	int ret;
	CHECK_SYSCALL(ret, ::select(FD_SETSIZE, &fds, NULL, NULL, &timeout))
	return ret;
}
int  BPF::transmit(uint8_t* data, uint32_t dataLen) {
	ByteBuffer bb(dataLen, data);
	int ret = write(bb);
	return ret;
}
int  BPF::receive (uint8_t* data, uint32_t dataLen, std::chrono::microseconds timeout, std::chrono::microseconds* timestamp) {
	ByteBuffer bb(dataLen, data);
	int ret = read(bb, timeout, timestamp);
	return ret;
}
void BPF::clear() {
	// clear readData
	readData.clear();
	// clear buffer
	flush();
}

// BIOCGBLEN
//   Returns the required buffer length	for reads on bpf files
uint32_t BPF::getBufferSize() {
	int ret;
	uint32_t value;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGBLEN, &value))
	return value;
}

// BIOCPROMISC
//   Forces the interface into promiscuous mode
void BPF::setPromiscuous() {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCPROMISC, NULL))
}
// BIOCFLUSH
//   Flushes the buffer	of incoming packets, and resets	the statistics
void BPF::flush() {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCFLUSH, NULL))
}

// BIOCGETIF
//   Returns the name of the hardware interface that the file is listening
std::string BPF::getInterface() {
	int ret;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGETIF, &ifr))
	std::string value = ifr.ifr_name;
	return value;
}

// BIOCSETIF
//   Sets the hardware interface associate with the file.
void BPF::setInterface(const std::string& value) {
	int ret;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	::strncpy(ifr.ifr_name, value.c_str(), IFNAMSIZ - 1);
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSETIF, &ifr))
}

// BIOCSRTIMEOUT
//   Sets the read timeout parameter
//   Default value is 0. Which means no timeout.
void BPF::setReadTimeout(const struct timeval& value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSRTIMEOUT, &value))
}

// BIOCGRTIMEOUT
//   Gets the read timeout parameter
void BPF::getReadTimeout(struct timeval& value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGRTIMEOUT, &value))
}


// BIOCIMMEDIATE
//   Enables or	disables "immediate mode"
//   When immediate more is enabled, reads return immediately upon packet reception
//   When immediate mode is disabled, read will block until buffer become full or timeout.
void BPF::setImmediate(uint32_t value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCIMMEDIATE, &value))
}

// BIOCSETFNR
//   Sets the read filter program
void BPF::setReadFilter(const struct bpf_program* value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSETFNR, value))
}

// BIOCGHDRCMPLT
//   Sets the status of	the "header complete" flag.
uint32_t BPF::getHeaderComplete() {
	int ret;
	uint32_t value;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGHDRCMPLT, &value))
	return value;
}

// BIOCSHDRCMPLT
//   Gets the status of	the "header complete" flag.
//   When value is 0, source address is filled automatically
//   When value is 1, source address is not filled automatically
//   Default value is 0
void BPF::setHeaderComplete(uint32_t value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSHDRCMPLT, &value))
}

//// BIOCGDIRECTION
////   Gets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
//uint32_t BPF::getDirection() {
//	int ret;
//	uint32_t value;
//	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGDIRECTION, &value))
//	return value;
//}
//
//// BIOCSDIRECTION
////   Sets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
////   Vfalue must be BPF_D_IN, BPF_D_OUT or BPF_D_INOUT
////   Default is BPF_D_INOUT
//void BPF::setDirection(uint32_t value) {
//	int ret;
//	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSDIRECTION, &value))
//}

// BIOCGSEESENT
//   These commands are obsolete but left for compatibility.
//   Use BIOCSDIRECTION and BIOCGDIRECTION instead.
//   Sets or gets the flag determining whether locally generated packets on the interface should be returned by BPF.
//   Set to zero to see only incoming packets on the interface.  Set to one to see packets
uint32_t BPF::getSeeSent() {
	int ret;
	uint32_t value;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGSEESENT, &value))
	return value;
}

// BIOCSSEESENT
// These commands are obsolete but left for compatibility.
//   Use BIOCSDIRECTION and BIOCGDIRECTION instead.
//   Sets or gets the flag determining whether locally generated packets on the interface should be returned by BPF.
//   Set to zero to see only incoming packets on the interface.
//   Set to one to see packets originating locally and remotely on the interface.
//   This flag is initialized to one by default.
void BPF::setSeeSent(uint32_t value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSSEESENT, &value))
}



// FIONREAD
//   Returns the number of bytes that are immediately available for	reading
int BPF::getNonBlockingReadBytes() {
	int ret;
	int value;
	CHECK_SYSCALL(ret, ::ioctl(fd, FIONREAD, &value))
	return value;
}

// FIONBIO
//   Sets or clears non-blocking I/O
//   If arg is non-zero, then doing a read(2) when no data is available will return -1 and errno will be set to EAGAIN
void BPF::setNonBlockingIO(int value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, FIONBIO, &value))
}

