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
// MTRecord.cpp
//

#include <cstdint>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "MesaBuffer.h"

#include "MTRecord.h"

MesaBuffer& MTRecord::read(MesaBuffer& bb) {
    uint16_t u8;
    bb.read(name, file, config, code, sseg, links, u8, frameSize, entries, atoms);

    linkLoc        = (LinkLocation)bitField(u8,  0, 1);
    namedInstance  = (bool)bitField(u8,  2,  2);
    initial        = (bool)bitField(u8,  3,  3);
    boundsChecks   = (bool)bitField(u8,  4,  4);
    nilChecks      = (bool)bitField(u8,  5,  5);
    tableCompiled  = (bool)bitField(u8,  6,  6);
    residentFrame  = (bool)bitField(u8,  7,  7);
    crossJumped    = (bool)bitField(u8,  8,  8);
    packageable    = (bool)bitField(u8,  9,  9);
    packed         = (bool)bitField(u8, 10, 10);
    linkspace      = (bool)bitField(u8, 11, 11);
    spare0         = (bool)bitField(u8, 12, 12);
    spare1         = (bool)bitField(u8, 13, 13);
    spare2         = (bool)bitField(u8, 14, 14);
    spare3         = (bool)bitField(u8, 15, 15);

    return bb;
}
std::string MTRecord::toString() const {
    return std_sprintf("[%s  %s  %s  %s]", name.toString(), file.toString(), code.toString(), sseg.toString());
}
