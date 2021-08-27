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
#include "../xns/RIP.h"
#include "../xns/Echo.h"
#include "../xns/Error.h"
#include "../xns/PEX.h"
#include "../xns/SPP.h"
#include "../xns/Boot.h"
#include "../xns/Courier.h"
#include "../xns/Time.h"

#include <functional>


namespace XNS::Server {
	using ByteBuffer::Buffer;
	using Network::Device;
	using Network::Driver;
	using Network::Packet;
	using Courier::ExpeditedCourier;

	class Context {
	public:
		quint32 localNet;
		quint64 localAddress;

		Driver* driver;

		Context() : localNet(0), localAddress(0), driver(nullptr) {}
		Context(const Config& config);
	};


	class Data {
	public:
		// creation time in milliseconds since unix time epoch, used to remove old entry
		quint64  timeStamp;
		Config&  config;
		Context& context;

		// received data
		Packet   packet;
		Ethernet ethernet;
		IDP      idp;

		Data(quint64 timeStamp_, Config& config_, Context& context_, Packet& packet_, Ethernet ethernet_, IDP idp_) :
			timeStamp(timeStamp_), config(config_), context(context_), packet(packet_), ethernet(ethernet_), idp(idp_) {}
	};


	class Service {
	public:
		class Base {
		public:
			virtual ~Base() {}
			virtual quint16     socket()                  = 0;
			virtual const char* name  ()                  = 0;
			virtual void        init  (Config*, Context*) = 0;
			virtual void        start ()                  = 0;
			virtual void        stop  ()                  = 0;

			virtual void        handle(const Data& data) = 0;
		};

		std::function<quint16    (void)>              socket;
		std::function<const char*(void)>              name;
		std::function<void       (Config*, Context*)> init;
		std::function<void       (void)>              start;
		std::function<void       (void)>              stop;
		std::function<void       (const Data&)>       handle;

		Service() :
			socket(nullptr), name(nullptr), init(nullptr), start(nullptr), stop(nullptr), handle(nullptr) {}
		Service(Base& base) :
			socket ([&base]()                                {return base.socket();}),
			name   ([&base]()                                {return base.name();}),
			init   ([&base](Config* config, Context* context){base.init(config, context);}),
			start  ([&base]()                                {base.start();}),
			stop   ([&base]()                                {base.stop();}),
			handle ([&base](const Data& data)                {base.handle(data);}) {}
	};


	class ProcessThread : public QRunnable {
		Config&                 config;
		Context&                context;
		QMap<quint16, Service>& serviceMap;
		bool                    stopThread;
		bool                    threadRunning;

	public:
		ProcessThread(Config& config_, Context& context_, QMap<quint16, Service>& serviceMap_) :
			config(config_), context(context_), serviceMap(serviceMap_), stopThread(false), threadRunning(false) {}

		bool running();

		// initialize and start thread
		void start();
		// thread main
		void run();
		// stop thread and finalize
		void stop();
	};


	class Server {
		Config                 config;
		Context                context;
		QMap<quint16, Service> serviceMap;

		QThreadPool*           processThreadPool;    // thread pool for ProcessThread
		ProcessThread*         processThread;

	public:
		Server() : processThreadPool(new QThreadPool()), processThread(nullptr) {}

		// add service
		void add(Service service);

		// after adding service finished, initialize server
		void init(const QString& path);

		bool running();
		void start();
		void stop();
	};


	namespace Services {
		class Default : public Service::Base {
		public:
			// initialize idp for transmit
			static void init(const Data& data, quint8 type, BLOCK& block, IDP& idp);

			Default() : config(nullptr), context(nullptr) {}
			virtual ~Default() {}

			void init(Config* config_, Context* context_) {
				config  = config_;
				context = context_;
			}

			void start() {}
			void stop () {}

		protected:
			Config*  config;
			Context* context;

			// for RIP broadcast
			void transmit(const Context& context, quint64 dst, const IDP& idp);

			// transmit idp packet
			void transmit(const Data& data, const IDP& idp) {
				transmit(data.context, data.ethernet.src, idp);
			}

			void transmit(const Data& data, const RIP&   rip);
			void transmit(const Data& data, const Echo&  echo);
			void transmit(const Data& data, const Error& error);
			void transmit(const Data& data, const PEX&   pex);
			void transmit(const Data& data, const SPP&   spp);
			void transmit(const Data& data, const Boot&  boot);
		};


		class RIPService : public Default {
			typedef std::function<void(const Data&, const RIP&)>   ReceiveRIP;
			typedef std::function<void(const Data&, const Error&)> ReceiveError;

			ReceiveRIP   receiveRIP;
			ReceiveError receiveError;

		public:
			RIPService() :
				receiveRIP  ([this](const Data& data, const RIP& rip)    {this->receive(data, rip);}),
				receiveError([this](const Data& data, const Error& error){this->receive(data, error);}) {}
			virtual ~RIPService() {}

			quint16 socket(){
				return XNS::IDP::Socket::RIP;
			}
			void handle(const Data& data);

		protected:
			// receive packet
			virtual void receive(const Data& data, const RIP&   rip)   = 0;
			virtual void receive(const Data& data, const Error& error) = 0;
		};

		class EchoService : public Default {
			typedef std::function<void(const Data&, const Echo&)>  ReceiveEcho;
			typedef std::function<void(const Data&, const Error&)> ReceiveError;

			ReceiveEcho  receiveEcho;
			ReceiveError receiveError;

		public:
			EchoService() :
				receiveEcho ([this](const Data& data, const Echo& echo)  {this->receive(data, echo);}),
				receiveError([this](const Data& data, const Error& error){this->receive(data, error);}) {}
			virtual ~EchoService() {}

			quint16 socket(){
				return XNS::IDP::Socket::ECHO;
			}
			void handle(const Data& data);

		protected:
			// receive packet
			virtual void receive(const Data& data, const Echo&  echo)  = 0;
			virtual void receive(const Data& data, const Error& error) = 0;
		};

		class CHService : public Default {
			typedef std::function<void(const Data&, const PEX&, const ExpeditedCourier& exp)> ReceiveExp;
			typedef std::function<void(const Data&, const Error&)>                            ReceiveError;

			ReceiveExp   receiveExp;
			ReceiveError receiveError;

		public:
			CHService() :
				receiveExp  ([this](const Data& data, const PEX& pex, const ExpeditedCourier& exp){this->receive(data, pex, exp);}),
				receiveError([this](const Data& data, const Error& error)                         {this->receive(data, error);   }) {}
			virtual ~CHService() {}

			quint16 socket(){
				return XNS::IDP::Socket::CHS;
			}
			void handle(const Data& data);

		protected:
			// receive packet
			virtual void receive(const Data& data, const PEX&   pex, const ExpeditedCourier& exp) = 0;
			virtual void receive(const Data& data, const Error& error)                            = 0;
		};

		class TimeService : public Default {
			typedef std::function<void(const Data&, const PEX&, const Time& time)> ReceiveTime;
			typedef std::function<void(const Data&, const Error&)>                 ReceiveError;

			ReceiveTime  receiveTime;
			ReceiveError receiveError;

		public:
			TimeService() :
				receiveTime ([this](const Data& data, const PEX& pex, const Time& time){this->receive(data, pex, time);}),
				receiveError([this](const Data& data, const Error& error)              {this->receive(data, error);    }) {}
			virtual ~TimeService() {}

			quint16 socket(){
				return XNS::IDP::Socket::TIME;
			}
			void handle(const Data& data);

		protected:
			// receive packet
			virtual void receive(const Data& data, const PEX&   pex, const Time& time) = 0;
			virtual void receive(const Data& data, const Error& error)                 = 0;
		};
	}
}
