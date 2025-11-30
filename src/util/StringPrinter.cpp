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
// StringPrinter.cpp
//

#include <string>

#include "Util.h"
static const Logger logger(__FILE__);

#include "StringPrinter.h"

void StringPrinter::save(const std::string& path) {
    writeFile(path, buffer);
}


StringPrinter& StringPrinter::newLine() {
    // maintain bufffer capacity
    if (buffer.capacity() < MINIMU_CAPACITY) buffer.reserve(BUFFER_RESERVE);

    std::string wholeLine;

    // add line to buffer
    // add TAB
    for(int i = 0; i < tabLevel; i++) wholeLine += TAB;
    // add line
    wholeLine += line;

    logger.info("XX %s", wholeLine);

    buffer += wholeLine;
    buffer += NEW_LINE;
    line.clear();

    return *this;
}


StringPrinter& StringPrinter::nest() {
    tabLevel++;
    return *this;
}
StringPrinter& StringPrinter::unnest() {
    tabLevel--;
    if (tabLevel < 0) ERROR()
    return *this;
}


StringPrinter& StringPrinter::print(const std::string& value) {
    // FIXME call indent/unindent using content of value
    //   if value contains '{', call indent()   except not in string literal 
    //   if value contains '}', call unindent() except not in string literal
    line += value;
    return *this;
}
