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


#include "Util.h"
static const Logger logger(__FILE__);

#include "Network.h"

static std::vector<Network::Device> getDeviceList_() {
	std::vector<Network::Device> list;
	return list;
}
std::vector<Network::Device> Network::getDeviceList() {
	static std::vector<Network::Device> list = getDeviceList_();
	return list;
}

class Dummy : public Network::Driver {
public:
	int select  (uint32_t timeout, int& opErrno); // returns return value of of select().  default timeout is 1 second
	int transmit(uint8_t* data, uint32_t dataLen, int& opErrno);   // returns return value of send()
	int receive (uint8_t* data, uint32_t dataLen, int& opErrno, int64_t* msecSinceEpoch);   // returns return value of of recv()

	// discard received packet
	void discard();

	Dummy(const Network::Device& device) : Network::Driver(device) {}
};
int Dummy::select(uint32_t timeout, int& opErrno) {
	(void)timeout;
	opErrno = 0;
	return 0;
}
int Dummy::transmit(uint8_t* data, uint32_t dataLen, int& opErrno) {
	(void)data;
	(void)dataLen;
	opErrno = ENETDOWN;
	return 0;
}
int Dummy::receive (uint8_t* data, uint32_t dataLen, int& opErrno, int64_t* msecSinceEpoch) {
	(void)data;
	(void)dataLen;
	opErrno = ENETDOWN;
	(void)msecSinceEpoch;
	return 0;
}
void Dummy::discard () {
}

Network::Driver* Network::getDriver(const Network::Device& device) {
	Dummy* ret = new Dummy(device);
	return ret;
}
