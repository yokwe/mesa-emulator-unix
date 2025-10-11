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
// AgentNetwork.h
//

#pragma once

#include <mutex>
#include <condition_variable>
#include <deque>

#include "../util/net.h"

#include "Agent.h"

class AgentNetwork : public Agent {
public:
        using EthernetIOCBType = EthernetIOFaceGuam::EthernetIOCBType;
        using EthernetFCBType  = EthernetIOFaceGuam::EthernetFCBType;
	class TransmitThread {
	public:
		static void stop();

		TransmitThread() : interruptSelector(0), driver(0) {
			transmitQueue.clear();
		}

		void setInterruptSelector(CARD16 interruptSelector_) {
			interruptSelector = interruptSelector_;
		}
		void setDriver(net::Driver* driver_) {
			driver = driver_;
		}

		void enqueue(EthernetIOCBType* iocb);
		void transmit(EthernetIOCBType* iocb);

		void run();
		void reset();

	private:
		class Item {
		public:
			EthernetIOCBType* iocb;

			Item(EthernetIOCBType* iocb_) : iocb(iocb_) {}
			Item(const Item& that) : iocb(that.iocb) {}
		};

		static inline int stopThread = 0;

		CARD16                  interruptSelector;
		net::Driver*            driver;

		std::mutex              transmitMutex;
		std::condition_variable transmitCV;
		std::deque<Item>        transmitQueue;
	};
	class ReceiveThread {
	public:
		static const CARD32 MAX_WAIT_SEC = 40;

		static void stop();

		ReceiveThread() : interruptSelector(0), driver(0) {
			receiveQueue.clear();
		}

		void setInterruptSelector(CARD16 interruptSelector_) {
			interruptSelector = interruptSelector_;
		}
		void setDriver(net::Driver* driver_) {
			driver = driver_;
		}

		void enqueue(EthernetIOCBType* iocb);
		void receive(EthernetIOCBType* iocb);
		void discardOnePacket();

		void run();
		void reset();

		uint64_t getSec();

	private:
		class Item {
		public:
			// queued time in second
			int64_t                                sec;
			EthernetIOCBType* iocb;

			Item(int64_t sec_, EthernetIOCBType* iocb_) : sec(sec_), iocb(iocb_) {}
//			Item(const Item& that) : sec(that.sec), iocb(that.iocb) {}
		};

		static inline int stopThread = 0;

		CARD16           interruptSelector;
		net::Driver*     driver;

		std::mutex       receiveMutex;
		std::deque<Item> receiveQueue;
	};

	ReceiveThread  receiveThread;
	TransmitThread transmitThread;

	static const inline auto index_ = GuamInputOutput::AgentDeviceIndex::network;
	static const inline auto name_ = "Network";
	static const inline auto fcbSize_ = SIZE(EthernetFCBType);
	AgentNetwork() : Agent(index_, name_, fcbSize_), fcb(0), driver(0) {}

	void Initialize();
	void Call();

	void poll();

	void setDriver(net::Driver* driver_) {
		driver = driver_;
	}

private:
	EthernetFCBType* fcb;
	net::Driver*     driver;
};
