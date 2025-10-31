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
 // Config.cpp
 //

#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include "guam_config.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

using json = nlohmann::json;

#define simple(name) p.name = j.at(#name);

void from_json(const json& j, guam_config::Entry::Display& p) {
	simple(type)
	simple(width)
	simple(height)
}
void from_json(const json& j, guam_config::Entry::File& p) {
	simple(disk)
	simple(germ)
	simple(boot)
	simple(floppy)
}
void from_json(const json& j, guam_config::Entry::Boot& p) {
	p.switch_ = j.at("switch");
	simple(device)
}
void from_json(const json& j, guam_config::Entry::Memory& p) {
	simple(vmbits)
	simple(rmbits)
}
void from_json(const json& j, guam_config::Entry::Network& p) {
	simple(interface)
	simple(address)
}
void from_json(const json& j, guam_config::Entry& p) {
	simple(name)
	simple(display)
	simple(file)
	simple(boot)
	simple(memory)
	simple(network)
}


guam_config guam_config::getInstance(const std::string& path) {
//	logger.info("path  %s", path);
	std::ifstream f(path);
	json data = json::parse(f);

	guam_config ret;

	for(auto e: data["entry"]) {
		ret.entryList.push_back(e.template get<Entry>());
	}

	return ret;
}

guam_config::Entry guam_config::getEntry(const std::string& name) {
	for(auto e: entryList) {
		if (e.name == name) return e;
	}
	logger.error("Unexpected name");
	logger.error("  name  %s", name);
	ERROR()
}

bool guam_config::containsEntry(const std::string& name) {
	for(auto e: entryList) {
		if (e.name == name) return true;
	}
	return false;
}
