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

#include "Setting.h"

static log4cpp::Category& logger = Logger::getLogger("setting");

static const char* PATH_FILE = "data/Guam/setting.xml";

static const char* XML_SETTING = "setting";

static const char* XML_DISPLAY = "display";
static const char* XML_WIDTH   = "width";
static const char* XML_HEIGHT  = "height";

static const char* XML_FILE   = "file";
static const char* XML_DISK   = "disk";
static const char* XML_GERM   = "germ";
static const char* XML_BOOT   = "boot";
static const char* XML_FLOPPY = "floppy";

//const char* XML_BOOT   = "boot";
static const char* XML_SWITCH = "switch";
static const char* XML_DEVICE = "device";

static const char* XML_MEMORY = "memory";
static const char* XML_VMBITS = "vmbits";
static const char* XML_RMBITS = "rmbits";

static const char* XML_NETWORK   = "network";
static const char* XML_INTERFACE = "interface";

static const char* XML_ENTRY = "entry";
static const char* XML_NAME  = "name";

static void readAttribute(QXmlStreamAttributes& attributes, QString name, QString& value) {
	if (attributes.hasAttribute(name)) {
		value = attributes.value(name).toString();
	} else {
		logger.error("Unexpected no attribute %s!", name.toLocal8Bit().constData());
		ERROR();
	}
}

static quint32 toInt(QString string) {
	bool ok;
	quint32 ret = string.toInt(&ok, 10);

	if (!ok) {
		logger.error("Unexpected valueString %s!", string.toLocal8Bit().constData());
		ERROR();
	}

	return ret;
}

static void readAttribute(QXmlStreamAttributes& attributes, QString name, quint32& value) {
	QString stringValue;

	readAttribute(attributes, name, stringValue);

	value = toInt(stringValue);
}


Setting::Entry::Entry(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_ENTRY) {
		logger.error("Unexpected %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_NAME, name);

	while(reader.readNextStartElement()) {
		QString elementName = reader.name().toString();

		if (elementName == XML_DISPLAY) {
			Display newValue(reader);
			this->display = newValue;
		} else if (elementName == XML_FILE) {
			File newValue(reader);
			this->file = newValue;
		} else if (elementName == XML_BOOT) {
			Boot newValue(reader);
			this->boot = newValue;
		} else if (elementName == XML_MEMORY) {
			Memory newValue(reader);
			this->memory = newValue;
		} else if (elementName == XML_NETWORK) {
			Network newValue(reader);
			this->network = newValue;
		} else {
			logger.error("Unexpected name %s!", reader.name().toLocal8Bit().constData());
			ERROR();
		}
	}

//	logger.info("name    %s", name.toLocal8Bit().constData());
//	logger.info("display %d %d", display.width, display.height);
//	logger.info("file    %s  %s  %s  %s",
//		file.boot.toLocal8Bit().constData(), file.disk.toLocal8Bit().constData(), file.germ.toLocal8Bit().constData(), file.floppy.toLocal8Bit().constData());
//	logger.info("boot    %s  %s", boot.device.toLocal8Bit().constData(), boot.switch_.toLocal8Bit().constData());
//	logger.info("memory  %d %d", memory.rmbits, memory.vmbits);
//	logger.info("network %s", network.interface.toLocal8Bit().constData());
}

Setting::Display::Display(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_DISPLAY) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_WIDTH,  width);
	readAttribute(attributes, XML_HEIGHT, height);
	reader.skipCurrentElement();
}


Setting::File::File(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_FILE) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_BOOT,   boot);
	readAttribute(attributes, XML_DISK,   disk);
	readAttribute(attributes, XML_GERM,   germ);
	readAttribute(attributes, XML_FLOPPY, floppy);
	reader.skipCurrentElement();
}

Setting::Boot::Boot(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected  %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_BOOT) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_SWITCH, switch_);
	readAttribute(attributes, XML_DEVICE, device);
	reader.skipCurrentElement();
}

Setting::Memory::Memory(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_MEMORY) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_RMBITS, rmbits);
	readAttribute(attributes, XML_VMBITS, vmbits);
	reader.skipCurrentElement();
}

Setting::Network::Network(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected  %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_NETWORK) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_INTERFACE, interface);
	reader.skipCurrentElement();
}

void initialize(QMap<QString, Setting::Entry>& map) {
	QString path(PATH_FILE);

	QFile file(path);
	if (!file.exists()) {
		logger.error("Unexpected path %s", path.toLocal8Bit().constData());
		ERROR();
	}

	// Open file
	file.open(QFile::ReadOnly);
	QXmlStreamReader reader(&file);

	if (reader.readNextStartElement()) {
		// sanity check
		if (reader.name() != XML_SETTING) {
			logger.error("Unexpected token %s!", reader.tokenString().toLocal8Bit().constData());
			ERROR();
		}

		while(reader.readNextStartElement()) {
			Setting::Entry entry(reader);
			map[entry.name] = entry;
			logger.info("entry   %s", entry.name.toLocal8Bit().constData());
		}
	} else {
		logger.error("Unexpected token %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	// Close files
	file.close();
}

static QMap<QString, Setting::Entry> map;

Setting::Entry Setting::getInstance(QString name) {
	if (map.isEmpty()) {
		initialize(map);
	}

	if (map.contains(name)) {
		return map[name];
	} else {
		logger.error("Unexpected name %s", name.toLocal8Bit().constData());
		ERROR();
	}
}

