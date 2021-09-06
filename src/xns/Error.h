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
// Error.h
//

#pragma once

#include <QtCore>

#include "../util/Network.h"
#include "../util/ByteBuffer.h"
#include "../util/NameMap.h"

#include "XNS.h"

namespace XNS {
	using Courier::UINT48;
	using Courier::UINT32;
	using Courier::UINT16;
	using Courier::UINT8;
	using Courier::BLOCK;
	using Courier::Base;


	class Error : public Base {
	public:
		class Type : public UINT16 {
		public:
			enum Value {
				UNSPEC               = 0, // An unspecified error is detected at destination
				BAD_CHECKSUM         = 1, // The checksum is incorrect or the packet has some other serious inconsistency detected at destination
				NO_SOCKET            = 2, // The specified socket does not exist at the specified destination host
				RESOURCE_LIMIT       = 3, // The destination cannot accept the packet due to resource limitations
				LISTEN_REJECT        = 4,
				INVALID_PACKET_TYPE  = 5,
				PROTOCOL_VIOLATION   = 6,

				UNSPECIFIED_IN_ROUTE = 01000, // An unspecified error occurred before reaching destination
				INCONSISTENT         = 01001, // The checksum is incorrect, or the packet has some other serious inconsistency before reaching destination
				CANT_GET_THERE       = 01002, // The destination host cannot be reached from here.
				EXCESS_HOPS          = 01003, // The packet has passed through 15 internet routes without reaching its destination.
				TOO_BIG              = 01004, // The packet is too large to be forwarded through some intermediate network.
			                                  // The Error Parameter field contains the length of the largest packet that can be accommodated.
				CONGESTION_WARNING   = 01005,
				CONGESTION_DISCARD   = 01006,
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

		Type   type;
		UINT16 param;
		BLOCK  block;

		QString toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

}
