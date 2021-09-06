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
// Boot.h
//

#pragma once

#include <QtCore>

#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/NameMap.h"

#include "XNS.h"

#include <variant>

namespace XNS {
	using Courier::UINT16;
	using Courier::BLOCK;
	using Courier::Base;

	//
	// See APilot/15.0.1/Pilot/Friends/BootServerTypes.mesa
	//
	class Boot : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value {
				SIMPLE_REQUEST = 1, SIMPLE_DATA = 2, SPP_REQUEST = 3,
			};

			// define operator =
			quint16 operator =(const quint16& newValue) const {
				value(newValue);
				return newValue;
			}

			QString toString() const {
				return nameMap.toString(value());
			}
			static void addNameMap(quint16 value, QString name) {
				nameMap.add(value, name);
			}
		private:
			static NameMap::Map<quint16> nameMap;
		};

		class SimpleRequest : public Base {
		public:
			Host bootFileNumber; // XNS::HOST::BFN_GVWIN

			QString toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class SimpleData : public Base {
		public:
			Host   bootFileNumber; // XNS::HOST::BFN_GVWIN
			UINT16 packetNumber;   // start with 1
			BLOCK  block;          // one page of data normally or no data means end of file

			QString toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class SPPRequest : public Base {
		public:
			Host   bootFileNumber; // XNS::HOST::BFN_GVWIN
			UINT16 connectionID;

			QString toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};

		Type type;
		std::variant<std::monostate, class SimpleRequest, class SimpleData, class SPPRequest> body;

		void get(SimpleRequest& newValue) const;
		void get(SimpleData&    newValue) const;
		void get(SPPRequest&    newValue) const;

		void set(const SimpleRequest& newValue);
		void set(const SimpleData&    newValue);
		void set(const SPPRequest&    newValue);

		QString toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

}
