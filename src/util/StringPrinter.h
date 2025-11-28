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
// StringPrinter.h
//

#pragma once

#include <string>

#include "Util.h"


class StringPrinter final {
    static const inline int   BUFFER_RESERVE  = 65536;
    static const inline int   MINIMU_CAPACITY = 1024;
    static const inline char* NEW_LINE = "\n";
    static const inline char* TAB = "    ";

    std::string buffer;
    int tabLevel = 0;
public:
    StringPrinter() {
        buffer.reserve(BUFFER_RESERVE);
    }

    void clear() {
        tabLevel = 0;
        buffer.clear();
        buffer.reserve(BUFFER_RESERVE);
    }
    const std::string& toString() const {
        return buffer;
    }

    void save(const std::string& path);

    //
    // new line
    //
    StringPrinter& newLine();

    //
    // tab
    //
    StringPrinter& indent();
    StringPrinter& unindent();
    StringPrinter& tab();

    //
    // print
    //
    StringPrinter& print(const std::string& value) {
        buffer += value;
        return *this;
    }
    template <typename T>
    StringPrinter& print_(const char* format, T value) {
        return print(std_sprintf(format, value));
    }
    template<typename ... Args>
    StringPrinter& print_(const char* format, Args&& ... args) {
        return print(std_sprintf(format, args ...));
    }
    // unsigned
    StringPrinter& print(uint16_t value) {
        return print_("%u", value);
    }
    StringPrinter& print(uint32_t value) {
        return print_("%u", value);
    }
    // signed
    StringPrinter& print(int16_t value) {
        return print_("%d", value);
    }
    StringPrinter& print(int32_t value) {
        return print_("%d", value);
    }
    // string
    StringPrinter& print(const char* format, const std::string& value) {
        return print_(format, value);
    }
    
    //
    // println
    //
    StringPrinter& println(const std::string& value) {
        return print(value).newLine();
    }
    // unsigned
    StringPrinter& println(uint16_t value) {
        return print_("%u", value).newLine();
    }
    StringPrinter& println(uint32_t value) {
        return print_("%u", value).newLine();
    }
    // signed
    StringPrinter& println(int16_t value) {
        return print_("%d", value).newLine();
    }
    StringPrinter& println(int32_t value) {
        return print_("%d", value).newLine();
    }
    // string
    StringPrinter& println(const char* format, const std::string& value) {
        return print_(format, value).newLine();
    }

};