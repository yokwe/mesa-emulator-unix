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
// Server.h
//

#pragma once

#include <QtCore>

#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/BPF.h"
#include "../util/Network.h"

#include "../xns/XNS.h"
#include "../xns/Error.h"


namespace XNS::Server {
	using ByteBuffer::Buffer;
	using Network::Device;
	using Network::Driver;
	using Network::Packet;

	class Context {
	public:
		Device  device;
		quint32 localNet;
		Driver* driver;

		Context() : localNet(0), driver(nullptr) {}
		Context(const QString& path); // path of config json file
	};

	class SocketListener : public QRunnable {
	public:
		class Entry {
		public:
			quint64  time;     // creation time. seconds since unix time epoch, used to remove old queue entry
			Context& context;

			Packet   packet;   // received packet
			Ethernet ethernet;
			IDP      idp;

			Entry(Context& context_, Packet& packet_, Ethernet ethernet_, IDP idp_) :
				time(QDateTime::currentSecsSinceEpoch()), context(context_), packet(packet_), ethernet(ethernet_), idp(idp_) {}
		};

		SocketListener(quint16 socket) : listenerSocket(socket), stopThread(false), threadRunning(false) {}
		virtual ~SocketListener() {}

		void add(const Entry& entry); // operation is protected by mutex. Also do list maintenance.

		bool running();

		virtual void run() = 0;  // for QRunnable
		void stop(); // stop thread

		quint16 socket() {
			return listenerSocket;
		}
	protected:
		quint16        listenerSocket;
		bool           stopThread;
		bool           threadRunning;
		QList<Entry>   list;
		QMutex         listMutex;
		QWaitCondition listCV;

		bool  isEmpty(); // operation is protected by mutex. Also do list maintenance.
		Entry get();     // operation is protected by mutex. Also do list maintenance. Entry can be null
		void  clear();   // operaiton is protected by mutex.

		void maintainList(); // do list maintenance
	};

	// create thread for socket listener and queue received packet to socket listener queue
	class ProcessThread : public QRunnable {
		Context&                        context;
		QMap<quint16, SocketListener*>& listenerMap;
		QThreadPool*                    threadPool; // thread pool for socket listener
		bool                            stopThread;
		bool                            threadRunning;

	public:
		ProcessThread(Context& context_, QMap<quint16, SocketListener*>& listenerMap_, QThreadPool* threadPool_) :
			context(context_), listenerMap(listenerMap_), threadPool(threadPool_), stopThread(false), threadRunning(false) {}

		bool running();
		void run();  // for QRunnable
		void stop(); // stop thread
	};


	class Server {
		Context                        context;
		QMap<quint16, SocketListener*> listenerMap;
		QThreadPool*                   processThreadPool;    // thread pool for ProcessThread
		QThreadPool*                   listenerThreadPool;    // thread pool for ProcessThread
		ProcessThread*                 processThread;

	public:
		Server() : processThreadPool(new QThreadPool()), listenerThreadPool(new QThreadPool()), processThread(nullptr) {}

		void init(const QString& path);

		void add(SocketListener* listener);

		bool running();
		void start();
		void stop();
		void wait();
	};

}
