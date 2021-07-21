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

#include "./Preference.h"

static log4cpp::Category& logger = Logger::getLogger("preference");

static const char* PATH_FILE = "data/Guam/Preference.xml";


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


static QMap<QString, Preference> map;

Preference::Preference(QXmlStreamReader& reader) {
	if (reader.isStartElement() && reader.name() == "entry") {
		{
			QXmlStreamAttributes attributes = reader.attributes();
			readAttribute(attributes, "name", name);
		}
		while(reader.readNextStartElement()) {
			QXmlStreamAttributes attributes = reader.attributes();
			if (reader.name() == "display") {
				readAttribute(attributes, "width", display.width);
				readAttribute(attributes, "height", display.height);
			} else if (reader.name() == "file") {
				readAttribute(attributes, "boot",   file.boot);
				readAttribute(attributes, "disk",   file.disk);
				readAttribute(attributes, "germ",   file.germ);
				readAttribute(attributes, "floppy", file.floppy);
			} else if (reader.name() == "boot") {
				readAttribute(attributes, "device", boot.device);
				readAttribute(attributes, "switch", boot.switchString);
			} else if (reader.name() == "memory") {
				readAttribute(attributes, "rmbits", memory.rmbits);
				readAttribute(attributes, "vmbits", memory.vmbits);
			} else if (reader.name() == "network") {
				readAttribute(attributes, "interface", network.interface);
			} else {
				logger.error("Unexpected name %s!", reader.name().toLocal8Bit().constData());
				ERROR();
			}
			reader.skipCurrentElement();
		}

//		logger.info("name %s", name.toLocal8Bit().constData());
//		logger.info("display %d %d", display.width, display.height);
//		logger.info("file %s  %s  %s  %s",
//			file.boot.toLocal8Bit().constData(), file.disk.toLocal8Bit().constData(), file.germ.toLocal8Bit().constData(), file.floppy.toLocal8Bit().constData());
//		logger.info("boot %s  %s", boot.device.toLocal8Bit().constData(), boot.switchString.toLocal8Bit().constData());
//		logger.info("memory %d %d", memory.rmbits, memory.vmbits);
//		logger.info("network %s", network.interface.toLocal8Bit().constData());
	} else {
		logger.error("Unexpected name %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}
}

void initialize() {
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
		if (reader.name() == "preference") {
			while(reader.readNextStartElement()) {
				if (reader.name() == "entry") {
					Preference preference(reader);
					map[preference.name] = preference;
					logger.info("preference %s", preference.name.toLocal8Bit().constData());
				} else {
					logger.error("Unexpected token %s!", reader.tokenString().toLocal8Bit().constData());
					ERROR();
				}
			}
		} else {
			logger.error("Unexpected token %s!", reader.tokenString().toLocal8Bit().constData());
			ERROR();
		}
	} else {
		logger.error("Unexpected token %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	// Close files
	file.close();
}

Preference Preference::getInstance(QString name) {
	if (map.isEmpty()) {
		initialize();
	}

	if (map.contains(name)) {
		return map[name];
	} else {
		logger.error("Unexpected name %s", name.toLocal8Bit().constData());
		ERROR();
	}
}

