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
// ModuleMap.h
//

#ifndef MODULEMAP_H__
#define MODULEMAP_H__

#include "../util/JSONUtil.h"

#include "QtCore"


namespace ModuleMap {
	class Module : JSONBase {
	public:
		//Global Frames for Modules in GermGuam.bcd:
		//			    Frame      Code (* = code links)
		//  GermOpsImpl               0AB0H    1209H     1CH

		QString module;
		int     gf;
		int     cb;
		int     gfi;

		Module() : module(""), gf(0), cb(0), gfi(0) {}
		Module(const Module& that) : module(that.module), gf(that.gf), cb(that.cb), gfi(that.gfi) {}
		Module& operator= (const Module& that) {
			this->module = that.module;
			this->gf     = that.gf;
			this->cb     = that.cb;
			this->gfi    = that.gfi;
			return *this;
		}

		Module(QString module_, int gf_, int cb_, int gfi_) : module(module_), gf(gf_), cb(cb_), gfi(gfi_) {}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;

		// read Guam.loadmap and returns QList<Module>
		static QList<Module> load(QString file);
	};

	class Map : JSONBase {
	public:
		// Bytes   EVI  Offset    IPC   Module               Procedure
		//    42B   13   1030B     20B  ProcessorHeadGuam    GetNextAvailableVM
		QString module;
		QString proc;
		int     bytes;
		int     evi;
		int     pc;

		Map() : module(""), proc(""), bytes(0), evi(0), pc(0) {}
		Map(const Map& that) : module(that.module), proc(that.proc), bytes(that.bytes), evi(that.evi), pc(that.pc) {}
		Map& operator= (const Map& that) {
			this->module = that.module;
			this->proc   = that.proc;
			this->bytes  = that.bytes;
			this->evi    = that.evi;
			this->pc     = that.pc;
			return *this;
		}

		Map(QString module_, QString proc_, int bytes_, int evi_, int pc_) : module(module_), proc(proc_), bytes(bytes_), evi(evi_), pc(pc_) {}


		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;

		// read *.map and returns QList<Map>
		static QList<Map> load(QString file);
	};

};


#endif
