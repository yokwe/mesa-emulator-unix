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
// Config.h
//

#pragma once

#include "../util/JSONUtil.h"

namespace XNS {
	class Config : public JSONBase {
	public:
		class Network : public JSONBase {
		public:
			class Entry : public JSONBase {
			public:
				QString name;
				int     net;
				int     hop;

				void fromJsonObject(const QJsonObject& jsonObject);
				QJsonObject toJsonObject() const;
			};

			QString      interface;
			int          local;
			QList<Entry> list;

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		class Host : public JSONBase {
		public:
			class Entry : public JSONBase {
			public:
				QString name;
				quint64 value;

				void fromJsonObject(const QJsonObject& jsonObject);
				QJsonObject toJsonObject() const;
			};

			QList<Entry>  list;

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		class Time : public JSONBase {
		public:
			int  offsetDirection;  // east or west of prime meridian
			int  offsetHours;
			int  offsetMinutes;
			int  dstStart;         // 0 for no DST
			int  dstEnd;           // 0 for no DST

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		Network network;
		Host    host;
		Time    time;

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};
}
