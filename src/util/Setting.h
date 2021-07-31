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

#ifndef PREFERENCE_H__
#define PREFERENCE_H__

#include "JSONUtil.h"

#include <QtCore>

class Setting : public JSONBase {
public:
	class Entry : public JSONBase {
	public:
		class Display : public JSONBase {
		public:
			int width;
			int height;

			Display() : width(0), height(0) {}
			Display(const Display& that) : width(that.width), height(that.height) {}
			Display& operator= (const Display& that) {
				this->width  = that.width;
				this->height = that.height;
				return *this;
			}

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		class File : public JSONBase {
		public:
			QString disk;
			QString germ;
			QString boot;
			QString floppy;

			File() : disk(""), germ(""), boot(""), floppy("") {}
			File(const File& that) : disk(that.disk), germ(that.germ), boot(that.boot), floppy(that.floppy) {}
			File& operator= (const File& that) {
				this->disk   = that.disk;
				this->germ   = that.germ;
				this->boot   = that.boot;
				this->floppy = that.floppy;
				return *this;
			}

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		class Boot : public JSONBase {
		public:
			QString switch_; // To avoid using keyword as variable name, append underscore
			QString device;

			Boot() : switch_(""), device("") {}
			Boot(const Boot& that) : switch_(that.switch_), device(that.device) {}
			Boot& operator= (const Boot& that) {
				this->switch_ = that.switch_;
				this->device  = that.device;
				return *this;
			}

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		class Memory : public JSONBase {
		public:
			int vmbits;
			int rmbits;

			Memory() : vmbits(0), rmbits(0) {}
			Memory(const Memory& that) : vmbits(that.vmbits), rmbits(that.rmbits) {}
			Memory& operator= (const Memory& that) {
				this->vmbits = that.vmbits;
				this->rmbits = that.rmbits;
				return *this;
			}

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};

		class Network : public JSONBase {
		public:
			QString interface;

			Network() : interface("") {}
			Network(const Network& that) : interface(that.interface) {}
			Network& operator= (const Network& that) {
				this->interface = that.interface;
				return *this;
			}

			void fromJsonObject(const QJsonObject& jsonObject);
			QJsonObject toJsonObject() const;
		};


		QString name;
		Display display;
		File    file;
		Boot    boot;
		Memory  memory;
		Network network;

		Entry() : name(""), display(), file(), boot(), memory(), network() {}
		Entry(const Entry& that) : name(that.name), display(that.display), file(that.file), boot(that.boot), memory(that.memory), network(that.network) {}
		Entry& operator= (const Entry& that) {
			this->name    = that.name;
			this->display = that.display;
			this->file    = that.file;
			this->boot    = that.boot;
			this->memory  = that.memory;
			this->network = that.network;
			return *this;
		}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};


	class LevelVKeys : public JSONBase {
	public:
		QString name;
		int     keyName;

		LevelVKeys() : name(""), keyName(0) {}
		LevelVKeys(const LevelVKeys& that) : name(that.name), keyName(that.keyName) {}
		LevelVKeys& operator= (const LevelVKeys& that) {
			this->name    = that.name;
			this->keyName = that.keyName;
			return *this;
		}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};


	class Keyboard : public JSONBase {
	public:
		QString name;
		int     scanCode;

		Keyboard() : name(""), scanCode(0) {}
		Keyboard(const Keyboard& that) : name(that.name), scanCode(that.scanCode) {}
		Keyboard& operator= (const Keyboard& that) {
			this->name     = that.name;
			this->scanCode = that.scanCode;
			return *this;
		}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};


	class KeyMap : public JSONBase {
	public:
		QString levelVKeys;
		QString keyboard;

		KeyMap() : levelVKeys(""), keyboard("") {}
		KeyMap(const KeyMap& that) : levelVKeys(that.levelVKeys), keyboard(that.keyboard) {}
		KeyMap& operator= (const KeyMap& that) {
			this->levelVKeys = that.levelVKeys;
			this->keyboard   = that.keyboard;
			return *this;
		}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};


	class Mouse : public JSONBase {
	public:
		QString name;
		int     bitMask;

		Mouse() : name(""), bitMask(0) {}
		Mouse(const Mouse& that) : name(that.name), bitMask(that.bitMask) {}
		Mouse& operator= (const Mouse& that) {
			this->name    = that.name;
			this->bitMask = that.bitMask;
			return *this;
		}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};


	class ButtonMap : public JSONBase {
	public:
		QString levelVKeys;
		QString button;

		ButtonMap() : levelVKeys(""), button("") {}
		ButtonMap(const ButtonMap& that) : levelVKeys(that.levelVKeys), button(that.button) {}
		ButtonMap& operator= (const ButtonMap& that) {
			this->levelVKeys  = that.levelVKeys;
			this->button = that.button;
			return *this;
		}

		void fromJsonObject(const QJsonObject& jsonObject);
		QJsonObject toJsonObject() const;
	};


	QList<Setting::Entry>      entryList;
	QList<Setting::LevelVKeys> levelVKeysList;
	QList<Setting::Keyboard>   keyboardList;
	QList<Setting::KeyMap>     keyMapList;
	QList<Setting::Mouse>      mouseList;
	QList<Setting::ButtonMap>  buttonMapList;

	Setting() {}
	Setting(const Setting& that) :
		entryList(that.entryList), levelVKeysList(that.levelVKeysList), keyboardList(that.keyboardList),
		keyMapList(that.keyMapList), mouseList(that.mouseList), buttonMapList(that.buttonMapList) {}
	Setting& operator= (const Setting& that) {
		this->entryList      = that.entryList;
		this->levelVKeysList = that.levelVKeysList;
		this->keyboardList   = that.keyboardList;
		this->keyMapList     = that.keyMapList;
		this->mouseList      = that.mouseList;
		this->buttonMapList  = that.buttonMapList;
		return *this;
	}

	void fromJsonObject(const QJsonObject& jsonObject);
	QJsonObject toJsonObject() const;

	static Setting getInstance();
};

#endif
