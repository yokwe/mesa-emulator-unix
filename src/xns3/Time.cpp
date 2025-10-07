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
 // Time.cpp
 //

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include <ctime>

#include "Time.h"

namespace xns::time {

std::string toStringLocalTime(const uint32_t time) {
    time_t temp = (time_t)time;
    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%d-%02d-%02d %02d:%02d:%02d", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

#undef  DECL_CLASS_CONSTANT
#define DECL_CLASS_CONSTANT(type, name, value) constantMap.map[type :: name ] = #name;

void Version::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Version, CURRENT, 2)
}
void Type::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Type, REQUEST,  1)
    DECL_CLASS_CONSTANT(Type, RESPONSE, 2)
}
void Direction::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Direction, WEST, 0)
    DECL_CLASS_CONSTANT(Direction, EAST, 1)
}
void Tolerance::MyConstantMap::initialize() {
    DECL_CLASS_CONSTANT(Tolerance, UNKNOWN, 0)
    DECL_CLASS_CONSTANT(Tolerance, KNOWN  , 1)
}

}