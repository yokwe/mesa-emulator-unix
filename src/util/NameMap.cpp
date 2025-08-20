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

std::string NameMap::toString8u(uint8_t value) {
	return std::string::asprintf("%u", value);
}
std::string NameMap::toString16u(uint16_t value) {
	return std::string::asprintf("%u", value);
}
std::string NameMap::toString32u(uint32_t value) {
	return std::string::asprintf("%u", value);
}
std::string NameMap::toString64u(uint64_t value) {
	return std::string::asprintf("%llu", value);
}

std::string NameMap::toString8X(uint8_t value) {
	return std::string::asprintf("%X", value);
}
std::string NameMap::toString16X(uint16_t value) {
	return std::string::asprintf("%X", value);
}
std::string NameMap::toString32X(uint32_t value) {
	return std::string::asprintf("%X", value);
}
std::string NameMap::toString64X(uint64_t value) {
	return std::string::asprintf("%llX", value);
}

std::string NameMap::toString16X04(uint16_t value) {
	return std::string::asprintf("%04X", value);
}
