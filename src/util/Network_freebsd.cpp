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


// add user to group wheel to get access /dev/bpf
// OSX
//   dseditgroup -o edit -a hasegawa -t use wheel
// FreeBSD
//   pw group mod wheel -m hasegawa



static QList<XNS::Device> getDeviceList_() {
	QList<XNS::Device> list;

	struct ifaddrs *ifap;
	int ret = ::getifaddrs(&ifap);
	if (ret != 0) {
		int myErrno = errno;
		logger.error("Unexpected getifaddrs");
		logger.error("  error = %d  %s", myErrno, strerror(myErrno));
		ERROR();
	}

	for(; ifap; ifap = ifap->ifa_next) {
		struct sockaddr *p = ifap->ifa_addr;
		if (p->sa_family == AF_LINK) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifap->ifa_addr;
			if (sdl->sdl_alen != 0) {
				XNS::Device device;

				// copy name
				{
					char* q = sdl->sdl_data;
					for(int i = 0; i < sdl->sdl_nlen; i++) {
						device.name.append(q[i]);
					}
				}
				// copy address
				{
					quint8* q = (quint8*)sdl->sdl_data + sdl->sdl_nlen;
					XNS::Address address(q);
					device.address = address;
				}

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


//Network::Interface::Interface(const QString& name) {
//	logger.info("name %s", name);
//	bool foundEthernet = false;
//	for(auto e: Network::getEthernetList()) {
//		if (e.name == name) {
//			this->ethernet = e;
//			foundEthernet = true;
//			break;
//		}
//	}
//	if (!foundEthernet) {
//		logger.error("Unexpected name");
//		logger.error("  name %s!", name);
//		ERROR();
//	}
//
//	{
//		const char deviceNamePattern[] = "/dev/bpf000";
//
//		char deviceName[sizeof(deviceNamePattern)];
//
//		for(int i = 0; i < 999; i++) {
//			sprintf(deviceName, "/dev/bpf%d", i);
//			logger.info("open %s!", deviceName);
//			fd = open( deviceName, O_RDONLY );
//			if (fd == -1) {
//				if (errno == EACCES) {
//					logger.error("need to read and write permission to /dev/bpf");
//					ERROR();
//				}
//				if (errno == ENOENT) {
//					logger.error("device /dev/bpf does not exist");
//					ERROR();
//				}
//				// continue to next device
//				logger.warn("open error");
//				logger.error("  name  = %s", deviceName);
//				logger.error("  error = %d  %s", errno, strerror(errno));
//				continue;
//			}
//			break;
//		}
//		logger.info("fd = %d", fd);
//	}
//
//	// BIOCSETIF
//	{
//		struct ifreq ifr;
//		memset(&ifr, 0, sizeof(ifr));
//		::strncpy(ifr.ifr_name, TO_CSTRING(ethernet.name), IFNAMSIZ - 1);
//	    int ret = ioctl(fd, BIOCSETIF, &ifr);
//		logger.info("BIOCSETIF %d", ret);
//		if (ret == -1) {
//			logger.error("  error = %d  %s", errno, strerror(errno));
//		}
//	}
//	// BIOCPROMISC
//	{
//		int ret = ioctl(fd, BIOCPROMISC, NULL);
//		logger.info("BIOCPROMISC %d", ret);
//		if (ret == -1) {
//			logger.error("  error = %d  %s", errno, strerror(errno));
//		}
//	}
//	// BIOCIMMEDIATE
//	{
//		int value = 1; // 1 for immediate mode
//		int ret = ioctl(fd, BIOCIMMEDIATE, &value);
//		logger.info("BIOCIMMEDIATE %d", ret);
//		if (ret == -1) {
//			logger.error("  error = %d  %s", errno, strerror(errno));
//		}
//	}
//	// BIOCGHDRCMPLT
//	{
//		int value = 1; // 1 for no automatic setting of source address
//		int ret = ioctl(fd, BIOCGHDRCMPLT, &value);
//		logger.info("BIOCGHDRCMPLT %d", ret);
//		if (ret == -1) {
//			logger.error("  error = %d  %s", errno, strerror(errno));
//		}
//	}
//}


