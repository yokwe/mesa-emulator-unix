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

#include <filesystem>
#include <fstream>
#include <vector>

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "floppy.h"

int main(int /*argc*/, char** /*argv*/) {
	const char* floppyImagePath   = "build/run/floppy144";
	const char* floppyDiskDirPath = "build/run/floppy-disk";

	logger.info("floppyDiskDirPath = %s", floppyDiskDirPath);
	logger.info("floppyImagePath   = %s", floppyImagePath);

	FloppyDisk floppyDisk(floppyImagePath);

	SectorNine sectorNine(floppyDisk);
	sectorNine.dump();
	logger.info("floppy  %d-%d-%d  %s", sectorNine.cylinders, sectorNine.tracksPerCylinder, sectorNine.sectorsPerTrack, sectorNine.label);

	FileList fileList(floppyDisk, sectorNine.fileList, sectorNine.fileListSize);
	fileList.dump();

	// get floppy name from sectorNine
	std::string floppyName(sectorNine.label);

	// prepare floppyDiskDirPath
    auto floppyDiskDir = std::filesystem::path(floppyDiskDirPath);
    auto floppyDir = floppyDiskDir / floppyName;

    if (std::filesystem::exists(floppyDir)) {
        std::filesystem::remove_all(floppyDir);
    }
	// create empty floppyName directory
    std::filesystem::create_directories(floppyDir);

	// set current directory as
	logger.info("floppyDir = %s", floppyDir.string());

	for(int i = 0; i < fileList.count; i++) {
		const FileList::Entry& entry = *fileList.files.at(i);
		if (entry.type != FloppyDisk::tFloppyLeaderPage) continue;
		FloppyLeaderPage leaderPage(floppyDisk, entry);

        auto name = leaderPage.name;
		logger.info("  file %8d  %s  %s", leaderPage.totalSizeInBytes, Util::toString(Util::toUnixTime(leaderPage.createData)), name);
		//leaderPage.dump();

		// create file in floppyDir
        auto path = floppyDir / name;
        auto contents = leaderPage.contents;

        std::ofstream of(path, std::ios::out | std::ios::binary);
        of.write((char*)contents.data(), contents.size());
        of.close();
	}

	return 0;
}
