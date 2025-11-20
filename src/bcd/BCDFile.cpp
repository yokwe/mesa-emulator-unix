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
// BCDFile.cpp
//

#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <iterator>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"

#include "BCD.h"
#include "Symbols.h"

#include "BCDFile.h"

BCDFile::BCDFile(const std::string& path_) {
    path = path_;
    contents = readFile(path);

    bb = ByteBuffer(contents.size(), contents.data());

    bb.position(0);
    auto wordBCD = bb.get16();

    // logger.info("wordBCD  %d", wordBCD);
    // logger.info("wrodSYM  %d", wordSYM);

    bcdFile     = (wordBCD == BCD::VersionID);
    bb.rewind();

    logger.info("path     %s", path);
    if (bcdFile) {
        logger.info("This is bcd file.");
    } else {
        logger.error("This is not bcd file");
        ERROR();
    }
}

std::vector<uint8_t> BCDFile::readFile(const std::string& path) {
    // sanity check
    if (!std::filesystem::exists(path)) {
        logger.error("The path doesn't exist");
        logger.error("  path %s!", path);
        ERROR();
    }
    if (!std::filesystem::is_regular_file(path)) {
        logger.error("The path is not regular file");
        logger.error("  path %s!", path);
        ERROR();
    }

    std::ifstream ifs(path, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    ifs.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    ifs.seekg(0, std::ios::end);
    fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // reserve capacity
    std::vector<uint8_t> ret;
    ret.reserve(fileSize);

    // read the data:
    std::copy(
        std::istream_iterator<uint8_t>(ifs),
        std::istream_iterator<uint8_t>(),
        std::back_inserter(ret));
    return ret;
}
