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
// CHS.h
//

#pragma once

#include "../xns/XNS.h"

#include "Service.h"
#include "Type.h"

namespace Courier::CHS {
	using ByteBuffer::Buffer;
	using ByteBuffer::Base;
	using ByteBuffer::UINT16;
	using XNS::Host;
	using XNS::Net;
	using XNS::Socket;

	//NetworkAddress: TYPE = RECORD [
	//	network: UNSPECIFIED2,
	//	host: UNSPECIFIED3,
	//	socket: UNSPECIFIED ];
	class NetworkAddress : public Base {
	public:
		Net    network;
		Host   host;
		Socket socket;

		QString toString();

		// ByteBuffer::Base
		void fromByteBuffer(Buffer& bb);
		void toByteBuffer  (Buffer& bb) const;
	};
	// NetworkAddressList: TYPE = SEQUENCE 40 OF NetworkAddress;


	//
	// ERROR
	//

	//CallProblem: TYPE = {
	//	accessRightsInsufficient(1), -- operation prevented by access controls --
	//	tooBusy(2), -- server is too busy to service this request --
	//	serverDown(3), -- a remote Clearinghouse server was down and was needed for this request --
	//	useCourier(4), -- server insists that Courier be used for this particular request --
	//	other(5) };
	//CallError: ERROR [problem: CallProblem] = 1;

	class CallProblem : public UINT16 {
	public:
		enum Value : quint16 {
			accessRightsInsufficient = 1,
			tooBusy                  = 2,
			serverDown               = 3,
			useCourier               = 4,
			other                    = 5,
		};

		// define operator =
		quint16 operator =(const quint16& newValue) const {
			value(newValue);
			return newValue;
		}

		QString toString() const {
			return nameMap.toString(value());
		}
	private:
		static NameMap::Map<quint16> nameMap;
	};

	class CallError : public Base {
		static const quint16 ABORT = 1;
	public:
		CallProblem problem;

		void toByteBuffer(Buffer& bb) const {
			bb.write16(ABORT);
			TO_BYTE_BUFFER(bb, problem);
		}
	};


	//
	// PROCEDURE
	//

	//RetrieveAddresses: PROCEDURE
	//RETURNS [address: NetworkAddressList]
	//REPORTS [errorEror: CallError] = 0;
	class RetrieveAddresses : public Procedure {
		const char*   NAME      = "RetrieveAddresses";
		const quint16 PROCEDURE = 0;
	public:
		const char* name() {
			return NAME;
		}
		quint16 procedure() {
			return PROCEDURE;
		}
		void call(const Data& data, const PEX& pex, const Protocol3Body::CallBody& callBody);

		class Parameter : Base {
		public:
			// ByteBuffer::Base
			void fromByteBuffer(Buffer& bb) { (void)bb; }
		};
		class Returns : Base {
		public:
			SEQUENCE<NetworkAddress, 40> value;

			// ByteBuffer::Base
			void toByteBuffer  (Buffer& bb) const {
				TO_BYTE_BUFFER(bb, value);
			}
		};
	};

	const quint32 PROGRAM  = 2;
	const quint16 VERSION2 = 2;
	const quint16 VERSION3 = 3;



}
