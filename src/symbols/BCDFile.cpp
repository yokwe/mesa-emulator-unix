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
// BCDFile.cpp
//

#include "../util/Util.h"
static const util::Logger logger(__FILE__);

#include "BCDFile.h"

#include "BCD.h"
#include "Symbols.h"

CARD16 BCDFile::getCARD16() {
	CARD16 b0 = getCARD8();
	CARD16 b1 = getCARD8();
	return (b0 << 8) | b1;
}
CARD32 BCDFile::getCARD32() {
	CARD32 b0 = getCARD16();
	CARD32 b1 = getCARD16();
	return (b1 << 16) | b0;
}
void   BCDFile::get(int size, CARD8* data) {
	for(int i = 0; i < size; i++) {
		data[i] = getCARD8();
	}
}

bool BCDFile::isBCDFile() {
	int oldPosition = getPosition();

    position(0);
	CARD16 word0 = getCARD16();

	setPosition(oldPosition);
	return word0 == BCD::VersionID;
}
bool BCDFile::isSymbolsFile() {
	if (getLength() < Environment::wordsPerPage) return false;

	int oldPosition = getPosition();

	position(Environment::wordsPerPage);
    CARD16 word256 = getCARD16();

	setPosition(oldPosition);
    return word256 == Symbols::VersionID;
}


//
// BCDFileFile
//

class BCDFileFile : public BCDFile {
public:
	BCDFileFile(std::string path_) : path(path_), file(path_) {
		if (!file.exists()) {
			logger.fatal("File does not exist. path = %s", path);
			ERROR();
		}
		bool result = file.open(QIODevice::OpenModeFlag::ReadOnly);
		if (!result) {
			logger.fatal("File open error %s", file.errorString());
			ERROR();
		}
		capacity = file.size();
		logger.info("%s  %s  %d", __FUNCTION__, path, capacity);
	}
	~BCDFileFile() {
		if (file.isOpen()) file.close();
	}

	std::string getPath() {
		return path;
	}

	int getPosition() {
		return (int)file.pos();
	}
	void setPosition(int newPosition) {
		if (newPosition < 0) ERROR();
		if (capacity <= newPosition) ERROR();

		file.seek(newPosition);
	}
	CARD8 getCARD8() {
		char data;
		uint32_t nRead = (uint32_t)file.read(&data, 1);
		if (nRead != 1) {
			logger.fatal("nRead %d", nRead);
			ERROR();
		}
		return (CARD8)data;
	}
	CARD32 getLength() {
		return (CARD32)capacity;
	}

private:
	std::string path;
	QFile   file;
	int64_t  capacity;
};

BCDFile* BCDFile::getInstance(std::string path) {
	return new BCDFileFile(path);
}
