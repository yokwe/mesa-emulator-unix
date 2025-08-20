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
// NameMap.h
//

#pragma once

#include <QtCore>

namespace NameMap {
	template <typename T>
	class Map {
	public:
		Map(std::function<std::string(uint16_t)> func, std::initializer_list<std::pair<T, std::string>> list) {
			toStringDefault = func;
			map = list;
		}

		void add(T value, std::string name) {
			map[value] = name;
		}
		std::string toString(T value) const {
			if (map.contains(value)) {
				return map[value];
			} else {
				return toStringDefault((uint16_t)value);
			}
		}

	protected:
		std::function<std::string(uint16_t)> toStringDefault;

	private:
		QMap<T, std::string> map;
	};

	// predefined function of std::function<std::string(uint16_t)>
	std::string toString8u(uint8_t value);
	std::string toString16u(uint16_t value);
	std::string toString32u(uint32_t value);
	std::string toString64u(uint64_t value);

	std::string toString8X(uint8_t value);
	std::string toString16X(uint16_t value);
	std::string toString32X(uint32_t value);
	std::string toString64X(uint64_t value);

	std::string toString16X04(uint16_t value);
}
