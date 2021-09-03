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
// BulkData.h
//

#pragma once

#include "../xns/XNS.h"

#include "Service.h"
#include "Type.h"

#include <variant>


namespace Courier::BulkData {
	using XNS::Net;
	using XNS::Host;
	using XNS::Socket;

	//	-- streams (for external use) --
	//
	//	StreamOfUnspecified: TYPE = CHOICE OF {
	//		nextSegment(0) => RECORD [
	//			segment: SEQUENCE OF UNSPECIFIED,
	//			restOfStream: StreamOfUnspecified],
	//		lastSegment(1) => RECORD [
	//			segment: SEQUENCE OF UNSPECIFIED]};
	//
	//	-- types --
	//
	//	Identifier: TYPE = RECORD [
	//		host: UNSPECIFIED3,
	//		hostRelativeIdentifier: UNSPECIFIED2 ];
	class Identifier : public Base {
	public:
		Host   host;
		UINT32 hostRelativeIdentifier;

		Identifier() {}
		Identifier(const Identifier& that) {
			this->host                   = that.host;
			this->hostRelativeIdentifier = that.hostRelativeIdentifier;
		}
		Identifier& operator = (const Identifier& that) {
			this->host                   = that.host;
			this->hostRelativeIdentifier = that.hostRelativeIdentifier;
			return *this;
		}

		QString toString();

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};


	//	Descriptor: TYPE = CHOICE OF {
	//		null(0), immediate(1) => RECORD [ ],
	//		passive(2), active(3) => RECORD [
	//			network: UNSPECIFIED2,
	//			host: UNSPECIFIED3,
	//			identifier: Identifier ]
	//		};
	class Descriptor : public Base {
	public:
		class Choice : public UINT16 {
		public:
			enum Value : quint16 {
				NULL_     = 0,
				IMMEDIATE = 1,
				PASSIVE   = 2,
				ACTIVE    = 3,
			};

			QString toString() const {
				return nameMap.toString(value());
			}
			private:
				static NameMap::Map<quint16> nameMap;
		};
		class NetworkIdentifier : public Base {
		public:
			Net        net;
			Host       host;
			Identifier identifier;

			NetworkIdentifier() {}
			NetworkIdentifier(const NetworkIdentifier& that) {
				this->net        = that.net;
				this->host       = that.host;
				this->identifier = that.identifier;
			}
			NetworkIdentifier& operator = (const NetworkIdentifier& that) {
				this->net        = that.net;
				this->host       = that.host;
				this->identifier = that.identifier;
				return *this;
			}

			QString toString() {
				return QString("%1-%2-%3").arg(net.toString()).arg(host.toString()).arg(identifier.toString());
			}

			// ByteBuffer::Base
			void fromByteBuffer(Buffer& bb) {
				FROM_BYTE_BUFFER(bb, net);
				FROM_BYTE_BUFFER(bb, host);
				FROM_BYTE_BUFFER(bb, identifier);
			}
			void toByteBuffer  (Buffer& bb) const {
				TO_BYTE_BUFFER(bb, net);
				TO_BYTE_BUFFER(bb, host);
				TO_BYTE_BUFFER(bb, identifier);
			}
		};

		Choice choice;
		std::variant<std::monostate, NetworkIdentifier> body;

		void get(      NetworkIdentifier&  newValue)  const;
		void set(const NetworkIdentifier&  newValue);

		QString toString() const;

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};

	//	-- sinks (for external use) --
	//
	//	Sink: TYPE = Descriptor;
	typedef Descriptor Sink;

	//	immediateSink: Sink = immediate[];
	//	nullSink: Sink = null[];
	//
	//	-- sources (for external use) --
	//
	//	Source: TYPE = Descriptor;
	//	immediateSource: Source = immediate[];
	//	nullSource: Source = null[];
	//
	//	-- errors --
	//
	//	InvalidDescriptor: ERROR = 0; -- source or sink is passive, active, or null
	//	NoSuchIdentifier: ERROR = 1; -- identifier is unrecognized
	//	IdentifierBusy: ERROR = 2; -- BD object is being sent, received, or cancelled
	//	WrongHost: ERROR = 3; -- caller's host is unauthorized to transfer the data
	//	WrongDirection: ERROR = 4; -- BDT in the other direction was expected
	//	TransferAborted: ERROR = 5; -- BDT was aborted by sender or receiver
	//
	//
	//	-- procedures
	//
	//	Send: PROCEDURE [identifier: Identifier, sink: Sink, timeout: CARDINAL]
	//	REPORTS [ InvalidDescriptor, NoSuchIdentifier, IdentifierBusy, WrongHost,
	//		WrongDirection, TransferAborted ] = 0;
	//		-- transfers bulk data from callee to caller
	//
	//	Receive: PROCEDURE [identifier: Identifier, source: Source, timeout: CARDINAL]
	//	REPORTS [ InvalidDescriptor, NoSuchIdentifier, IdentifierBusy, WrongHost,
	//		WrongDirection, TransferAborted ] = 1;
	//		-- transfers bulk data from caller to callee
	//
	//	Cancel: PROCEDURE [ identifier: Identifier, timeout: CARDINAL ]
	//	REPORTS [ NoSuchIdentifier, IdentifierBusy, WrongHost ] = 2;
	//		-- cancels a bulk data transfer before it begins

}
