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
// SPPQueue.h
//

#pragma once

#include <functional>

#include <QtConcurrent/QtConcurrent>

#include "SPPListener.h"


// FIXME When remote side behave wrong, close connection and delete socket.
// FIXME If we didn't get packet more then 30 seconds, close connection and delete socket.
// FIXME   other end is down
// FIXME If we retransmit same packet more than 30 seconds, close connection and delete socket.
// FIXME   other end is up but not accept packet

// FIXME process ATTENTION packet

// When sending data to remote,
// sendSeq is used as seq
// recvSeq is used as ack and alloc (window size is one)

// When receiving data from remote,
// if seq equals to recvSeq, use receive this data and increment recvSeq.
// if ack equals to sendSeq plus one, increment sendSeq.
// if there is a data in sendQeueu, send data with updated recvSeq and sendSeq.
// if there is no data in sendQeueu, send system ack packet

// The Sequence Number counts packets sent on the connection.
// The first packet is assigned number zero, and the count proceeds from there.
// If the count overflows the 16-bit field, the overflow is ignored and the count proceeds from zero again.

// The Acknowledge Number field specifies the sequence number of the first packet which has not yet been seen traveling in the reverse direction.
// Acknowledge Number indicates the sequence number of the next expected packet.

// The Allocation Number specifies the sequence number up to and including which packets will be accepted from the other end.
// One plus the difference between the Allocation Number and the Acknowledge Number indicates the number of packets that may be outstanding in the reverse direction

// Packets with the Attention bit set must have only one byte of data.


namespace XNS::Server {
	// forward declaration
	class Server;
	class SPPQueue;

	class SPPQueueServer : public SPPListener {
	public:
		class Context {
		public:
			QByteArray newName;
			uint64_t    time;

			uint64_t    remoteHost;
			uint16_t    remoteSocket;
			uint16_t    remoteID;

			uint16_t    localSocket;
			uint16_t    localID;

			Context() : time(0), remoteHost(0), remoteSocket(0), remoteID(0), localSocket(0), localID(0) {}
		};

		SPPQueueServer(SPPQueue* impl);

		void init (Server* server) {
			myServer = server;
		}
		void start();
		void stop () {}

		void handle(const Data& data, const SPP& spp);

	private:
		SPPQueue* myImpl;
		Server*   myServer;
	};


	class SPPQueue : public SPPListener {
	public:
		SPPQueue(const char* name, uint16_t socket);
		SPPQueue(const SPPQueue& that);

		virtual ~SPPQueue() {}

		void init(Server* server);
		void start();
		void stop();

		void handle(const Data& data, const SPP& spp);

		// clone method for SPPQueue
		virtual SPPQueue* clone() = 0;
		// create copy constructor and call it in clone()
		// return new SPPServerImpl(*this);

		void set(const SPPQueueServer::Context& context);

		void sendAck(const Data& data);

	protected:
		class QueueData {
			void copyFrom(const Data& data_, const SPP& spp_);
			void fixBlock();
		public:
			Data    data;
			SPP     spp;

			QueueData();
			~QueueData();
			QueueData(const QueueData& that);
			QueueData& operator = (const QueueData& that);

			QueueData(const Data& data_, const SPP& spp_);
		};


		//
		// FunctionTable for run
		//

		// if recv returns nullptr, no data is arrived for now
		QueueData* recv();
		void       send(const Data& data, const SPP& spp);
		void       close();
		bool       stopRun();
		Config*    getConfig();
		Context*   getContext();
		Listeners* getListeners();
		Services*  getServices();

		class FunctionTable {
		public:
			std::function<QueueData*()>      recv;
			std::function<void(Data&, SPP&)> send;
			std::function<void(void)>        close;
			std::function<bool(void)>        stopRun;
			std::function<Config*(void)>     getConfig;
			std::function<Context*(void)>    getContext;
			std::function<Listeners*(void)>  getListeners;
			std::function<Services*(void)>   getServices;
		};
		FunctionTable functionTable;

		virtual void run(FunctionTable functionTable) = 0;


	private:
		class Buffer {
		public:
			class Entry {
			public:
				uint16_t    seq;
				QueueData *myData;

				Entry() : seq(0), myData(nullptr) {}
				Entry(const Entry& that) {
					this->seq    = that.seq;
					this->myData = that.myData;
				}
				Entry& operator = (const Entry& that) {
					this->seq    = that.seq;
					this->myData = that.myData;
					return *this;
				}

				Entry(uint16_t seq_) : seq(seq_), myData(nullptr) {}

				bool inUse() {
					return myData != nullptr;
				}
				std::string toString();
			};

			~Buffer();
			// ger return nullptr if entry is not found
			Entry* get  (uint16_t seq);
			Entry* alloc(uint16_t seq);
			void   free (uint16_t seq);

			void   clear();
			int    countFree();
			bool   isEmpty() {
				return map.isEmpty();
			}

			std::string toString();
		private:
			std::map<uint16_t, Entry*> map;
			//   seq
		};

		void allocNext(uint16_t seq);

		void runThread();
		void sendThread();
		void transmitThread();

		void transmit(QueueData* myData);

		//
		// variable for access context, config, listeners and services
		//
		Server* myServer;

		//
		// variables for future
		//
		std::atomic_uint    stopFuture;
		std::atomic_uint    stopIsCalled;
		std::atomic_uint    closeIsCalled;
		QFuture<void> futureRun;
		QFuture<void> futureSend;
		QFuture<void> futureTransmit;

		//
		// variable for temporary receiving data
		//
		Buffer recvBuffer;

		//
		// variables for recvList
		//
		uint16_t           recvListSeq;
		std::vector<QueueData*> recvList;
		std::mutex           recvListMutex;
		std::condition_variable    recvListCV;

		//
		// variable for temporary sending data
		//
		Buffer         sendBuffer;
		std::mutex        sendBufferMutex;
		std::condition_variable sendBufferCV;

		//
		// variable for transmitList
		std::vector<QueueData*> transmitList;
		std::mutex           transmitListMutex;
		std::condition_variable    transmitListCV;

		//
		// variables for recv/send
		//
		QByteArray newName;
		uint64_t    time;
		uint64_t    remoteHost;
		uint16_t    remoteSocket;
		uint16_t    remoteID;
		uint16_t    localSocket;
		uint16_t    localID;

		// sendSeq is used as spp.seq in transmit
		// The Sequence Number counts data packets sent on the connection.
		std::atomic_uinteger<uint16_t> sendSeq;
		// recvSeq is used as spp.ack in transmit
		// Acknowledge Number indicates the sequence number of the next data packet.
		std::atomic_uinteger<uint16_t> recvSeq;

		//
		// variables for transmit packet
		//
		Driver* driver;
		uint32_t localNet;
		uint64_t localHost;

	};

}
