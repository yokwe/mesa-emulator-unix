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
// MTRecord.h
//

#pragma once

#include "../util/Util.h"

#include "MesaBuffer.h"

#include "NameRecord.h"
#include "FTIndex.h"
#include "ENIndex.h"
#include "SGIndex.h"

//   CodeDesc: TYPE = RECORD [
    //     sgi: SGIndex, offset, length: CARDINAL];
    struct CodeDesc: public MesaBuffer::HasRead, public HasToString {
        SGIndex  sgi;
        uint16_t offset;
        uint16_t length;
    
        MesaBuffer& read(MesaBuffer& bb) override {
            bb.read(sgi, offset, length);
            return bb;
        }
    
        std::string toString() const override {
            return std_sprintf("[%s  %5d  %5d]", sgi.toString(), offset, length);
        }
    };
    
    // LinkLocation: TYPE = {frame, code, dontcare};
    enum class LinkLocation {
        FRAME, CODE, DONTCARE,
    };
    std::string toString(LinkLocation value);
    // MTRecord: TYPE = --MACHINE DEPENDENT-- RECORD [
    //     name: NameRecord,
    //     file: FTIndex,
    //     config: CTIndex,
    //     code: CodeDesc,
    //     sseg: SGIndex,
    //     links: LFIndex,
    //     linkLoc: LinkLocation,
    //     namedInstance, initial: BOOLEAN, 
    //     boundsChecks, nilChecks: BOOLEAN,
    //     tableCompiled, residentFrame, crossJumped, packageable: BOOLEAN,
    //     packed: BOOLEAN, linkspace: BOOLEAN,
    //     spare: PACKED ARRAY [0..4) OF BOOLEAN,
    //     framesize: [0..PrincOps.MaxFrameSize),
    //     entries: ENIndex,
    //     atoms: ATIndex];
    struct MTRecord : public MesaBuffer::HasRead, public HasToString {
        // LinkLocation: TYPE = {frame, code, dontcare};
        enum class LinkLocation {
            FRAME, CODE, DONTCARE,
        };
    
        NameRecord   name;
        FTIndex      file;
        uint16_t     config;
        CodeDesc     code;
        SGIndex      sseg;
        uint16_t     links;
        LinkLocation linkLoc;        //  01
        bool         namedInstance;  //  2
        bool         initial;        //  3
        bool         boundsChecks;   //  4
        bool         nilChecks;      //  5
        bool         tableCompiled;  //  6
        bool         residentFrame;  //  7
        bool         crossJumped;    //  8
        bool         packageable;    //  9
        bool         packed;         // 10
        bool         linkspace;      // 11
        bool         spare0;         // 12
        bool         spare1;         // 13
        bool         spare2;         // 14
        bool         spare3;         // 15
        uint16_t     frameSize;
        ENIndex      entries;
        uint16_t     atoms;
    
        MesaBuffer& read(MesaBuffer& bb) override;
        std::string toString() const override;
    };
