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
static const Logger logger = Logger::getLogger("net-dummy");

#include "Network.h"

static QList<Network::Device> getDeviceList_() {
	QList<Network::Device> list;
	return list;
}
QList<Network::Device> Network::getDeviceList() {
	static QList<Network::Device> list = getDeviceList_();
	return list;
}

class Dummy : public Network::Driver {
public:
	int select  (quint32 timeout, int& opErrno); // returns return value of of select().  default timeout is 1 second
	int transmit(quint8* data, quint32 dataLen, int& opErrno);   // returns return value of send()
	int receive (quint8* data, quint32 dataLen, int& opErrno, quint64* msecSinceEpoch);   // returns return value of of recv()

	// discard received packet
	void discard();

	Dummy(const Network::Device& device) : Network::Driver(device) {}
};
int Dummy::select(quint32 timeout, int& opErrno) {
	(void)timeout;
	opErrno = 0;
	return 0;
}
int Dummy::transmit(quint8* data, quint32 dataLen, int& opErrno) {
	(void)data;
	(void)dataLen;
	opErrno = ENETDOWN;
	return 0;
}
int Dummy::receive (quint8* data, quint32 dataLen, int& opErrno, quint64* msecSinceEpoch) {
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
