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

#include <net/if.h>
#include <net/if_dl.h>

#include <ifaddrs.h>


// add user to group wheel to get access /dev/bpf
// OSX
//   dseditgroup -o edit -a hasegawa -t use wheel
// FreeBSD
//   pw group mod wheel -m hasegawa


QList<Network::Ethernet> Network::getEthernetList() {
	QList<Network::Ethernet> list;

	struct ifaddrs *ifap;
	int ret = ::getifaddrs(&ifap);
	logger.info("getifaddrs %d", ret);

	for(; ifap; ifap = ifap->ifa_next) {
		struct sockaddr *p = ifap->ifa_addr;
		if (p->sa_family == AF_LINK) {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifap->ifa_addr;
			if (sdl->sdl_alen != 0) {
				Network::Ethernet ethernet;

				// copy name
				{
					char* q = sdl->sdl_data;
					for(int i = 0; i < sdl->sdl_nlen; i++) {
						ethernet.name.append(q[i]);
					}
				}
				// copy address
				{
					char* q = sdl->sdl_data + sdl->sdl_nlen;
					Network::Ethernet::Address address(q);
					ethernet.address = address;
				}

				list += ethernet;
				logger.info("ethernet %s", ethernet.toString());
			}
		}
	}

	::freeifaddrs(ifap);

	return list;
}
