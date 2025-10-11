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

#include <chrono>
#include <condition_variable>

#include "../util/Debug.h"
#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/net.h"

#include "../mesa/Pilot.h"
#include "../mesa/memory.h"
#include "../mesa/interrupt_thread.h"
#include "../mesa/processor_thread.h"

#include "AgentNetwork.h"


void AgentNetwork::TransmitThread::process(const TransmitItem& item) {
	auto interruptSelector = item.interruptSelector;
	auto iocb = item.iocb;
	auto driver = item.driver;

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
	// byteswap and copy from data to to buffer
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
	interrupt_thread::notifyInterrupt(interruptSelector);
}

void AgentNetwork::ReceiveThread::stop() {
	logger.info("AgentNetwork::ReceiveThread::stop");
	stopThread = 1;
}

void AgentNetwork::ReceiveThread::enqueue(EthernetIOCBType* iocb) {
	std::unique_lock<std::mutex> locker(receiveMutex);

	int64_t sec = Util::getSecondsSinceEpoch();

	// TODO Is this correct?
	// Remove item which has same data
	// Remove item that is waiting more than MAX_WAIT_SEC.
	{

		// remove too old entry
		// for(;;) {
		// 	if (receiveQueue.empty()) break;
		// 	auto& item = receiveQueue.front();
		// 	if ((item.sec + MAX_WAIT_SEC) < sec) {
		// 		receiveQueue.pop_back();
		// 		continue;
		// 	}
		// 	break;
		// }

		// CARD32 myBufferAddress = iocb->bufferAddress;
		// for(auto i = receiveQueue.begin(); i != receiveQueue.end(); i++) {
		// 	const Item& item = *i;
		// 	if (item.iocb->bufferAddress == myBufferAddress) receiveQueue.erase(i);
		// }
	}
	Item item(sec, iocb);
	receiveQueue.push_front(item);

	if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("ReceiveThread receiveQueue.size = %d", receiveQueue.size());
}
void AgentNetwork::ReceiveThread::receive(EthernetIOCBType* iocb) {
	if (iocb == 0) ERROR();
	if (iocb->bufferLength == 0) ERROR();
	if (iocb->bufferAddress == 0) ERROR();

	CARD8* data    = (CARD8*)memory::peek(iocb->bufferAddress);
	CARD32 dataLen = iocb->bufferLength;
	
	CARD8 buffer[net::PACKET_SIZE];
	int ret;
	for(;;) {
		ret = driver->receive(buffer, sizeof(buffer), std::chrono::seconds(60 * 10));
		if (0 < ret) break;
		if (ret == 0) {
			logger.info("AgentNetwork::ReceiveThread receive timeout");
			continue;
		}
		logger.fatal("ret %d", ret);
		ERROR();
	}
	//static const CARD16 S_inProgress              =   1;
	//static const CARD16 S_completedOK             =   2;
	//static const CARD16 S_tooManyCollisions       =   4;
	//static const CARD16 S_badCRC                  =   8;
	//static const CARD16 S_alignmentError          =  16;
	//static const CARD16 S_packetTooLong           =  32;
	//static const CARD16 S_bacCRDAndAlignmentError = 128;

	if (dataLen < (CARD32)ret) {
		iocb->status = EthernetIOFaceGuam::S_packetTooLong;
		return;
	}

	Util::byteswap((uint16_t*)buffer, (CARD16*)data, (ret + 1) / 2);
	iocb->actualLength = ret;
	iocb->status = EthernetIOFaceGuam::S_completedOK;
}

void AgentNetwork::ReceiveThread::run() {
	logger.info("AgentNetwork::ReceiveThread::run START");
	if (driver == 0) ERROR();

	stopThread = 0;
	
	reset();
	for(;;) {
		if (stopThread) break;

		// Below "1" means 1 second
		PERF_COUNT(network, select)
		int ret = driver->select(Util::ONE_SECOND);
		if (ret == -1) {
			ERROR();
		}
		if (ret < 0) {
			logger.fatal("ret < 0.  ret = %d", ret);
			ERROR();
		}

		if (ret == 0) continue;

		// When data is arrived but noone want to receive, dicards received data
		if (receiveQueue.empty()) {
			PERF_COUNT(network, discard)
			driver->clear();
			continue;
		}

		{
			std::unique_lock<std::mutex> locker(receiveMutex);
			// data is ready
			if (receiveQueue.empty()) {
				// there is no item in queue, discard packet
				discardOnePacket();
			} else {
				// there is item in queue
				// remove one item from queue
				Item& item = receiveQueue.back();
				EthernetIOCBType* iocb = item.iocb;
				receiveQueue.pop_back();

				// use this iocb to receive packet
				receive(iocb);
				interrupt_thread::notifyInterrupt(interruptSelector);
				PERF_COUNT(network, receive)
			}
		}
	}
	logger.info("AgentNetwork::ReceiveThread::run STOP");
}
void AgentNetwork::ReceiveThread::reset() {
	std::unique_lock<std::mutex> locker(receiveMutex);
	receiveQueue.clear();
	driver->clear();
}
void AgentNetwork::ReceiveThread::discardOnePacket() {
	net::Packet packet;
	driver->receive(packet.data(), packet.capacity(), Util::ONE_SECOND);
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

	receiveThread.setDriver(driver);
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
			receiveThread.reset();
			receiveThread.setInterruptSelector(fcb->receiveInterruptSelector);
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
			iocb->status = EthernetIOFaceGuam::S_inProgress;
			receiveThread.enqueue(iocb);
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

			TransmitItem item(fcb->transmitInterruptSelector, iocb, driver);
			transmitThread.push(item);
			//
			if (iocb->nextIOCB == 0) break;
			iocb = (EthernetIOCBType*)Store(iocb->nextIOCB);
		}
	}
}
