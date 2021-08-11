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


#include "Util.h"
static const Logger logger = Logger::getLogger("net-freebsd");

#include "Network.h"

#include <net/bpf.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

#include <unistd.h>

#include <QtCore>

// add user to group wheel to get access /dev/bpf
// OSX
//   dseditgroup -o edit -a hasegawa -t use wheel
// FreeBSD
//   pw group mod wheel -m hasegawa
//   sudo chmod 660 /dev/bpf


// IMPORTANT
// To enable XNS packet sent to/from Jail, change following sysctl
// Default setting is 1. So cannot see XNS packet from other Jail
//   sysctl net.link.bridge.pfil_onlyip=0


static QList<XNS::Device> getDeviceList_() {
	QList<XNS::Device> list;

	struct ifaddrs *ifap;
	int ret;

	CHECK_SYSCALL(ret, ::getifaddrs(&ifap))

	for(; ifap; ifap = ifap->ifa_next) {
		auto *p = ifap->ifa_addr;
		if (p->sa_family == AF_LINK) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifap->ifa_addr;
			if (sdl->sdl_alen != 0) {
				quint8* data = (quint8*)sdl->sdl_data;

				// build device
				XNS::Device device;

				// copy name
				for(int i = 0; i < sdl->sdl_nlen; i++) {
					device.name.append(data[i]);
				}
				// copy address
				XNS::Address address(data + sdl->sdl_nlen);
				device.address = address;

				list += device;
//				logger.info("device %s", device.toString());
			}
		}
	}

	::freeifaddrs(ifap);

	return list;
}

QList<XNS::Device> Network::getDeviceList() {
	static QList<XNS::Device> list = getDeviceList_();
	return list;
}


class FreeBSDDriver : public Network::Driver {
public:
	int select  (int& opError, quint32 timeout = 1); // returns return value of of select().  default timeout is 1 second
	int transmit(int& opError, XNS::Packet& data);   // returns return value of send()
	int receive (int& opError, XNS::Packet& data);   // returns return value of of recv()

	// discard received packet
	void discard() = 0;

	FreeBSDDriver(const XNS::Device& device_);

	void openBPF();
	void prepareBPF();

	int     bufferSize;
	quint8* buffer;

	std::list<XNS::Packet> list;
	QString bpfPath;
};

int FreeBSDDriver::select  (int& opErrno, quint32 timeout) {
	if (!list.empty()) {
		return 1;
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	struct timeval t;
	t.tv_sec  = timeout;
	t.tv_usec = 0;

	int ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
	opErrno = errno;
	return ret;
}
int FreeBSDDriver::transmit(int& opErrno, XNS::Packet& packet) {
	int ret = ::send(fd, packet.data, packet.limit(), 0);
	opErrno = errno;
	return ret;
}
int FreeBSDDriver::receive(int& opErrno, XNS::Packet& packet) {
	// FIXME
	(void)opErrno;
	(void)packet;
	return 0;
}

#define LOG_RET_ERRNO(ret, errNo) logger.error(#ret " = %d  errno = %d  %s", ret, errNo, strerror(errNo))
#define CHECK_RET(ret) {if (ret < 0) { int opErrno = errno; LOG_RET_ERRNO(ret, opErrno); ERROR(); }}

void FreeBSDDriver::discard() {
	list.clear();

	int readCount = 0;
	int ret;
	for(;;) {
		{
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			struct timeval t;
			t.tv_sec  = 0;
			t.tv_usec = 0;

			ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
			CHECK_RET(ret);
		}
		if (ret == 0) break;
		if (0 < ret) {
			ret = ::read(fd, buffer, bufferSize);
			CHECK_RET(ret);
		}
	}
	logger.debug("discards %d", readCount);
}


void FreeBSDDriver::openBPF() {
	const char pathPattern[] = "/dev/bpf000";
	char path[sizeof(pathPattern)];

	for(int i = 0; i < 99; i++) {
		sprintf(path, "/dev/bpf%d", i);
		logger.info("open %s!", path);

		fd = ::open(path, O_RDONLY);
		if (fd < 0) {
			int opErrno = errno;
			LOG_ERRNO(opErrno);
			continue;
		}
		break;
	}
	bpfPath = path;
	logger.info("bpfPath  %s", bpfPath);
	logger.info("fd       %d", fd);
}

void FreeBSDDriver::prepareBPF() {
	// BIOCSETIF
	{
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		::strncpy(ifr.ifr_name, TO_CSTRING(device.name), IFNAMSIZ - 1);
		int ret = ::ioctl(fd, BIOCSETIF, &ifr);
		int opErrno = errno;
		if (ret == -1) {
			logger.error("BIOCSETIF %d", ret);
			logger.error("  error = %d  %s", opErrno, strerror(opErrno));
			ERROR();
		}
	}
	// BIOCPROMISC
	{
		int ret = ::ioctl(fd, BIOCPROMISC, NULL);
		int opErrno = errno;
		if (ret == -1) {
			logger.error("BIOCPROMISC %d", ret);
			logger.error("  error = %d  %s", opErrno, strerror(opErrno));
			ERROR();
		}
	}
	// BIOCIMMEDIATE
	{
		int value = 1; // 1 for immediate mode
		int ret = ioctl(fd, BIOCIMMEDIATE, &value);
		int opErrno = errno;
		if (ret == -1) {
			logger.error("BIOCIMMEDIATE %d", ret);
			logger.error("  error = %d  %s", opErrno, strerror(opErrno));
		}
	}
	// BIOCGHDRCMPLT
	{
		int value = 1; // 1 for no automatic setting of source address
		int ret = ioctl(fd, BIOCGHDRCMPLT, &value);
		int opErrno = errno;
		if (ret == -1) {
			logger.error("BIOCGHDRCMPLT %d", ret);
			logger.error("  error = %d  %s", opErrno, strerror(opErrno));
			ERROR();
		}
	}
	// BIOCGBLEN
	{
		int value;
		int ret = ioctl(fd, BIOCGBLEN, &value);
		int opErrno = errno;
		if (ret == -1) {
			logger.error("BIOCGBLEN %d", ret);
			logger.error("  error = %d  %s", opErrno, strerror(opErrno));
			ERROR();
		}
		bufferSize = value;
		buffer = (quint8*)::malloc(bufferSize);
		logger.info("buffer   %d  %p", bufferSize, buffer);
	}
}
FreeBSDDriver::FreeBSDDriver(const XNS::Device& device_) : Driver(device_) {
	openBPF();
	prepareBPF();
}
