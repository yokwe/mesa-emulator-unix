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

#include <functional>


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


	class Data {
	public:
		quint64  time;     // creation time. seconds since unix time epoch, used to remove old queue entry
		Context& context;

		Packet   packet;   // received packet
		Ethernet ethernet;
		IDP      idp;

		Data(Context& context_, Packet& packet_, Ethernet ethernet_, IDP idp_) :
			time(QDateTime::currentSecsSinceEpoch()), context(context_), packet(packet_), ethernet(ethernet_), idp(idp_) {}
	};

	class DataHandler {
	public:
		class Base {
		public:
			virtual ~Base() {}
			virtual quint16     socket()            = 0;
			virtual const char* name()              = 0;
			virtual void        start()             = 0;
			virtual void        process(Data& data) = 0;
			virtual void        stop()              = 0;
		};

		std::function<quint16(void)>     socket;
		std::function<const char*(void)> name;
		std::function<void(void)>        start;
		std::function<void(Data&)>       process;
		std::function<void(void)>        stop;

		DataHandler() : socket(nullptr), name(nullptr), start(nullptr), process(nullptr), stop(nullptr) {}
		DataHandler(Base& base) :
			socket ([&base](){return base.socket();}),
			name   ([&base](){return base.name();}),
			start  ([&base](){base.start();}),
			process([&base](Data& data){base.process(data);}),
			stop   ([&base](){base.stop();}) {}
	};


	class ProcessThread : public QRunnable {
		Context&                    context;
		QMap<quint16, DataHandler>& handlerMap;
		bool                        stopThread;
		bool                        threadRunning;

	public:
		ProcessThread(Context& context_, QMap<quint16, DataHandler>& handlerMap_) :
			context(context_), handlerMap(handlerMap_), stopThread(false), threadRunning(false) {}

		bool running();

		void start();
		void run();  // for QRunnable
		void stop(); // stop thread
	};


	class Server {
		Context                    context;
		QMap<quint16, DataHandler> handlerMap;

		QThreadPool*               processThreadPool;    // thread pool for ProcessThread
		ProcessThread*             processThread;

	public:
		Server() : processThreadPool(new QThreadPool()), processThread(nullptr) {}

		void init(const QString& path);

		void add(DataHandler handler);

		bool running();
		void start();
		void stop();
	};

}
