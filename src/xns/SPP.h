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
// SPP.h
//

#pragma once

#include <QtCore>

#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/NameMap.h"

#include "XNS.h"

namespace XNS {
	using Courier::UINT16;
	using Courier::BLOCK;
	using Courier::Base;


	class SPP : public Base {
	public:
		// SST - Sub System Type
		class SST : public UINT8 {
		public:
			enum Value {
				// From Courier/Friends/CourierProtocol.mesa
				DATA        = 0,   // Courier
				// From Courier/Private/BulkData.mesa
				BULK        = 1,   // Bulk Data
				// From NS/Public/NetworkStream.mesa
				CLOSE       = 254, // Closing connection
				CLOSE_REPLY = 255, // Reply of CLOSE (handshake)
			};

			// define operator =
			quint8 operator =(const quint8& newValue) const {
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
		class Control : public UINT8 {
		public:
			static const quint8 BIT_SYSTEM         = 0x80; // System Packet
			static const quint8 BIT_SEND_ACK       = 0x40; // Send Acknowledgment
			static const quint8 BIT_ATTENTION      = 0x20; // Attention
			static const quint8 BIT_END_OF_MESSAGE = 0x10; // End of Message
			static const quint8 BIT_UNUSED         = 0x0F;

			// define operator =
			quint8 operator =(const quint8& newValue) const {
				value(newValue);
				return newValue;
			}

			bool isSystem() const {
				return value() & BIT_SYSTEM;
			}
			bool isSendAck() const {
				return value() & BIT_SEND_ACK;
			}
			bool isAttention() const {
				return value() & BIT_ATTENTION;
			}
			bool isEndOfMessage() const {
				return value() & BIT_END_OF_MESSAGE;
			}

			bool isData() const {
				return !isSystem();
			}

			QString toString() const;
		};

		Control control; // Control Bit
		SST     sst;     // Sub System Type
		UINT16  idSrc;   // connection id of source
		UINT16  idDst;   // connection id of destination
		UINT16  seq;     // sequence
		UINT16  ack;     // acknowledgment
		UINT16  alloc;   // allocation
		BLOCK   block;

		void updateBlock(const BLOCK& that) {
			block.updateBufferData(that);
		}

		// Courier::Base
		QString toString() const;
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

}
