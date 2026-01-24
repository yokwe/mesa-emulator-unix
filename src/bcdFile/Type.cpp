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
// Type.cpp
//

#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "Type.h"

//
// Timestamp
//
Timestamp Timestamp::getNull() {
    static Timestamp ret(0, 0, 0);
    return ret;
}
static std::string toTimestamp(uint32_t unixTime) {
	time_t temp = unixTime;
    struct tm tm;
    localtime_r(&temp, &tm);
    return std_sprintf("%04d%02d%02d#%02d%02d%02d", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
std::string Timestamp::toString() const {
    return isNull() ? "#NULL" : std_sprintf("%s#%03d#%03d", toTimestamp(Util::toUnixTime(time)), net, host);
}



#undef  ENUM_NAME
#define ENUM_NAME(enum,name) {enum::name, #name},

std::string toString(ContextLevel value) {
    static std::map<ContextLevel, std::string> map {
        ENUM_NAME(ContextLevel, LZ)
        ENUM_NAME(ContextLevel, LG)
        ENUM_NAME(ContextLevel, LL)
    };

    if (map.contains(value)) return map[value];
    return std_sprintf("%2d", (uint16_t)value);
    // logger.error("Unexpected value");
    // logger.error("  value  %d", (uint16_t)value);
    // ERROR();
}

std::string toString(ExtensionType value) {
    static std::map<ExtensionType, std::string> map {
		ENUM_NAME(ExtensionType, VALUE)
		ENUM_NAME(ExtensionType, FORM)
		ENUM_NAME(ExtensionType, DEFAULT)
		ENUM_NAME(ExtensionType, NONE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

std::string toString(TypeClass value) {
    static std::map<TypeClass, std::string> map {
        ENUM_NAME(TypeClass, MODE)
        ENUM_NAME(TypeClass, BASIC)
        ENUM_NAME(TypeClass, ENUMERATED)
        ENUM_NAME(TypeClass, RECORD)
        ENUM_NAME(TypeClass, REF)
        //
        ENUM_NAME(TypeClass, ARRAY)
        ENUM_NAME(TypeClass, ARRAYDESC)
        ENUM_NAME(TypeClass, TRANSFER)
        ENUM_NAME(TypeClass, DEFINITION)
        ENUM_NAME(TypeClass, UNION)
        //
        ENUM_NAME(TypeClass, SEQUENCE)
        ENUM_NAME(TypeClass, RELATIVE)
        ENUM_NAME(TypeClass, SUBRANGE)
        ENUM_NAME(TypeClass, LONG) 
        ENUM_NAME(TypeClass, REAL)
        //
        ENUM_NAME(TypeClass, OPAQUE)
        ENUM_NAME(TypeClass, ZONE)
        ENUM_NAME(TypeClass, ANY)
        ENUM_NAME(TypeClass, NIL)
        ENUM_NAME(TypeClass, BITS)
        ENUM_NAME(TypeClass, BITS)
		//
		ENUM_NAME(TypeClass, FIXEDSEQUENCE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

std::string toString(TransferMode value) {
    static std::map<TransferMode, std::string> map {
		ENUM_NAME(TransferMode, PROC)
		ENUM_NAME(TransferMode, PORT)
		ENUM_NAME(TransferMode, SIGNAL)
		ENUM_NAME(TransferMode, ERROR_)
		ENUM_NAME(TransferMode, PROCESS)
		ENUM_NAME(TransferMode, PROGRAM)
		ENUM_NAME(TransferMode, NONE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
