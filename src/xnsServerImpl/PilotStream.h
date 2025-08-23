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
// PilotStream.h
//

#pragma once


#include "../xns/SPP.h"

namespace XNS::PilotStream {
	enum class CompletionCode {
		NORMAL        = 0,
		END_RECORD    = 1,
		SST_CHANGE    = 2,
		END_OF_STREAM = 3,
		ATTENTION     = 4,
		TIMEOUT       = 5
	};

	enum class SST : uint8_t {
		// From Courier/Friends/CourierProtocol.mesa
		DATA        = 0,   // Courier
		// From Courier/Private/BulkData.mesa
		BULK        = 1,   // Bulk Data
		// From NS/Public/NetworkStream.mesa
		CLOSE       = 254, // Closing connection
		CLOSE_REPLY = 255, // Reply of CLOSE (handshake)
	};

	class Result {
	public:
		CompletionCode why;
		SST            sst;
	};

	class Stream {
	public:
		virtual ~Stream() {}

		virtual void get(QByteArray& data, Result& result) = 0;
		virtual void put(QByteArray& data, bool endRecord = false) = 0;

		virtual void setSST(SST  sst) = 0;
		virtual void getSST(SST& sst) = 0;

		virtual void sendAttention   (uint8_t  data) = 0;
		virtual void waitForAttention(uint8_t& data) = 0;

		virtual void setTimeout(uint16_t  msec) = 0;
		virtual void getTimeout(uint16_t& msec) = 0;

		virtual void close() = 0;
	};

}



