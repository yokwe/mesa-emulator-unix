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
// Service.h
//

#pragma once

#include "../util/Util.h"


#include "../util/ByteBuffer.h"

#include "../xns/XNS.h"
#include "../xns/PEX.h"

#include "../courier/Protocol.h"

// forward declaration of XNS::Server::Server
namespace XNS::Server {
	class Server;
}

namespace Courier {
	using XNS::Config;
	using XNS::Context;
	using XNS::PEX;
	using XNS::Data;
	using XNS::Server::Server;

	// forward declaration
	class Service;

	class Procedure {
	public:
		virtual ~Procedure() {}
		const char* name() const {
			return myName;
		}
		uint16_t     procedure() const {
			return myProcedure;
		}
		bool        useBulk() const {
			return myUseBulk;
		}

		Procedure() : myName(nullptr), myProcedure(0), myUseBulk(false) {}
		Procedure(const Procedure& that) {
			this->myName      = that.myName;
			this->myProcedure = that.myProcedure;
			this->myUseBulk   = that.myUseBulk;
		}
		Procedure& operator = (const Procedure& that) {
			this->myName      = that.myName;
			this->myProcedure = that.myProcedure;
			this->myUseBulk   = that.myUseBulk;
			return *this;
		}

		Procedure(const char* name_, uint16_t procedure_, bool useBulk_) : myName(name_), myProcedure(procedure_), myUseBulk(useBulk_) {}

		std::string toString() const;

		// service is to access session
		// result contains serialized Protocol3Body
		// result can be empty for error during process of call
		virtual void call(const Config& config, Service& service, const Protocol3Body::CallBody& callBody, ByteBuffer& result) = 0;

	protected:
		const char* myName;
		uint16_t     myProcedure;
		bool        myUseBulk;
	};


	class Session {
		static const uint64_t VALID_SESSION_PERIOD = 10 * 60; // 10 minutes

	public:
		Session(uint16_t transaction_) {
			myTimestamp = QDateTime::currentSecsSinceEpoch();
			myTransaction = transaction_;
		}
		Session() : myTimestamp(0), myTransaction(0) {}
		Session(const Session& that) {
			this->myTimestamp   = that.myTimestamp;
			this->myTransaction = that.myTransaction;
		}
		Session& operator = (const Session& that) {
			this->myTimestamp   = that.myTimestamp;
			this->myTransaction = that.myTransaction;
			return *this;
		}
		std::string toString();

		uint16_t transaction() {
			return myTransaction;
		}
		void update() {
			myTimestamp = QDateTime::currentSecsSinceEpoch();
		}
		bool isTimeout() {
			uint64_t now = QDateTime::currentSecsSinceEpoch();
			return (myTimestamp + VALID_SESSION_PERIOD) < now;
		}

	protected:
		uint64_t myTimestamp;   // myTimestamp is for session timeout management
		uint16_t myTransaction;
	};


	class Service {
	public:
		virtual ~Service() {}
		const char* name() const {
			return myName;
		}
		uint32_t     program() const {
			return myProgram;
		}
		uint16_t     version() const {
			return myVersion;
		}

		// life cycle management
		virtual void init  () = 0;
		virtual void start () = 0;
		virtual void stop  () = 0;

		void defaultStart() {
			sessionMap.clear();
		}

		Service() : myName(nullptr), myProgram(0), myVersion(0) {}
		Service(const Service& that) {
			this->myName       = that.myName;
			this->myProgram    = that.myProgram;
			this->myVersion    = that.myVersion;
			this->procedureMap = that.procedureMap;
			this->sessionMap   = that.sessionMap;
		}
		Service& operator = (const Service& that) {
			this->myName       = that.myName;
			this->myProgram    = that.myProgram;
			this->myVersion    = that.myVersion;
			this->procedureMap = that.procedureMap;
			this->sessionMap   = that.sessionMap;
			return *this;
		}

		Service(const char* name_, uint32_t program_, uint16_t version_) : myName(name_), myProgram(program_), myVersion(version_) {}

		std::string toString() const;

		// procedure
		void addProcedure(Procedure* procedure);
		Procedure* getProcedure(const uint16_t procedure) const;

		// session
		void addSesion(Session* session);
		void removeSession(uint16_t transaction);
		// If three is no session for transaction, returns nullptr
		Session* getSession(uint16_t transaction);

	protected:
		const char* myName;
		uint32_t     myProgram;
		uint16_t     myVersion;

		std::map<uint16_t, Procedure*> procedureMap;
		//   procedure
		std::map<uint16_t, Session*>   sessionMap;
		//   transaction
	};


	class ProgramVersion {
	public:
		uint32_t program;
		uint16_t version;

		ProgramVersion(Service& service) : program(service.program()), version(service.version()) {}
		ProgramVersion(uint32_t program_, uint16_t version_) : program(program_), version(version_) {}

		ProgramVersion() : program(0), version(0) {}
		ProgramVersion(const ProgramVersion& that) : program(that.program), version(that.version) {}
		ProgramVersion& operator = (const ProgramVersion& that) {
			this->program = that.program;
			this->version = that.version;
			return *this;
		}

		std::string toString() const {
			return std::string("%1-%2").arg(program).arg(version);
		}

		bool operator == (const ProgramVersion& that) const {
			return this->program == that.program && this->version == that.version;
		}
		bool operator < (const ProgramVersion& that) const {
			if (this->program == that.program) {
				return this->version < that.version;
			} else {
				return this->program < that.program;
			}
		}
	};


	class Services {
	public:
		Services() : server(nullptr), started(false) {}

		// life cycle management
		void init(Server* server_) {
			server = server_;
		}
		void start();
		void stop();

		// add service
		void add(Service* service);
		// get service
		//   if there is no service for programVersion, returns nullptr
		Service* getService(const ProgramVersion& programVersion) const;

		// call service specified in body and set outcome in result
		// result contains serialized Protocol3Body
		// result can be empty for error during process of call
		void call(const Protocol3Body& body, ByteBuffer& result, bool& useBulk) const;
	protected:
		Server* server;
		bool    started;

		std::map<ProgramVersion, Service*> map;
	};
}

