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



#undef  ENUM_VALUE
#define ENUM_VALUE(enum,value) {enum::value, #value},

std::string toString(ContextLevel value) {
    static std::map<ContextLevel, std::string> map {
        ENUM_VALUE(ContextLevel, LZ)
        ENUM_VALUE(ContextLevel, LG)
        ENUM_VALUE(ContextLevel, LL)
    };

    if (map.contains(value)) return map[value];
    return std_sprintf("%2d", (uint16_t)value);
    // logger.error("Unexpected value");
    // logger.error("  value  %d", (uint16_t)value);
    // ERROR();
}

std::string toString(ExtensionType value) {
    static std::map<ExtensionType, std::string> map {
		ENUM_VALUE(ExtensionType, VALUE)
		ENUM_VALUE(ExtensionType, FORM)
		ENUM_VALUE(ExtensionType, DEFAULT)
		ENUM_VALUE(ExtensionType, NONE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

std::string toString(TypeClass value) {
    static std::map<TypeClass, std::string> map {
        ENUM_VALUE(TypeClass, MODE)
        ENUM_VALUE(TypeClass, BASIC)
        ENUM_VALUE(TypeClass, ENUMERATED)
        ENUM_VALUE(TypeClass, RECORD)
        ENUM_VALUE(TypeClass, REF)
        //
        ENUM_VALUE(TypeClass, ARRAY)
        ENUM_VALUE(TypeClass, ARRAYDESC)
        ENUM_VALUE(TypeClass, TRANSFER)
        ENUM_VALUE(TypeClass, DEFINITION)
        ENUM_VALUE(TypeClass, UNION)
        //
        ENUM_VALUE(TypeClass, SEQUENCE)
        ENUM_VALUE(TypeClass, RELATIVE)
        ENUM_VALUE(TypeClass, SUBRANGE)
        ENUM_VALUE(TypeClass, LONG) 
        ENUM_VALUE(TypeClass, REAL)
        //
        ENUM_VALUE(TypeClass, OPAQUE)
        ENUM_VALUE(TypeClass, ZONE)
        ENUM_VALUE(TypeClass, ANY)
        ENUM_VALUE(TypeClass, NIL)
        ENUM_VALUE(TypeClass, BITS)
        ENUM_VALUE(TypeClass, BITS)
		//
		ENUM_VALUE(TypeClass, FIXEDSEQUENCE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}

std::string toString(TransferMode value) {
    static std::map<TransferMode, std::string> map {
		ENUM_VALUE(TransferMode, PROC)
		ENUM_VALUE(TransferMode, PORT)
		ENUM_VALUE(TransferMode, SIGNAL)
		ENUM_VALUE(TransferMode, ERROR_)
		ENUM_VALUE(TransferMode, PROCESS)
		ENUM_VALUE(TransferMode, PROGRAM)
		ENUM_VALUE(TransferMode, NONE)
    };

    if (map.contains(value)) return map[value];
    logger.error("Unexpected value");
    logger.error("  value  %d", (uint16_t)value);
    ERROR();
}
