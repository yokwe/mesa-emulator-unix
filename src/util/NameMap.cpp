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
// NameMap.cpp
//

#include "NameMap.h"

QString NameMap::toString8u(quint8 value) {
	return QString::asprintf("%u", value);
}
QString NameMap::toString16u(quint16 value) {
	return QString::asprintf("%u", value);
}
QString NameMap::toString32u(quint32 value) {
	return QString::asprintf("%u", value);
}
QString NameMap::toString64u(quint64 value) {
	return QString::asprintf("%llu", value);
}

QString NameMap::toString8X(quint8 value) {
	return QString::asprintf("%X", value);
}
QString NameMap::toString16X(quint16 value) {
	return QString::asprintf("%X", value);
}
QString NameMap::toString32X(quint32 value) {
	return QString::asprintf("%X", value);
}
QString NameMap::toString64X(quint64 value) {
	return QString::asprintf("%llX", value);
}

QString NameMap::toString16X04(quint16 value) {
	return QString::asprintf("%04X", value);
}
