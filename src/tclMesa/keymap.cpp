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
// keybord.cpp
//

#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include "../mesa/Pilot.h"

#include "keymap.h"

#include "../util/Util.h"
static const Logger logger(__FILE__);

namespace keymap {

using json = nlohmann::json;

struct KeyMap {
    std::string keySym;
    std::string levelVKey;
};

#define simple(name) p.name = j.at(#name);

struct NameCode {
    std::string name;
    int         code;
};

void from_json(const json& j, NameCode& p) {
	simple(name)
	simple(code)
}
void from_json(const json& j, LevelVKey& p) {
	simple(name)
	simple(keyName)
}
void from_json(const json& j, KeyMap& p) {
	simple(keySym)
	simple(levelVKey)
}

std::vector<NameCode> getNameCodeVector(const std::string& path, const std::string& name) {
	std::vector<NameCode> ret;

	std::ifstream f(path);
	json data = json::parse(f);
	for(auto e: data[name]) {
		ret.push_back(e.template get<NameCode>());
	}

	return ret;
}
std::vector<KeyMap> getKeyMapVector(const std::string& path) {
	std::vector<KeyMap> ret;

	std::ifstream f(path);
	json data = json::parse(f);
	for(auto e: data["KeyMap"]) {
		ret.push_back(e.template get<KeyMap>());
	}

	return ret;
}

std::map<int, LevelVKey> keyMap;
//       KeySymNumber

void initialize() {
    std::vector<NameCode> keySymVector     = getNameCodeVector("data/KeySym.json", "KeySym");
    std::vector<NameCode> levelVKeysVector = getNameCodeVector("data/LevelVKeys.json", "LevelVKeys");
    std::vector<KeyMap>   keyMapVector     = getKeyMapVector("data/KeyMap.json");

    std::map<std::string, int> keySymMap;
    for(auto e: keySymVector) {
        keySymMap[e.name] = e.code;
    }
    std::map<std::string, LevelVKey> levelVKeysMap;
    for(const auto& e: levelVKeysVector) {
        LevelVKey entry = {e.name, (LevelVKeys::KeyName)e.code};
        levelVKeysMap[e.name] = entry;
    }

    keyMap.clear();
    for(const auto& e: keyMapVector) {
        if (e.keySym    == "null") continue;
        if (e.levelVKey == "null") continue;

        // sanity check
        if (!keySymMap.contains(e.keySym)) {
            logger.error("Unexpected keySym  %s", e.keySym);
            ERROR();
        }
        if (!levelVKeysMap.contains(e.levelVKey)) {
            logger.error("Unexpected levelVKeyes  %s", e.levelVKey);
            ERROR();
        }
        int key = keySymMap.at(e.keySym);
        auto value = levelVKeysMap.at(e.levelVKey);

        keyMap[key] = value;
    }
    logger.info("keyMap  %d", keyMap.size());
}

static LevelVKey null = {"null", LevelVKeys::null};

const LevelVKey& getLevelVKey(int keySymNumber) {
    if (keyMap.empty()) initialize();
    
    if (keyMap.contains(keySymNumber)) {
        return keyMap[keySymNumber];
    } else {
        return null;
    }
}

}