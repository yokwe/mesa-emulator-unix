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
#include <fstream>

#include <nlohmann/json.hpp>

#include "setting.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

using json = nlohmann::json;


#define simple(name) p.name = j.at(#name);

void from_json(const json& j, Setting::Entry::Display& p) {
	simple(width)
	simple(height)
}
void from_json(const json& j, Setting::Entry::File& p) {
	simple(disk)
	simple(germ)
	simple(boot)
	simple(floppy)
}
void from_json(const json& j, Setting::Entry::Boot& p) {
	p.switch_ = j.at("switch");
	simple(device)
}
void from_json(const json& j, Setting::Entry::Memory& p) {
	simple(vmbits)
	simple(rmbits)
}
void from_json(const json& j, Setting::Entry::Network& p) {
	simple(interface)
}
void from_json(const json& j, Setting::Entry& p) {
	simple(name)
	simple(display)
	simple(file)
	simple(boot)
	simple(memory)
	simple(network)
}
void from_json(const json& j, Setting::LevelVKeys& p) {
	simple(name)
	p.keyName = toIntMesaNumber(j.at("keyName"));
}
void from_json(const json& j, Setting::Keyboard& p) {
	simple(name)
	p.scanCode = toIntMesaNumber(j.at("scanCode"));
}
void from_json(const json& j, Setting::KeyMap& p) {
	simple(levelVKeys)
	simple(keyboard)
}
void from_json(const json& j, Setting::Mouse& p) {
	simple(name)
	p.bitMask = toIntMesaNumber(j.at("bitMask"));
}
void from_json(const json& j, Setting::ButtonMap& p) {
	simple(levelVKeys)
	simple(button)
}


Setting Setting::getInstance(const std::string& path) {
//	logger.info("path  %s", path);
	std::ifstream f(path);
	json data = json::parse(f);

	Setting ret;

	for(auto e: data["entry"]) {
		ret.entryList.push_back(e.template get<Entry>());
	}

	for(auto e: data["levelVKeys"]) {
		ret.levelVKeysList.push_back(e.template get<LevelVKeys>());
	}
	for(auto e: data["keyboard"]) {
		ret.keyboardList.push_back(e.template get<Keyboard>());
	}
	for(auto e: data["keyMap"]) {
		ret.keyMapList.push_back(e.template get<KeyMap>());
	}
	for(auto e: data["mouse"]) {
		ret.mouseList.push_back(e.template get<Mouse>());
	}
	for(auto e: data["buttonMap"]) {
		ret.buttonMapList.push_back(e.template get<ButtonMap>());
	}

	return ret;
}

Setting::Entry Setting::getEntry(const std::string& name) {
	for(auto e: entryList) {
		if (e.name == name) return e;
	}
	logger.error("Unexpected name");
	logger.error("  name  %s", name);
	ERROR()
}

bool Setting::containsEntry(const std::string& name) {
	for(auto e: entryList) {
		if (e.name == name) return true;
	}
	return false;
}
