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


namespace XNS::Server {
	// forward declaration
	class Server;

	class SPPQueue : public SPPListener {
	public:
		class State {
			void copyFrom(const State& that) {
				this->name         = that.name;
				this->time         = that.time;

				this->remoteHost   = that.remoteHost;
				this->remoteSocket = that.remoteSocket;
				this->remoteID     = that.remoteID;

				this->localSocket  = that.localSocket;
				this->localID      = that.localID;
			}
		public:
			QByteArray name;
			quint64    time;

			quint64    remoteHost;
			quint16    remoteSocket;
			quint16    remoteID;

			quint16    localSocket;
			quint16    localID;

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


			State() : name(nullptr), time(0), remoteHost(0), remoteSocket(0), remoteID(0), localSocket(0), localID(0) {}
			State(const State& that) {
				copyFrom(that);
			}
			State& operator = (const State& that) {
				copyFrom(that);
				return *this;
			}
		};

		SPPQueue(const char* name, quint16 socket) : SPPListener(name, socket), myServer(nullptr), sendSeq(0), recvSeq(0), driver(nullptr), localNet(0), localHost(0) {}
		SPPQueue(const SPPQueue& that)             : SPPListener(that),         myServer(nullptr), sendSeq(0), recvSeq(0), driver(nullptr), localNet(0), localHost(0) {}

		virtual ~SPPQueue() {}

		void init(Server* server);
		void start();
		void stop();

		void handle(const Data& data, const SPP& spp);

		// clone method for SPPQueue
		virtual SPPQueue* clone() = 0;
		// create copy constructor and call it in clone()
		// return new SPPServerImpl(*this);

		void state(const State& state_) {
			myState = state_;
		}

		void sendAck(const Data& data);

	protected:
		class RecvData {
			void copyFrom(const quint16 seq_, const Data& data_, const SPP& spp_) {
				seq  = seq_;
				data = data_;
				spp  = spp_;

				// reflect change of address of data.packet
				BLOCK newValue(data.packet);
				spp.updateBlock(newValue);
			}
		public:
			// seq is for RecvBuffer
			quint16 seq;
			Data    data;
			SPP     spp;

			RecvData() {
				seq = 0;
				// reflect change of address of data.packet
				BLOCK newValue(data.packet);
				spp.updateBlock(newValue);
			}
			RecvData(const RecvData& that) : RecvData(that.seq, that.data, that.spp) {}
			RecvData& operator = (const RecvData& that) {
				copyFrom(that.seq, that.data, that.spp);
				return *this;
			}

			RecvData(const quint16 seq_, const Data& data_, const SPP& spp_) {
				copyFrom(seq_, data_, spp_);
			}
			RecvData(const Data& data_, const SPP& spp_) : RecvData(0, data_, spp_) {}

			void empty() {
				data.timeStamp = 0;
			}
			bool isEmpty() {
				return data.timeStamp == 0;
			}
		};

		// if recv returns true, data and spp are assigned
		// if recv returns false, data and spp are NOT assigned
		bool       recv(RecvData* recvData);
		void       send(const Data* data, const SPP* spp);
		void       close();
		bool       stopRun();
		Config*    getConfig();
		Context*   getContext();
		Listeners* getListeners();

		class FunctionTable {
		public:
			std::function<bool(RecvData*)>   recv;
			std::function<void(Data*, SPP*)> send;
			std::function<void(void)>        close;
			std::function<bool(void)>        stopRun;
			std::function<Config*(void)>     getConfig;
			std::function<Context*(void)>    getContext;
			std::function<Listeners*(void)>  getListeners;
		};
		FunctionTable functionTable;

		virtual void run(FunctionTable functionTable) = 0;

		State   myState;
		Server* myServer;

	private:
		class RecvBuffer {
			static const int SIZE = 4;
		public:
			RecvData array[SIZE];

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
			bool add(const RecvData& newValue) {
				for(int i = 0; i < SIZE; i++) {
					RecvData *p = array + i;

					if (p->isEmpty()) {
						*p = newValue;
						return true;
					}
				}
				return false;
			}
			RecvData* get(quint16 seq) {
				for(int i = 0; i < SIZE; i++) {
					RecvData *p = array + i;

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
			RecvData* getYougest() {
				RecvData* ret = nullptr;
				for(int i = 0; i < SIZE; i++) {
					RecvData *p = array + i;

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

		QAtomicInt      stopFuture;
		QAtomicInt      stopIsCalled;
		QAtomicInt      closeIsCalled;
		QFuture<void>   futureRun;
		QFuture<void>   futureSend;

		QList<RecvData> recvList;
		QMutex          recvListMutex;
		QWaitCondition  recvListCV;

		QList<RecvData> sendList;
		QMutex          sendListMutex;
		QWaitCondition  sendListCV;

		RecvBuffer recvBuffer;
		quint16 sendSeq;
		quint16 recvSeq;

		// for transmit
		Driver* driver;
		quint32 localNet;
		quint64 localHost;

	};


	class SPPQueueServer : public SPPListener {
	public:
		SPPQueueServer(SPPQueue* impl) : SPPListener(impl->name(), impl->socket()), myImpl(impl), myServer(nullptr) {}

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

}
