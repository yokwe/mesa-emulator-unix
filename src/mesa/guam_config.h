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
 // guam_config.h
 //

#pragma once

#include <string>
#include <deque>

class guam_config {
public:
	class Entry {
	public:
		class Display {
		public:
			std::string type;
			int width;
			int height;

			Display() : width(0), height(0) {}
		};

		class File {
		public:
			std::string disk;
			std::string germ;
			std::string boot;
			std::string floppy;

			File() : disk(""), germ(""), boot(""), floppy("") {}
		};

		class Boot {
		public:
			std::string switch_; // To avoid using keyword as variable name, append underscore
			std::string device;

			Boot() : switch_(""), device("") {}
		};

		class Memory {
		public:
			int vmbits;
			int rmbits;

			Memory() : vmbits(0), rmbits(0) {}
		};

		class Network {
		public:
			std::string interface;

			Network() : interface("") {}
		};

		std::string name;
		Display display;
		File    file;
		Boot    boot;
		Memory  memory;
		Network network;

		Entry() : name(""), display(), file(), boot(), memory(), network() {}
	};

	std::deque<guam_config::Entry>  entryList;

	guam_config() {}
	guam_config(const guam_config& that) : entryList(that.entryList) {}

	static guam_config getInstance() {
		return getInstance("data/guam-config.json");
	}
	static guam_config getInstance(const std::string& path);
	
	Entry getEntry(const std::string& name);
	bool containsEntry(const std::string& name);
};
