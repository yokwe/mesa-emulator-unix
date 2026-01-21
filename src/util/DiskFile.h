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
// DiskFile.h
//

#pragma once

#include <cstdint>
#include <string>

#include "ByteBuffer.h"


class DiskFile {
public:
	static const uint32_t PAGE_SIZE = 256;
	static const uint32_t PAGE_SIZE_IN_BYTE = PAGE_SIZE * sizeof(uint16_t);

	using PageData = uint16_t[PAGE_SIZE];
	struct Page : public ByteBuffer::HasRead, public ByteBuffer::HasWrite {
		PageData data;

		ByteBuffer& read(ByteBuffer& bb) override;
		ByteBuffer& write(ByteBuffer& bb) const override;

		void byteswap();
	};

	// default constructor
	DiskFile() {
		pageData = 0;
		byteSize = 0;
		pageSize = 0;
	}

	void attach(const std::string& path);
	void detach();

	void readPage  (uint32_t pageNo, uint16_t *buffer);
	void writePage (uint32_t pageNo, uint16_t *buffer);
	int  verifyPage(uint32_t pageNo, uint16_t *buffer);
	void zeroPage  (uint32_t pageNo);

	void readPage  (uint32_t pageNo, Page& page) {
		readPage(pageNo, page.data);
	}
	void writePage (uint32_t pageNo, Page& page) {
		writePage(pageNo, page.data);
	}
	int  verifyPage(uint32_t pageNo, Page& page) {
		return verifyPage(pageNo, page.data);
	}

	const std::string& getPath() {
		return path;
	}
	uint32_t getByteSize() {
		return byteSize;
	}
	uint32_t getPageSize() {
		return pageSize;
	}

private:
	std::string path;
	PageData*   pageData;
	uint32_t    byteSize;
	uint32_t    pageSize;
};
