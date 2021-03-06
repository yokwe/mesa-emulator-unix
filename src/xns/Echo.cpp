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
// Echo.cpp
//

#include "../util/Util.h"
static const Logger logger = Logger::getLogger("xns-echo");

#include "../util/JSONUtil.h"

#include "Echo.h"


//
// XNS::Echo::Type
//
NameMap::Map<quint16> XNS::Echo::Type::nameMap(NameMap::toString16u, {{REQUEST, "REQUEST"}, {REPLY, "REPLY"}});


//
// XNS::Echo
//
QString XNS::Echo::toString() const {
	return QString("%1 %2").arg(type.toString()).arg(block.toString());
}
void XNS::Echo::fromByteBuffer(ByteBuffer& bb) {
	FROM_BYTE_BUFFER(bb, type);
	FROM_BYTE_BUFFER(bb, block);
}
void XNS::Echo::toByteBuffer  (ByteBuffer& bb) const {
	TO_BYTE_BUFFER(bb, type);
	TO_BYTE_BUFFER(bb, block);
}
