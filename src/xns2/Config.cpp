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

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/net.h"

#include "Config.h"

using json = nlohmann::json;

#define simple(name) p.name = j.at(#name);

namespace xns::config {

void from_json(const json& j, Server& p) {
	simple(interface)
    std::string string = j.at("address");
    p.address = net::fromString(string);
	simple(net)
}
void from_json(const json& j, Net& p) {
    simple(net)
    simple(delay)
	simple(name)
}
void from_json(const json& j, Host& p) {
	simple(name)

    std::string string = j.at("address");
    p.address = net::fromString(string);
}
void from_json(const json& j, Time& p) {
	simple(offsetDirection)
    simple(offsetHours)
    simple(offsetMinutes)
	simple(dstStart)
	simple(dstEnd)
}


Config Config::getInstance(const std::string& path) {
//	logger.info("path  %s", path);
	std::ifstream f(path);
	json data = json::parse(f);

	Config ret;

    ret.server = data.at("server").template get<Server>();
	for(auto e: data["net"]) {
		ret.net.push_back(e.template get<Net>());
	}
	for(auto e: data["host"]) {
		ret.host.push_back(e.template get<Host>());
	}
    ret.time = data.at("time").template get<Time>();
    return ret;
}

}
