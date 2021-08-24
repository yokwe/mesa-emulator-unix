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


	class Service {
	public:
		class Base {
		public:
			virtual ~Base() {}
			virtual quint16     socket()           = 0;
			virtual const char* name()             = 0;
			virtual void        start()            = 0;
			virtual void        stop()             = 0;

			virtual void        handle(Data& data) = 0;
		};

		std::function<quint16(void)>     socket;
		std::function<const char*(void)> name;
		std::function<void(void)>        start;
		std::function<void(void)>        stop;
		std::function<void(Data&)>       handle;

		Service() :
			socket(nullptr), name(nullptr), start(nullptr), stop(nullptr), handle(nullptr) {}
		Service(Base& base) :
			socket ([&base](){return base.socket();}),
			name   ([&base](){return base.name();}),
			start  ([&base](){base.start();}),
			stop   ([&base](){base.stop();}),
			handle ([&base](Data& data){base.handle(data);}) {}
	};


	class ProcessThread : public QRunnable {
		Context&                context;
		QMap<quint16, Service>& serviceMap;
		bool                    stopThread;
		bool                    threadRunning;

	public:
		ProcessThread(Context& context_, QMap<quint16, Service>& serviceMap_) :
			context(context_), serviceMap(serviceMap_), stopThread(false), threadRunning(false) {}

		bool running();

		void start();
		void run();  // for QRunnable
		void stop(); // stop thread
	};


	class Server {
		Context                context;
		QMap<quint16, Service> serviceMap;

		QThreadPool*           processThreadPool;    // thread pool for ProcessThread
		ProcessThread*         processThread;

	public:
		Server() : processThreadPool(new QThreadPool()), processThread(nullptr) {}

		void init(const QString& path);

		void add(Service handler);

		bool running();
		void start();
		void stop();
	};


	namespace Services {
		using Courier::ExpeditedCourier;

		class Default : public Service::Base {
		public:
			// initialize idp for transmit
			static void init(const Data& data, quint8 type, BLOCK& block, IDP& idp);

			virtual ~Default() {}

			void start() {}
			void stop () {}

		protected:
			// transmit idp packet
			void transmit(Data& data, IDP& idp);

			// transmit error packet
			void transmit(Data& data, Error& error);
		};


		class RIPService : public Default {
			typedef std::function<void(Data&, RIP&)>   ReceiveRIP;
			typedef std::function<void(Data&, Error&)> ReceiveError;

			ReceiveRIP   receiveRIP;
			ReceiveError receiveError;

		public:
			RIPService() :
				receiveRIP  ([this](Data& data, RIP& rip)    {this->receive(data, rip);}),
				receiveError([this](Data& data, Error& error){this->receive(data, error);}) {}
			virtual ~RIPService() {}

			quint16 socket(){
				return XNS::IDP::Socket::RIP;
			}
			void handle(Data& data);

		protected:
			// receive packet
			virtual void receive(Data& data, RIP&   rip)   = 0;
			virtual void receive(Data& data, Error& error) = 0;

			// transmit packet
			void transmit(Data& data, RIP& rip);
		};

		class EchoService : public Default {
			typedef std::function<void(Data&, Echo&)>  ReceiveEcho;
			typedef std::function<void(Data&, Error&)> ReceiveError;

			ReceiveEcho  receiveEcho;
			ReceiveError receiveError;

		public:
			EchoService() :
				receiveEcho ([this](Data& data, Echo& echo)  {this->receive(data, echo);}),
				receiveError([this](Data& data, Error& error){this->receive(data, error);}) {}
			virtual ~EchoService() {}

			quint16 socket(){
				return XNS::IDP::Socket::ECHO;
			}
			void handle(Data& data);

		protected:
			// receive packet
			virtual void receive(Data& data, Echo&  echo)  = 0;
			virtual void receive(Data& data, Error& error) = 0;

			// transmit packet
			void transmit(Data& data, Echo& echo);
		};

		class CHSService : public Default {
			typedef std::function<void(Data&, PEX&, ExpeditedCourier& exp)> ReceiveExp;
			typedef std::function<void(Data&, Error&)>                      ReceiveError;

			ReceiveExp   receiveExp;
			ReceiveError receiveError;

		public:
			CHSService() :
				receiveExp  ([this](Data& data, PEX& pex, ExpeditedCourier& exp){this->receive(data, pex, exp);}),
				receiveError([this](Data& data, Error& error)                   {this->receive(data, error);   }) {}
			virtual ~CHSService() {}

			quint16 socket(){
				return XNS::IDP::Socket::CHS;
			}
			void handle(Data& data);

		protected:
			// receive packet
			virtual void receive(Data& data, PEX&   pex, ExpeditedCourier& exp)   = 0;
			virtual void receive(Data& data, Error& error)                        = 0;

			// transmit packet
			void transmit(Data& data, PEX& pex, ExpeditedCourier& exp);
		};

		class TimeService : public Default {
			typedef std::function<void(Data&, PEX&, Time& time)> ReceiveTime;
			typedef std::function<void(Data&, Error&)>           ReceiveError;

			ReceiveTime  receiveTime;
			ReceiveError receiveError;

		public:
			TimeService() :
				receiveTime ([this](Data& data, PEX& pex, Time& time){this->receive(data, pex, time);}),
				receiveError([this](Data& data, Error& error)        {this->receive(data, error);    }) {}
			virtual ~TimeService() {}

			quint16 socket(){
				return XNS::IDP::Socket::TIME;
			}
			void handle(Data& data);

		protected:
			// receive packet
			virtual void receive(Data& data, PEX&   pex, Time& time)   = 0;
			virtual void receive(Data& data, Error& error)             = 0;

			// transmit packet
			void transmit(Data& data, PEX& pex, Time& time);
		};
	}
}
