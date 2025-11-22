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

#include <string>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../bcd/BCDFile.h"
#include "../bcd/BCD.h"
#include "../bcd/Symbols.h"

int main(int, char**) {
	logger.info("START");

	setSignalHandler(SIGINT);
	setSignalHandler(SIGTERM);
	setSignalHandler(SIGHUP);
	setSignalHandler(SIGSEGV);

	{
//		std::string path = "tmp/bcd/MesaRuntime.symbols";
//		std::string path = "tmp/bcd/File.bcd";
		std::string path = "tmp/bcd/FileImpl.bcd";

		BCDFile bcdFile(path);
		ByteBuffer bb = bcdFile.getByteBuffer();
		BCD bcd = BCD::getInstance(bb);

		bcd.dump();
		bcd.dumpTable();
//		bcd.dumpIndex();

		logger.info("sourceFile      %s", bcd.sourceFile.toString());
		logger.info("unpackagedFile  %s", bcd.unpackagedFile.toString());
		for(const auto& e: bcd.sgTable) {
			logger.info("sg-%d  %s", e.first, e.second.toString());
		}

		if (bcd.hasSymbol()) {
			Symbols symbols = Symbols::getInstance(bb, bcd.getSymbolOffset());
			symbols.dump();
			symbols.dumpTable();
			symbols.dumpIndex();
		}
	}

	logger.info("STOP");
	return 0;
}
