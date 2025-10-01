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
 // Error.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Error.h"

namespace xns::error {

void initialize() {
    logger.info("%s  intialize", __FUNCTION__);
}

UINT16 ErrorNumber::UNSPEC               = ErrorNumber(0, "UNSPEC");
UINT16 ErrorNumber::BAD_CHECKSUM         = ErrorNumber(1, "BAD_CHECKSUM");
UINT16 ErrorNumber::NO_SOCKET            = ErrorNumber(2, "NO_SOCKET");
UINT16 ErrorNumber::RESOURCE_LIMIT       = ErrorNumber(3, "RESOURCE_LIMIT");

UINT16 ErrorNumber::LISTEN_REJECT        = ErrorNumber(4, "LISTEN_REJECT");
UINT16 ErrorNumber::INVALID_PACKET_TYPE  = ErrorNumber(5, "INVALID_PACKET_TYPE");
UINT16 ErrorNumber::PROTOCOL_VIOLATION   = ErrorNumber(6, "PROTOCOL_VIOLATION");

UINT16 ErrorNumber::UNSPECIFIED_IN_ROUTE = ErrorNumber(01000, "UNSPECIFIED_IN_ROUTE");
UINT16 ErrorNumber::INCONSISTENT         = ErrorNumber(01001, "INCONSISTENT");
UINT16 ErrorNumber::CANT_GET_THERE       = ErrorNumber(01002, "CANT_GET_THERE");
UINT16 ErrorNumber::EXCESS_HOPS          = ErrorNumber(01003, "EXCESS_HOPS");
UINT16 ErrorNumber::TOO_BIG              = ErrorNumber(01004, "TOO_BIG");

UINT16 ErrorNumber::CONGESTION_WARNING   = ErrorNumber(01005, "CONGESTION_WARNING");
UINT16 ErrorNumber::CONGESTION_DISCARD   = ErrorNumber(01006, "CONGESTION_DISCARD");

}
