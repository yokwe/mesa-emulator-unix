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
// Perf.cpp
//

#include "Perf.h"

PERF_DEFFINE(Dispatch)
PERF_DEFFINE(Fetch)
PERF_DEFFINE(Store)
PERF_DEFFINE(ReadDbl)
PERF_DEFFINE(FetchMds)
PERF_DEFFINE(StoreMds)
PERF_DEFFINE(ReadDblMds)
PERF_DEFFINE(GetCodeByte)
PERF_DEFFINE(GetCodeWord)
PERF_DEFFINE(FetchByte)
PERF_DEFFINE(StoreByte)
PERF_DEFFINE(ReadField)
PERF_DEFFINE(WriteField)
PERF_DEFFINE(WriteMap)
PERF_DEFFINE(GetAddress)
PERF_DEFFINE(FetchPda)
PERF_DEFFINE(StorePda)
PERF_DEFFINE(MemoryFetch)
PERF_DEFFINE(MemoryStore)
// Fault
PERF_DEFFINE(FrameFault)
PERF_DEFFINE(PageFault)
// Trap
PERF_DEFFINE(CodeTrap)
PERF_DEFFINE(EscOpcodeTrap)
PERF_DEFFINE(OpcodeTrap)
PERF_DEFFINE(UnboundTrap)
