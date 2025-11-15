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
// floppy.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "floppy.h"

void FloppyDisk::readSector(int sector, ByteBuffer& bb, uint32_t count) {
    if (sector <= 0) {
        logger.fatal("sector = %d", sector);
        ERROR();
    }
    uint16_t buffer[PageSize];
    for(uint32_t i = 0; i < count; i++) {
        diskFile.readPage((sector - 1) + i, (CARD16*)buffer, PageSize);
        Util::byteswap(buffer, buffer, PageSize);
        bb.write(Environment::bytesPerPage, (uint8_t*)buffer);
    }
    bb.rewind();
}


SectorNine::SectorNine(FloppyDisk& floppyDisk) {
    ByteBuffer bb(Environment::bytesPerPage);
    floppyDisk.readSector(9, bb);

    bb.readAll(seal, version, cylinders, tracksPerCylinder, sectorsPerTrack);
    // sanity check
    if (seal != SEAL) {
        logger.fatal("seal = %6o  SEAL = %6o", seal, SEAL);
        ERROR();
    }
    if (version != VERSION) {
        logger.fatal("version = %6d  VERSIO = %6d", version, VERSION);
        ERROR();
    }
    if (cylinders != 80) {
        logger.fatal("cylinders = %6d", cylinders);
        ERROR();
    }
    if (tracksPerCylinder != DiskFile::FLOPPY_NUMBER_OF_HEADS) {
        logger.fatal("tracksPerCylinder = %6d  FLOPPY_NUMBER_OF_HEADS = %6d", tracksPerCylinder, DiskFile::FLOPPY_NUMBER_OF_HEADS);
        ERROR();
    }
    if (sectorsPerTrack != DiskFile::FLOPPY_SECTORS_PER_TRACK) {
        logger.fatal("sectorsPerTrack = %6d  FLOPPY_SECTORS_PER_TRACK = %6d", tracksPerCylinder, DiskFile::FLOPPY_SECTORS_PER_TRACK);
        ERROR();
    }

    bb.readAll(
        fileList, fileListID, fileListSize, rootFile, alternateMicrocode, pilotMicrocode,diagnosticMicrocode,
        germ, pilotBootFile, firstAlternateSector, countBadSectors, nextUnusedFileID, changing, labelSize);
    for(int i = 0; i < labelSize; i++) {
        label += bb.get8();
    }
}
void SectorNine::dump() {
    logger.info("==== SectorNine ====");
    logger.info("seal                  %6o", seal);
    logger.info("version               %6d", version);
    logger.info("cylinders             %6d", cylinders);
    logger.info("tracksPerCylinder     %6d", tracksPerCylinder);
    logger.info("sectorsPerTrack       %6d", sectorsPerTrack);
    logger.info("fileList              %6d", fileList);
    logger.info("fileListID            %6d", fileListID);
    logger.info("fileListSize          %6d", fileListSize);
    logger.info("rootFile              %6d", rootFile);
    logger.info("alternateMicrocode    %6d", alternateMicrocode);
    logger.info("pilotMicrocode        %6d", pilotMicrocode);
    logger.info("diagnosticMicrocode   %6d", diagnosticMicrocode);
    logger.info("germ                  %6d", germ);
    logger.info("pilotBootFile         %6d", pilotBootFile);
    logger.info("firstAlternateSector  %6d", firstAlternateSector);
    logger.info("countBadSectors       %6d", countBadSectors);
    logger.info("nextUnusedFileID      %6d", nextUnusedFileID);
    logger.info("changing              %6d", changing);
    logger.info("labelSize             %6d", labelSize);
    logger.info("label                     \"%s\"", label);
    logger.info("====");
}


FileList::Entry::Entry(ByteBuffer& bb) {
    bb.readAll(file, type, location, size);
}
void FileList::Entry::dump() const {
    logger.info("file      %6d", file);
    logger.info("type      %6d", type);
    logger.info("location  %6d", location);
    logger.info("size      %6d", size);
}


FileList::FileList(FloppyDisk& floppyDisk, uint32_t fileList, uint32_t fileListSize) {
    uint32_t dataSize = Environment::bytesPerPage * fileListSize;
    ByteBuffer bb(dataSize);
    floppyDisk.readSector(fileList, bb, fileListSize);

    bb.readAll(seal, version);
    // sanity check
    if (seal != SEAL) {
        logger.fatal("seal = %6o  SEL = %6o", seal, SEAL);
        ERROR();
    }
    if (version != VERSION) {
        logger.fatal("version = %6d  VERSIO = %6d", version, VERSION);
        ERROR();
    }

    bb.readAll(count, maxEntries);
    for(int i = 0; i < count; i++) {
        Entry* entry = new Entry(bb);
        files.push_back(entry);
    }
}
void FileList::dump() const {
    logger.info("==== FileList ====");
    logger.info("seal        %6o", seal);
    logger.info("version     %6d", version);
    logger.info("count       %6d", count);
    logger.info("maxEntries  %6d", maxEntries);
    for(int i = 0; i < count; i++) {
        const Entry* entry = files.at(i);
        logger.info("----");
        entry->dump();
    }
    logger.info("----");
}


FloppyLeaderPage::FloppyLeaderPage(FloppyDisk& floppyDisk, const FileList::Entry& entry) {
    ByteBuffer bb(Environment::bytesPerPage * entry.size);
    floppyDisk.readSector(entry.location, bb, entry.size);

    bb.
    // Identity attributes
    readAll(seal, version, type);
    // sanity check
    if (seal != SEAL) {
        logger.fatal("seal = %6o  SEAL = %6o", seal, SEAL);
        ERROR();
    }
    if (version != VERSION) {
        logger.fatal("version = %6d  VERSIO = %6d", version, VERSION);
        ERROR();
    }

    bb.
	// Activity attributes
    readAll(createData, lastWrittenData).
	// File attributes
    readAll(size, offset, totalSize, totalSizeInBytes).
	//Name attributes
    readAll(nameLength, nameMaxLength);
    for(int i = 0; i < nameMaxLength; i++) {
        uint8_t c = bb.get8();
        if (i < nameLength) name += c;
    }

	//Client attributes
    bb.
    readAll(clientDataLength, clientDataMaxLength);
    for(uint16_t i = 0; i < clientDataMaxLength; i++) {
        uint16_t w = bb.get16();
        if (i < clientDataLength) clientData.push_back(w);
    }

    // sanitcy check
    auto contentsPos = bb.position();
    if (contentsPos != Environment::bytesPerPage) ERROR();

    // file contents
    for(uint32_t i = 0; i < totalSizeInBytes; i++) {
        contents.push_back(bb.get8());
    }
}

void FloppyLeaderPage::dump() const {
    logger.info("==== LeaderPage ====");
    logger.info("seal             %6o", seal);
    logger.info("version          %6d", version);
    logger.info("size             %6d", size);
    logger.info("offset           %6d", offset);
    logger.info("totalSize        %6d", totalSize);
    logger.info("totalSizeInBytes %6d", totalSizeInBytes);
    logger.info("createData             %s", Util::toString(Util::toUnixTime(createData)));
    logger.info("lastWrittenData        %s", Util::toString(Util::toUnixTime(lastWrittenData)));
    logger.info("name                   \"%s\"", name);
    logger.info("clientDataLength %6d", clientDataLength);
    logger.info("clientDataMax    %6d", clientData.size());
    logger.info("clientData       %6d", clientData.size());
    logger.info("contents         %6d", contents.size());
}
