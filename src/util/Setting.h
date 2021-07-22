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

#include <QtCore>

class Setting {
public:
	class Display {
	public:
		quint32 width;
		quint32 height;

		Display() : width(0), height(0) {}
		Display(QXmlStreamReader& reader);
	};

	class File {
	public:
		QString disk;
		QString germ;
		QString boot;
		QString floppy;

		File() : disk(""), germ(""), boot(""), floppy("") {}
		File(QXmlStreamReader& reader);
	};

	class Boot {
	public:
		QString switch_; // To avoid using keyword as variable name, append underscore
		QString device;

		Boot() : switch_(""), device("") {}
		Boot(QXmlStreamReader& reader);
	};

	class Memory {
	public:
		quint32 vmbits;
		quint32 rmbits;

		Memory() : vmbits(0), rmbits(0) {}
		Memory(QXmlStreamReader& reader);
	};

	class Network {
	public:
		QString interface;

		Network() : interface("") {}
		Network(QXmlStreamReader& reader);
	};

	class Entry {
	public:
		QString name; // attribute
		Display display;
		File    file;
		Boot    boot;
		Memory  memory;
		Network network;

		Entry() : name("") {}
		Entry(QXmlStreamReader& reader);
	};

	static QMap<QString,          Setting::Entry> entryMap;
	//          name              entry
	static QHash<quint32,         quint32>        keyMap;
	//           scanCode         bitPosition
	static QHash<Qt::MouseButton, quint32>        buttonMap;
	//           Qt::MouseButton  bitPosition

	static Entry getInstance(QString name);
};

#endif
