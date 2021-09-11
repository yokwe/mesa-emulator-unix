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
// SPPQueue.h
//

#pragma once

#include <functional>

#include <QtConcurrent/QtConcurrent>

#include "SPPListener.h"


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
			quint64    time;

			quint64    remoteHost;
			quint16    remoteSocket;
			quint16    remoteID;

			quint16    localSocket;
			quint16    localID;

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
		SPPQueue(const char* name, quint16 socket) :
			SPPListener(name, socket), myServer(nullptr),
			time(0), remoteHost(0), remoteSocket(0), remoteID(0), localSocket(0), localID(0), sendSeq(0), recvSeq(0),
			driver(nullptr), localNet(0), localHost(0) {}
		SPPQueue(const SPPQueue& that)             :
			SPPListener(that),         myServer(nullptr),
			time(0), remoteHost(0), remoteSocket(0), remoteID(0), localSocket(0), localID(0), sendSeq(0), recvSeq(0),
			driver(nullptr), localNet(0), localHost(0) {}

		virtual ~SPPQueue() {}

		void init(Server* server);
		void start();
		void stop();

		void handle(const Data& data, const SPP& spp);

		// clone method for SPPQueue
		virtual SPPQueue* clone() = 0;
		// create copy constructor and call it in clone()
		// return new SPPServerImpl(*this);

		void set(const SPPQueueServer::Context& context) {
			newName      = context.newName;
			time         = context.time;
			remoteHost   = context.remoteHost;
			remoteSocket = context.remoteSocket;
			remoteID     = context.remoteID;
			localSocket  = context.localSocket;
			localID      = context.localID;
		}

		void sendAck(const Data& data);

	protected:
		class QueueData {
			void copyFrom(const quint16 seq_, const Data& data_, const SPP& spp_);
			void fixBlock();
		public:
			// seq is for RecvBuffer
			quint16 seq;
			Data    data;
			SPP     spp;

			QueueData();
			~QueueData();
			QueueData(const QueueData& that);
			QueueData& operator = (const QueueData& that);

			QueueData(const quint16 seq_, const Data& data_, const SPP& spp_);
			QueueData(const Data& data_, const SPP& spp_) : QueueData(0, data_, spp_) {}

			void empty() {
				data.timeStamp = 0;
			}
			bool isEmpty() {
				return data.timeStamp == 0;
			}
		};


		// if recv returns nullptr, no data is arrived for nwo
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
			std::function<QueueData*()>       recv;
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
		class RecvBuffer {
			static const int SIZE = 4;
		public:
			RecvBuffer() {
				for(int i = 0; i < SIZE; i++) {
					array[i].seq = i;
					array[i].empty();
				}
			}

			QueueData array[SIZE];

			void clear() {
				for(int i = 0; i < SIZE; i++) {
					array[i].empty();
				}
			}
			int countEmpty() {
				int ret = 0;
				for(int i = 0; i < SIZE; i++) {
					if (array[i].isEmpty()) ret++;
				}
				return ret;
			}
			bool add(const QueueData& newValue) {
				for(int i = 0; i < SIZE; i++) {
					QueueData *p = array + i;

					if (p->isEmpty()) {
						*p = newValue;
						return true;
					}
				}
				return false;
			}
			QueueData* get(quint16 seq) {
				for(int i = 0; i < SIZE; i++) {
					QueueData *p = array + i;

					if (p->isEmpty()) continue;
					if (p->spp.seq == seq) {
						return p;
					}
				}
				return nullptr;
			}
			bool exist(quint16 seq) {
				return get(seq) != nullptr;
			}
			QueueData* getYougest() {
				QueueData* ret = nullptr;
				for(int i = 0; i < SIZE; i++) {
					QueueData *p = array + i;

					if (p->isEmpty()) continue;
					if (ret == nullptr) {
						ret = p;
					} else {
						if (p->data.timeStamp < ret->data.timeStamp) {
							ret = p;
						}
					}
				}
				return ret;
			}

		};

		void runThread();
		void sendThread();

		void transmit(const Data& data, const SPP& spp);

		Server* myServer;

		QAtomicInt    stopFuture;
		QAtomicInt    stopIsCalled;
		QAtomicInt    closeIsCalled;
		QFuture<void> futureRun;
		QFuture<void> futureSend;

		QList<QueueData*> recvList;
		QMutex            recvListMutex;
		QWaitCondition    recvListCV;

		QList<QueueData*> sendList;
		QMutex            sendListMutex;
		QWaitCondition    sendListCV;

		RecvBuffer recvBuffer;

		// context
		QByteArray newName;
		quint64    time;
		quint64    remoteHost;
		quint16    remoteSocket;
		quint16    remoteID;
		quint16    localSocket;
		quint16    localID;
		quint16    sendSeq;
		quint16    recvSeq;

		// for transmit
		Driver* driver;
		quint32 localNet;
		quint64 localHost;

	};

}
