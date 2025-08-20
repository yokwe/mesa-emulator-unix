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
// AgentNetwork.cpp
//

#include "../util/Util.h"
static const util::Logger logger(__FILE__);


#include "../util/Debug.h"

#include "../mesa/Pilot.h"
#include "../mesa/Memory.h"
#include "../mesa/InterruptThread.h"
#include "../mesa/ProcessorThread.h"

#include "Agent.h"
#include "AgentNetwork.h"


int AgentNetwork::TransmitThread::stopThread;
void AgentNetwork::TransmitThread::enqueue(EthernetIOFaceGuam::EthernetIOCBType* iocb) {
	QMutexLocker locker(&transmitMutex);

	Item item(iocb);
	transmitQueue.push_back(item);
	if (transmitQueue.size() == 1) transmitCV.wakeOne();

	if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("TransmitThread receiveQueue.size = %d", transmitQueue.size());
}

void AgentNetwork::TransmitThread::run() {
	logger.info("AgentNetwork::TransmitThread::run START");
	if (networkPacket == 0) ERROR();

	int transmitCount = 0;
	stopThread = 0;
	QThread::currentThread()->setPriority(PRIORITY);

	try {
		for(;;) {
			if (stopThread) break;

			EthernetIOFaceGuam::EthernetIOCBType* iocb = 0;

			// minimize critical section
			{
				QMutexLocker locker(&transmitMutex);
				if (transmitQueue.empty()) {
					for(;;) {
						bool ret = transmitCV.wait(&transmitMutex, WAIT_INTERVAL);
						if (ret) break;
						if (stopThread) goto exitLoop;
					}
				}
				Item item = transmitQueue.front();
				iocb = item.iocb;
				transmitQueue.pop_front();
			}

			networkPacket->transmit(iocb);
			InterruptThread::notifyInterrupt(interruptSelector);
			transmitCount++;
		}
	} catch(Abort& e) {
		logger.fatal("Unexpected Abort %s %d %s", e.file, e.line, e.func);
		ProcessorThread::stop();
	}
exitLoop:
	logger.info("transmitCount          = %8u", transmitCount);
	logger.info("AgentNetwork::TransmitThread::run STOP");
}
void AgentNetwork::TransmitThread::reset() {
	QMutexLocker locker(&transmitMutex);
	transmitQueue.clear();
}


int AgentNetwork::ReceiveThread::stopThread;
uint64_t AgentNetwork::ReceiveThread::getSec() {
	return QDateTime::currentMSecsSinceEpoch() / 1000;
}

void AgentNetwork::ReceiveThread::enqueue(EthernetIOFaceGuam::EthernetIOCBType* iocb) {
	QMutexLocker locker(&receiveMutex);

	// convert form milliseconds to seconds
	int64_t sec = getSec();

	// TODO Is this correct?
	// Remove item which has same data
	// Remove item that is waiting more than MAX_WAIT_SEC.
	{
		CARD32 myBufferAddress = iocb->bufferAddress;

		for(auto i = receiveQueue.begin(); i != receiveQueue.end(); i++) {
			Item& item = *i;
			if (item.iocb->bufferAddress == myBufferAddress) receiveQueue.erase(i);
			if ((item.sec + MAX_WAIT_SEC) < sec) receiveQueue.erase(i);
		}
	}
	Item item(sec, iocb);
	receiveQueue.push_back(item);

	if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("ReceiveThread receiveQueue.size = %d", receiveQueue.size());
}

void AgentNetwork::ReceiveThread::run() {
	logger.info("AgentNetwork::ReceiveThread::run START");
	if (networkPacket == 0) ERROR();

	int receiveCount = 0;
	stopThread = 0;
	QThread::currentThread()->setPriority(PRIORITY);

	reset();
	for(;;) {
		if (stopThread) break;

		int opErrno = 0;
		// Below "1" means 1 second
		int ret = networkPacket->select(1, opErrno);
		if (ret == -1) {
			logger.fatal("%s  %d  select returns -1.  errno = %d", __FUNCTION__, __LINE__, opErrno);
			ERROR();
		}
		if (ret < 0) {
			logger.fatal("ret < 0.  ret = %d", ret);
			ERROR();
		}

		{
			QMutexLocker locker(&receiveMutex);
			if (ret == 0) {
				// data is not ready
				// do queue maintenance
				if (!receiveQueue.empty()) {
					int64_t sec = getSec();
					for(auto i = receiveQueue.begin(); i != receiveQueue.end(); i++) {
						Item& item = *i;
						if ((item.sec + MAX_WAIT_SEC) < sec) {
							receiveQueue.erase(i);
							if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("remove old item.  receiveQueue.size = %d", receiveQueue.size());
						}
					}
				}
				continue;
			} else {
				// data is ready
				if (receiveQueue.empty()) {
					// there is no item in queue, discard packet
					networkPacket->discardOnePacket();
				} else {
					// there is item in queue
					// remove one item from queue
					Item& item = receiveQueue.front();
					EthernetIOFaceGuam::EthernetIOCBType* iocb = item.iocb;
					receiveQueue.pop_front();

					// use this iocb to receive packet
					networkPacket->receive(iocb);
					InterruptThread::notifyInterrupt(interruptSelector);
					receiveCount++;
				}
			}
		}
	}
	logger.info("receiveCount           = %8u", receiveCount);
	logger.info("AgentNetwork::ReceiveThread::run STOP");
}
void AgentNetwork::ReceiveThread::reset() {
	QMutexLocker locker(&receiveMutex);
	receiveQueue.clear();
	networkPacket->discardRecievedPacket();
}



void AgentNetwork::Initialize() {
	if (fcbAddress == 0) ERROR();
	if (networkPacket == 0) ERROR();

	fcb = (EthernetIOFaceGuam::EthernetFCBType *)Store(fcbAddress);

	fcb->receiveIOCB               = 0;
	fcb->transmitIOCB              = 0;
	fcb->receiveInterruptSelector  = 0;
	fcb->transmitInterruptSelector = 0;
	fcb->stopAgent                 = 0;
	fcb->receiveStopped            = 1;
	fcb->transmitStopped           = 1;
	fcb->hearSelf                  = 0;

	// use local variable to avoid gcc 9 warning
	CARD16 id0, id1, id2;
	networkPacket->getAddress(id0, id1, id2);
	fcb->processorID[0] = id0;
	fcb->processorID[1] = id1;
	fcb->processorID[2] = id2;

	fcb->packetsMissed             = 0;
	fcb->agentBlockSize            = 0;

	receiveThread.setNetworkPacket(networkPacket);
	receiveThread.setAutoDelete(false);

	transmitThread.setNetworkPacket(networkPacket);
	transmitThread.setAutoDelete(false);
}

void AgentNetwork::Call() {
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
			transmitThread.reset();
			transmitThread.setInterruptSelector(fcb->transmitInterruptSelector);
		}
		fcb->receiveStopped  = 0;
		fcb->transmitStopped = 0;
	}
	if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("AGENT %s  receiveIOCB = %08X  transmitIOCB = %08X", name, fcb->receiveIOCB + 0, fcb->transmitIOCB + 0);

	if (fcb->receiveIOCB) {
		EthernetIOFaceGuam::EthernetIOCBType* iocb = (EthernetIOFaceGuam::EthernetIOCBType*)Store(fcb->receiveIOCB);

		for(;;) {
			CARD16 packetType= iocb->packetType;
			if (packetType != EthernetIOFaceGuam::PT_receive) ERROR();

			if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("AGENT %s  receive  status = %04X  nextIOCB = %08X", name, iocb->status + 0, iocb->nextIOCB);
			receiveThread.enqueue(iocb);
			//
			if (iocb->nextIOCB == 0) break;
			iocb = (EthernetIOFaceGuam::EthernetIOCBType*)Store(iocb->nextIOCB);
		}
	}

	if (fcb->transmitIOCB) {
		EthernetIOFaceGuam::EthernetIOCBType* iocb = (EthernetIOFaceGuam::EthernetIOCBType*)Store(fcb->transmitIOCB);

		for(;;) {
			CARD16 packetType= iocb->packetType;
			if (packetType != EthernetIOFaceGuam::PT_transmit) ERROR();

			if (DEBUG_SHOW_AGENT_NETWORK) logger.debug("AGENT %s  transmit status = %04X  nextIOCB = %08X", name, iocb->status + 0, iocb->nextIOCB);
			transmitThread.enqueue(iocb);
			//
			if (iocb->nextIOCB == 0) break;
			iocb = (EthernetIOFaceGuam::EthernetIOCBType*)Store(iocb->nextIOCB);
		}
	}
}
