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

#include <net/bpf.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <unistd.h>

#include <QtCore>

#include "Network.h"
#include "Network_freebsd.h"


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


QList<Network::Device> Network::getDeviceList() {
	static QList<Network::Device> list = Network::FreeBSD::getDeviceList();
	return list;
}

Network::Driver*       Network::getDriver(const Network::Device& device) {
	Network::FreeBSD::Driver* driver = new Network::FreeBSD::Driver(device);
	return driver;
}


//
// Network::FreeBSD::getDeviceList
//
QList<Network::Device> Network::FreeBSD::getDeviceList() {
	QList<Network::Device> list;

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
				Network::Device device;

				// copy name
				for(int i = 0; i < sdl->sdl_nlen; i++) {
					device.name.append(data[i]);
				}
				// copy address
				ByteBuffer bb(sdl->sdl_alen, data + sdl->sdl_nlen);
				bb.read48(device.address);

				list += device;
//				logger.info("device %s", device.toString());
			}
		}
	}

	::freeifaddrs(ifap);

	return list;
}

//
// Network::FreeBSD::Driver
//
Network::FreeBSD::Driver::Driver(const Network::Device& device_) : Network::Driver(device_) {
	bpf.open();
	logger.info("bpf.fd         = %d", bpf.fd);
	logger.info("bpf.path       = %s", bpf.path);
	logger.info("bpf.bufferSize = %d", bpf.bufferSize);

	bpf.setInterface(device.name);
	bpf.setPromiscuous();
	bpf.setImmediate(1);
	bpf.setHeaderComplete(0);
	bpf.setReadTimeout(1);
	bpf.setReadFilter(BPF::PROGRAM_XNS);

	logger.info("bufferSize     = %d", bpf.getBufferSize());
	logger.info("direction      = %d", bpf.getDirection());
	logger.info("headerComplete = %d", bpf.getHeaderComplete());
	logger.info("interface      = %s", bpf.getInterface());
	logger.info("nbrb           = %d", bpf.getNonBlockingReadBytes());
	logger.info("timeout        = %d", bpf.getReadTimeout());
	logger.info("buffer         = %p", bpf.buffer);
}

Network::FreeBSD::Driver::~Driver() {
	bpf.close();
}

int Network::FreeBSD::Driver::select  (quint32 timeout, int& opErrno) {
	return bpf.select(timeout, opErrno);
}
int Network::FreeBSD::Driver::transmit(quint8* data, quint32 dataLen, int& opErrno) {
	return bpf.transmit(data, dataLen, opErrno);
}
int Network::FreeBSD::Driver::receive(quint8* data, quint32 dataLen, int& opErrno, quint64* msecSinceEpoch) {
	return bpf.receive(data, dataLen, opErrno, msecSinceEpoch);
}
void Network::FreeBSD::Driver::discard() {
	bpf.discard();
}

