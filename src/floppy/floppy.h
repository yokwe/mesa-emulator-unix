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
// floppy.h
//

#pragma once

#include <vector>

#include "../util/DiskFile.h"

#include "ByteBuffer.h"

class FloppyDisk {
	DiskFile   diskFile;
public:
	// APilot/15.0.1/Pilot/Public/FileTypes.emsa
	//  FileTypes.mesa  CommonSoftwareFileType: TYPE = CARDINAL [2048..3072);
	static const uint16_t CommonSoftwareFileType    = 2048;

	// APilot/15.0.1/ComSoft/Public/CommonSoftwareFileTypes.emsa
	static const uint16_t COMMON_SOFTWARE_FILE_TYPE = 2048;
	static const uint16_t tUnassigned               = CommonSoftwareFileType + 0;
	static const uint16_t tDirectory                = CommonSoftwareFileType + 1;
	static const uint16_t tBackstopLog              = CommonSoftwareFileType + 3;
	static const uint16_t tCarryVolumeDirectory     = CommonSoftwareFileType + 4;
	static const uint16_t tClearingHouseBackupFile  = CommonSoftwareFileType + 5;
	static const uint16_t tFilelist                 = CommonSoftwareFileType + 6;
	static const uint16_t tBackstopDebugger         = CommonSoftwareFileType + 7;
	static const uint16_t tBackstopDebuggee         = CommonSoftwareFileType + 8;

	// AMesa/14.0/Floppy/Friends/AccessFloppy.mesa
	static const uint16_t tFloppyLeaderPage         = tCarryVolumeDirectory;

	FloppyDisk(const char* path) {
		diskFile.attach(path);
	}
	~FloppyDisk() {
		diskFile.detach();
	}

	void readSector(int sector, ByteBuffer& bb, uint32_t count);
	void readSector(int sector, ByteBuffer& bb) {
		readSector(sector, bb, 1);
	}
};

//
// APilot/15.0.1/ComSoft/Private/FloppyFormat.mesa
//
struct SectorNine {
	static const uint16_t SEAL    = 0141414;
	static const uint16_t VERSION =       1;

	uint16_t seal;
	uint16_t version;
	uint16_t cylinders;
	uint16_t tracksPerCylinder;
	uint16_t sectorsPerTrack;
	uint16_t fileList;
	uint32_t fileListID;
	uint16_t fileListSize;
	uint32_t rootFile;
	uint16_t alternateMicrocode;
	uint16_t pilotMicrocode;
	uint16_t diagnosticMicrocode;
	uint16_t germ;
	uint16_t pilotBootFile;
	uint16_t firstAlternateSector;
	uint16_t countBadSectors;
	uint32_t nextUnusedFileID;
	uint16_t changing;
	uint16_t labelSize;
	std::string label;

	SectorNine(FloppyDisk& floppyDisk);

	void dump();
};


//
// APilot/15.0.1/ComSoft/Private/FloppyFormat.mesa
//
struct FileList {
public:
	static const uint16_t SEAL    = 0131313;
	static const uint16_t VERSION =       1;

	class Entry {
	public:
		uint32_t file;
		uint16_t type;
		uint16_t location;
		uint16_t size;

		Entry(ByteBuffer& bb);
		// Entry(const Entry& that) {
		// 	file     = that.file;
		// 	type     = that.type;
		// 	location = that.location;
		// 	size     = that.type;
		// }
		void dump() const;
	};

	uint16_t            seal;
	uint16_t            version;
	uint16_t            count;
	uint16_t            maxEntries;
	std::vector<Entry*> files;

	FileList(FloppyDisk& floppyDisk, uint32_t fileList, uint32_t fileListSize);
	void dump() const;
};


//
// AMesa/14.0/Floppy/Friends/AccessFloppy.mesa
//
class FloppyLeaderPage {
public:
	static const int SEAL    = 0125252;
	static const int VERSION =       1;
    static const int MAX_NAME_LENGTH = 100;

	// Identity attributes
	uint16_t seal;     // used to check consistency of a file.
	uint16_t version;  // version of attributes record type.
	uint16_t type;     // file type of containing file.

	// Activity attributes
	uint32_t createData;       // the creation data of the disk file.
	uint32_t lastWrittenData;  // the date the disk file was last modified prior to copying to the floppy.

	// File attributes
	uint32_t size;              // number of pages in the floppy file not including the leader page.
	uint32_t offset;            // page number in the disk file corresponding to the first page in the floppy file piece.
	uint32_t totalSize;         // number of pages in the disk file.
	uint32_t totalSizeInBytes;  // the number of bytes in the disk file

	//Name attributes
	//  -- name attributes
	//  length: CARDINAL _ 0,
	//  maxLength: CARDINAL _ maxNameLength, -- so that @length is STRING.
	//  name: PACKED ARRAY [0..maxNameLength) OF Environment.Byte,
	uint16_t    nameLength;
	uint16_t    nameMaxLength;
	std::string name;

	//Client attributes
	//  -- client attributes
	//  clientDataLength: CARDINAL _ 0 ,-- number of component in client's private data.
	//  clientData: SEQUENCE maxlength: CARDINAL OF UNSPECIFIED
	uint16_t              clientDataLength;  // number of component in client's private data.
	uint16_t              clientDataMaxLength;  // number of component in client's private data.
	std::vector<uint16_t> clientData;

    // file contents
	std::vector<uint8_t>  contents;

    FloppyLeaderPage(FloppyDisk& floppyDisk, const FileList::Entry& entry);
    void dump() const;
};
