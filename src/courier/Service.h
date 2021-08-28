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
// Service.h
//

#pragma once

#include "../util/Util.h"


#include "../util/ByteBuffer.h"

#include "../xns/XNS.h"
#include "../xns/PEX.h"

#include "Courier.h"


namespace Courier {
	using ByteBuffer::Base;
	using ByteBuffer::Buffer;
	using XNS::PEX;
	using XNS::Data;
	using Courier::ExpeditedCourier;
	using Courier::Protocol3Body;

	class Procedure {
	public:
		virtual ~Procedure() {}

		virtual const char* name()      = 0;
		virtual quint16     procedure() = 0;;

		virtual void call(const Data& data, const PEX& pex, const Protocol3Body& body) = 0;
	};

	class Service {
	public:
		virtual ~Service() {}

		Service() {}
		Service(const Service& that) : map(that.map) {}
		Service& operator = (const Service& that) {
			this->map = that.map;
			return *this;
		}

		virtual const char* name()    = 0;
		virtual quint32     program() = 0;
		virtual quint16     version() = 0;

		// initialize this service. called once before call
		virtual void init() = 0;

		void add(Procedure* procedure);
		Procedure* getProcedure(quint16 procedure);

		void call(const Data& data, const PEX& pex, const Protocol3Body& body);

	protected:
		QMap<quint16, Procedure*> map;
		//   procedure
	};

	class Services {
	public:
		class ProgramVersion {
		public:
			quint32 program;
			quint16 version;

			ProgramVersion(quint32 program_, quint16 version_) : program(program_), version(version_) {}

			ProgramVersion() : program(0), version(0) {}
			ProgramVersion(const ProgramVersion& that) : program(that.program), version(that.version) {}
			ProgramVersion& operator = (const ProgramVersion& that) {
				this->program = that.program;
				this->version = that.version;
				return *this;
			}

			QString toString() {
				return QString("%1-%2").arg(program).arg(version);
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

		void add(Service* service);
		Service* getService(quint32 program, quint16 version);

		void call(const Data& data, const PEX& pex, const Protocol3Body& body);
	protected:
		QMap<ProgramVersion, Service*> map;
	};
}

