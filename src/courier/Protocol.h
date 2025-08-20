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
// Courier.h
//

#pragma once

#include "../util/NameMap.h"

#include <variant>

#include "Type.h"

namespace Courier {
	class ProtocolType : public UINT16 {
	public:
		enum Value : uint16_t {
			PROTOCOL2 = 2, PROTOCOL3 = 3,
		};

		// define operator =
		uint16_t operator =(const uint16_t& newValue) const {
			value(newValue);
			return newValue;
		}

		std::string toString() const {
			return nameMap.toString(value());
		}
		static void addNameMap(uint16_t value, std::string name) {
			nameMap.add(value, name);
		}
	private:
		static NameMap::Map<uint16_t> nameMap;
	};

	class MessageType : public UINT16 {
	public:
		enum Value : uint16_t {
			CALL = 0, REJECT = 1, RETURN = 2, ABORT = 3,s
		};

		// define operator =
		uint16_t operator =(const uint16_t& newValue) const {
			value(newValue);
			return newValue;
		}

		std::string toString() const {
			return nameMap.toString(value());
		}
		static void addNameMap(uint16_t value, std::string name) {
			nameMap.add(value, name);
		}
	private:
		static NameMap::Map<uint16_t> nameMap;
	};

	class RejectCode : public UINT16 {
	public:
		enum Value : uint16_t {
			NO_SUCH_PROGRAM   = 0,
			NO_SUCH_VERSION   = 1,
			NO_SUCH_PROCEDURE = 2,
			INVALID_ARGUMENTS = 3,
		};

		// define operator =
		uint16_t operator =(const uint16_t& newValue) const {
			value(newValue);
			return newValue;
		}

		std::string toString() const {
			return nameMap.toString(value());
		}
		static void addNameMap(uint16_t value, std::string name) {
			nameMap.add(value, name);
		}
	private:
		static NameMap::Map<uint16_t> nameMap;
	};

	// ProtocolRange: TYPE = RECORD [low, high: ProtocolType];
	class ProtocolRange : public Base {
	public:
		ProtocolType low;
		ProtocolType high;

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	// VersionRange:  TYPE = RECORD [low, high: CARDINAL];
	class VersionRange : public Base {
	public:
		UINT16 low;
		UINT16 high;

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	// Protocol2Body: TYPE = CHOICE MessageType OF {
	//   call   => RECORD [transaction, program, version, procedure: CARDINAL],
	//   reject => RECORD [transaction: CARDINAL, reject: RejectCode],
	//   return => RECORD [transaction: CARDINAL],
	//   abort  => RECORD [transaction: CARDINAL, abort: CARDINAL]};
	class Protocol2Body : public Base {
	public:
		class CallBody : public Base {
		public:
			UINT16     transaction;
			UINT16     program;
			UINT16     version;
			UINT16     procedure;
			BLOCK      block;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class RejectBody : public Base {
		public:
			UINT16     transaction;
			RejectCode reject;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class ReturnBody : public Base {
		public:
			UINT16     transaction;
			BLOCK      block;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class AbortBody : public Base {
		public:
			UINT16     transaction;
			UINT16     abort;
			BLOCK      block;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};

		MessageType type;
		std::variant<std::monostate, class CallBody, class RejectBody, class ReturnBody, class AbortBody> body;

		void get(CallBody&   newValue) const;
		void get(RejectBody& newValue) const;
		void get(ReturnBody& newValue) const;
		void get(AbortBody&  newValue) const;

		void set(const CallBody&   newValue);
		void set(const RejectBody& newValue);
		void set(const ReturnBody& newValue);
		void set(const AbortBody&  newValue);

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	// Protocol3Body: TYPE = CHOICE type: MessageType OF {
	//   call   => RECORD [transaction: CARDINAL,
	//		               program:     LONG CARDINAL,
	//		               version:     CARDINAL,
	//		               procedure:   CARDINAL],
	//   reject => RECORD [transaction: CARDINAL,
	//                     CHOICE code: RejectCode OF {
	//                              noSuchProgramNumber, noSuchProcedureValue, invalidArguments => RECORD [],
	//								noSuchVersionNumber => RECORD [range: VersionRange]}],
	//   return => RECORD [transaction: CARDINAL],
	//   abort  => RECORD [transaction: CARDINAL, abort: CARDINAL]};
	class Protocol3Body : public Base {
	public:
		class CallBody : public Base {
		public:
			UINT16     transaction;
			UINT32     program;
			UINT16     version;
			UINT16     procedure;
			BLOCK      block;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class RejectBody : public Base {
		public:
			UINT16     transaction;
			RejectCode code;
			std::variant<std::monostate, VersionRange> body;

			void get(VersionRange&  newValue)  const;
			void set(const VersionRange&   newValue);

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class ReturnBody : public Base {
		public:
			UINT16     transaction;
			BLOCK      block;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};
		class AbortBody : public Base {
		public:
			UINT16     transaction;
			UINT16     abort;
			BLOCK      block;

			std::string toString() const;

			// Courier::Base
			void fromByteBuffer(ByteBuffer& bb);
			void toByteBuffer  (ByteBuffer& bb) const;
		};

		MessageType type;
		std::variant<std::monostate, class CallBody, class RejectBody, class ReturnBody, class AbortBody> body;

		void get(CallBody&   newValue) const;
		void get(RejectBody& newValue) const;
		void get(ReturnBody& newValue) const;
		void get(AbortBody&  newValue) const;

		void set(const CallBody&   newValue);
		void set(const RejectBody& newValue);
		void set(const ReturnBody& newValue);
		void set(const AbortBody&  newValue);

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

//		MessageObject: TYPE = CHOICE ProtocolType OF {
//		    protocol0 => RECORD[],
//		    protocol1 => RECORD[],
//		    protocol2 => RECORD[ protocol2Body: Protocol2Body],
//		    protocol3 => RECORD[ protocol3Body: Protocol3Body]
//		};
	class MessageObject : public Base {
	public:
		ProtocolType type;
		std::variant<std::monostate, class Protocol2Body, class Protocol3Body> body;

		void get(Protocol2Body& newValue) const;
		void get(Protocol3Body& newValue) const;

		void set(const Protocol2Body& newValue);
		void set(const Protocol3Body& newValue);

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

//		Header: TYPE = MACHINE DEPENDENT RECORD [
//			protRange: CourierProtocol.ProtocolRange = [protocol3, protocol3],
//			body: CourierProtocol.Protocol3Body];
	class ExpeditedCourier : public Base {
	public:
		ProtocolRange range;
		Protocol3Body body;

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

}
