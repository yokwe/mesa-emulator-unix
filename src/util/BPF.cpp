/*******************************************************************************
 * Copyright (c) 2021, Yasuhiro Hasegawa
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

#include "Util.h"
static const Logger logger = Logger::getLogger("bpf");

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <net/if.h>

#include <string.h>
#include <unistd.h>

#include "ByteBuffer.h"
#include "Network.h"


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

	for(int i = 0; i < 99; i++) {
		snprintf(tempPath, sizeof(tempPath), "/dev/bpf%d", i);
		LOG_SYSCALL(tempFD, ::open(tempPath, O_RDWR));
		if (tempFD < 0) {
			int opErrno = errno;
			LOG_ERRNO(opErrno);
			continue;
		}
		break;
	}
	path       = tempPath;
	fd         = tempFD;
	bufferSize = getBufferSize();
	buffer     = new quint8[bufferSize];
}
void BPF::close() {
	if (0 <= fd) {
		int ret;
		LOG_SYSCALL(ret, ::close(fd))
		fd = -1;
	}
	delete buffer;
	buffer = 0;
}

const QList<ByteBuffer>& BPF::read() {
	int validBufferLen;
	CHECK_SYSCALL(validBufferLen, ::read(fd, buffer, bufferSize))

//	logger.debug("validBufferLen = %d", validBufferLen);
	readData.clear();

	for(int i = 0; i < validBufferLen; ) {
		struct bpf_hdr* p = (struct bpf_hdr*)(buffer + i);
		int     caplen = (int)(p->bh_caplen);
		int     hdrlen = (int)(p->bh_hdrlen);
		quint8* data   = buffer + i;

		ByteBuffer element(hdrlen + caplen, data);
		element.setBase(hdrlen);
		readData.append(element);

		i += BPF_WORDALIGN(caplen + hdrlen);
	}

	return readData;
}
void BPF::write(const Network::Packet& value) {
	int ret;
	LOG_SYSCALL(ret, ::write(fd, value.data(), value.limit()))
}

// for Network::Driver
// no error check
int  BPF::select  (quint32 timeout, int& opErrno) {
	(void)timeout;
	opErrno = 0;
	if (readData.isEmpty()) {
		int ret = getNonBlockingReadBytes();

		if (ret == 0) {
			// do actual select call
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			// 1 second
			struct timeval t;
			t.tv_sec  = timeout;
			t.tv_usec = 0;

			ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
			opErrno = errno;
		}

		return ret;
	} else {
		return readData.first().limit();
	}
}
int  BPF::transmit(quint8* data, quint32 dataLen, int& opErrno) {
	int ret;
	LOG_SYSCALL2(ret, opErrno, ::write(fd, data, dataLen));
	return ret;
}
int  BPF::receive (quint8* data, quint32 dataLen, int& opErrno, qint64* msecSinceEpoch) {
	opErrno = 0;
	// if readData is empty, fill readData
	if (readData.isEmpty()) read();

	// Take first entry
	ByteBuffer bb = readData.first();
	int len = bb.limit() - bb.base();
	if (dataLen < (quint32)len) {
		logger.error("Unexpected");
		logger.error("  dataLen %u", dataLen);
		logger.error("  len     %d", len);
		ERROR();
	}
	// copy bb to data
	bb.read(bb.base(), len, data);

	// set dateTime
	if (msecSinceEpoch != nullptr) {
		struct timeval* p = (struct timeval*)bb.data();
		*msecSinceEpoch = (p->tv_sec * 1000) + (p->tv_usec / 1000);
	}
	// remove first entry
	readData.pop_back();
	return len;
}
void BPF::discard() {
	// clear readData
	readData.clear();
	// clear buffer
	flush();
}

// BIOCGBLEN
//   Returns the required buffer length	for reads on bpf files
quint32 BPF::getBufferSize() {
	int ret;
	quint32 value;
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
QString BPF::getInterface() {
	int ret;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGETIF, &ifr))
	QString value = ifr.ifr_name;
	return value;
}

// BIOCSETIF
//   Sets the hardware interface associate with the file.
void BPF::setInterface(const QString& value) {
	int ret;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	::strncpy(ifr.ifr_name, TO_CSTRING(value), IFNAMSIZ - 1);
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
void BPF::setImmediate(quint32 value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCIMMEDIATE, &value))
}

// BIOCSETFNR
//   Sets the read filter program
void BPF::setReadFilter(const struct bpf_program* value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSETFNR, value))
}

// BIOCGRSIG
//   Sets the status of	the "header complete" flag.
quint32 BPF::getHeaderComplete() {
	int ret;
	quint32 value;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGRSIG, &value))
	return value;
}

// BIOCSRSIG
//   Gets the status of	the "header complete" flag.
//   When value is 0, source address is filled automatically
//   When value is 1, source address is not filled automatically
//   Default value is 0
void BPF::setHeaderComplete(quint32 value) {
	int ret;
	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSRSIG, &value))
}

//// BIOCGDIRECTION
////   Gets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
//quint32 BPF::getDirection() {
//	int ret;
//	quint32 value;
//	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGDIRECTION, &value))
//	return value;
//}
//
//// BIOCSDIRECTION
////   Sets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
////   Vfalue must be BPF_D_IN, BPF_D_OUT or BPF_D_INOUT
////   Default is BPF_D_INOUT
//void BPF::setDirection(quint32 value) {
//	int ret;
//	CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSDIRECTION, &value))
//}

// BIOCGSEESENT
//   These commands are obsolete but left for compatibility.
//   Use BIOCSDIRECTION and BIOCGDIRECTION instead.
//   Sets or gets the flag determining whether locally generated packets on the interface should be returned by BPF.
//   Set to zero to see only incoming packets on the interface.  Set to one to see packets
quint32 BPF::getSeeSent() {
	int ret;
	quint32 value;
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
void BPF::setSeeSent(quint32 value) {
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

