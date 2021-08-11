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

#include <net/bpf.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include "BPF.h"
#include "Network.h"

void BPF::open() {
	{
		const char pathPattern[] = "/dev/bpf00";
		char buf[sizeof(pathPattern)];

		for(int i = 0; i < 99; i++) {
			sprintf(buf, "/dev/bpf%d", i);
			fd = ::open(buf, O_RDWR);
			if (fd < 0) {
				int opErrno = errno;
				LOG_ERRNO(opErrno);
				continue;
			}
			break;
		}
		path = buf;
	}
	// BIOCGBLEN
	//   Returns the required buffer length for reads on bpf files.
	{
		int value;
		int ret;

		CHECK_SYSCALL(ret, ::ioctl(fd, BIOCGBLEN, &value))
		bufferSize = value;
		buffer     = new quint8[bufferSize];
	}

	logger.info("fd     %d", fd);
	logger.info("buffer %d", bufferSize);
	logger.info("path   %s", path);
}
void BPF::close() {
	if (0 <= fd) {
		int ret;
		LOG_SYSCALL(ret, ::close(fd))
		fd = -1;
	}
	delete buffer;
}

void BPF::attach(const QString& name_) {
	name = name_;
	logger.info("device %s", name);

	int ret;

	// BIOCSETIF
	//   Sets the hardware interface associated with the file
	{
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		::strncpy(ifr.ifr_name, TO_CSTRING(name), IFNAMSIZ - 1);
		CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSETIF, &ifr))
	}
	// BIOCPROMISC
	//   Forces the	interface into promiscuous mode
	{
		CHECK_SYSCALL(ret, ::ioctl(fd, BIOCPROMISC, NULL))
	}
	// BIOCIMMEDIATE
	//   Enables or	disables "immediate mode"
	{
		int value = 1; // 1 for immediate mode
		CHECK_SYSCALL(ret, ::ioctl(fd, BIOCIMMEDIATE, &value))
	}
	// BIOCSHDRCMPLT
	//   Sets the status of	the "header complete" flag
	{
		int value = 1; // 1 for no automatic setting of source address
		CHECK_SYSCALL(ret, ::ioctl(fd, BIOCSHDRCMPLT, &value))
	}

}
