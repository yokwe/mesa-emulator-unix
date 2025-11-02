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
// AgentNetwork.cpp
//

#include "../util/Debug.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/net.h"
#include "../util/Perf.h"

#include "../mesa/Pilot.h"
#include "../mesa/memory.h"
#include "../mesa/processor_thread.h"

#include "AgentNetwork.h"


void AgentNetwork::TransmitThread::process(const Item& item) {
	PERF_COUNT(network, transmit)
	auto interruptSelector = item.interruptSelector;
	auto iocb = item.iocb;

	if (iocb == 0) ERROR();
	if (iocb->bufferLength == 0) ERROR();
	if (iocb->bufferAddress == 0) ERROR();

	CARD32 dataLen = iocb->bufferLength;
	CARD8* data    = (CARD8*)memory::peek(iocb->bufferAddress);
	CARD8  buffer[net::PACKET_SIZE];

	// sanity check for odd byte and minimu packet length
	if (dataLen & 1 || dataLen < net::minBytesPerEthernetPacket) {
		logger.fatal("dataLen  %d", dataLen);
		ERROR()
	}
	// byteswap and copy from data to buffer
	Util::byteswap((CARD16*)data, (CARD16*)buffer, (dataLen + 1) / 2);
	int ret = driver->transmit(buffer, dataLen);

	if (ret == -1) {
		// set iocb->status if possibble

		//static const CARD16 S_inProgress              =   1;
		//static const CARD16 S_completedOK             =   2;
		//static const CARD16 S_tooManyCollisions       =   4;
		//static const CARD16 S_badCRC                  =   8;
		//static const CARD16 S_alignmentError          =  16;
		//static const CARD16 S_packetTooLong           =  32;
		//static const CARD16 S_bacCRDAndAlignmentError = 128;
		ERROR();
	}
	if (ret != (int)dataLen) {
		logger.fatal("%s  %d  ret != dataLen.  ret = %d  dataLen = %d", __FUNCTION__, __LINE__, ret, dataLen);
		ERROR();
	}
	iocb->status = EthernetIOFaceGuam::S_completedOK;
	processor_thread::notifyInterrupt(interruptSelector);
}

void AgentNetwork::ReceiveThread::push(const Item& item) {
	PERF_COUNT(network, receive_request)
	std::unique_lock<std::mutex> lock(mutex);
	queue.push_front(item);
	// {
	// 	auto iocb = item.iocb;
	// 	auto status = iocb->status;
	// 	logger.info("ReceiveThread  enqueue  %8X  %4d  %d", iocb->bufferAddress, iocb->bufferLength, status);
	// }
}

void AgentNetwork::ReceiveThread::run() {
	stopThread = false;
	for(;;) {
		if (stopThread) break;
		net::Packet packet;
		if (driver->read(packet, Util::ONE_SECOND)) {
			PERF_COUNT(network, receive_packet)
			std::unique_lock<std::mutex> lock(mutex);
			if (!queue.empty()) {
				const auto& item = queue.back();
				// process item
				process(item, packet);
				queue.pop_back();
			}
		}
	}
}

void AgentNetwork::ReceiveThread::process(const Item& item, const ByteBuffer& packet) {
	PERF_COUNT(network, receive_process)
	auto interruptSelector = item.interruptSelector;
	auto iocb = item.iocb;

	if (iocb == 0) ERROR();
	if (iocb->bufferLength == 0) ERROR();
	if (iocb->bufferAddress == 0) ERROR();

	CARD8* data    = (CARD8*)memory::peek(iocb->bufferAddress);
	CARD32 dataLen = iocb->bufferLength;
	
	CARD8* buffer    = packet.data();
	CARD32 bufferLen = packet.length();

	if (bufferLen < dataLen) {
		// byteswap and copy from buffer to data
		Util::byteswap((uint16_t*)buffer, (CARD16*)data, (bufferLen + 1) / 2);
		iocb->actualLength = bufferLen;
		iocb->status = EthernetIOFaceGuam::S_completedOK;
	} else {
		iocb->status = EthernetIOFaceGuam::S_packetTooLong;
	}
	processor_thread::notifyInterrupt(interruptSelector);
	// {
	// 	auto status = iocb->status;
	// 	logger.info("ReceiveThread  dequeue  %8X  %4d  %d", iocb->bufferAddress, iocb->bufferLength, status);
	// }
}


void AgentNetwork::Initialize() {
	if (fcbAddress == 0) ERROR();
	if (driver == 0) ERROR();

	fcb = (EthernetFCBType *)memory::peek(fcbAddress);

	fcb->receiveIOCB               = 0;
	fcb->transmitIOCB              = 0;
	fcb->receiveInterruptSelector  = 0;
	fcb->transmitInterruptSelector = 0;
	fcb->stopAgent                 = 0;
	fcb->receiveStopped            = 1;
	fcb->transmitStopped           = 1;
	fcb->hearSelf                  = 0;

	driver->device.getAddress(fcb->processorID[0], fcb->processorID[1], fcb->processorID[2]);

	fcb->packetsMissed             = 0;
	fcb->agentBlockSize            = 0;

	transmitThread.set(driver);
	receiveThread.set(driver);
}

void AgentNetwork::Call() {
	PERF_COUNT(agent, network)
	if (fcb->stopAgent) {
		if (!fcb->receiveStopped) {
			logger.info("AGENT %s  stop", name);
		}
		fcb->receiveStopped  = 1;
		fcb->transmitStopped = 1;
		// Do nothing when ask to stop
		return;
	} else {
		if (fcb->receiveStopped) {
			logger.info("AGENT %s  start  %04X %04X", name, fcb->transmitInterruptSelector + 0, fcb->receiveInterruptSelector + 0);
		}
		fcb->receiveStopped  = 0;
		fcb->transmitStopped = 0;
	}
	if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("AGENT %s  receiveIOCB = %08X  transmitIOCB = %08X", name, fcb->receiveIOCB + 0, fcb->transmitIOCB + 0);

	if (fcb->receiveIOCB) {
		EthernetIOCBType* iocb = (EthernetIOCBType*)Store(fcb->receiveIOCB);

		for(;;) {
			CARD16 packetType= iocb->packetType;
			if (packetType != EthernetIOFaceGuam::PT_receive) ERROR();

			if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("AGENT %s  receive  status = %04X  nextIOCB = %08X", name, iocb->status + 0, iocb->nextIOCB);
			Item item(fcb->receiveInterruptSelector, iocb);
			receiveThread.push(item);
			//
			if (iocb->nextIOCB == 0) break;
			iocb = (EthernetIOCBType*)Store(iocb->nextIOCB);
		}
	}

	if (fcb->transmitIOCB) {
		EthernetIOCBType* iocb = (EthernetIOCBType*)Store(fcb->transmitIOCB);

		for(;;) {
			CARD16 packetType= iocb->packetType;
			if (packetType != EthernetIOFaceGuam::PT_transmit) ERROR();

			if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("AGENT %s  transmit status = %04X  nextIOCB = %08X", name, iocb->status + 0, iocb->nextIOCB);

			Item item(fcb->transmitInterruptSelector, iocb);
			transmitThread.push(item);
			//
			if (iocb->nextIOCB == 0) break;
			iocb = (EthernetIOCBType*)Store(iocb->nextIOCB);
		}
	}
}
