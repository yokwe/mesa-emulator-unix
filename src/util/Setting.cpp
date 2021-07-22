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

static const QString PATH_FILE = QStringLiteral("data/Guam/setting.xml");

static const QString XML_SETTING    = QStringLiteral("setting");

static const QString XML_DISPLAY    = QStringLiteral("display");
static const QString XML_WIDTH      = QStringLiteral("width");
static const QString XML_HEIGHT     = QStringLiteral("height");

static const QString XML_FILE       = QStringLiteral("file");
static const QString XML_DISK       = QStringLiteral("disk");
static const QString XML_GERM       = QStringLiteral("germ");
static const QString XML_BOOT       = QStringLiteral("boot");
static const QString XML_FLOPPY     = QStringLiteral("floppy");

//const char* XML_BOOT   = QStringLiteral("boot");
static const QString XML_SWITCH     = QStringLiteral("switch");
static const QString XML_DEVICE     = QStringLiteral("device");

static const QString XML_MEMORY     = QStringLiteral("memory");
static const QString XML_VMBITS     = QStringLiteral("vmbits");
static const QString XML_RMBITS     = QStringLiteral("rmbits");

static const QString XML_NETWORK    = QStringLiteral("network");
static const QString XML_INTERFACE  = QStringLiteral("interface");

static const QString XML_ENTRY      = QStringLiteral("entry");
static const QString XML_NAME       = QStringLiteral("name");

static const QString XML_LEVELVKEYS = QStringLiteral("levelVKeys");
static const QString XML_KEYBOARD   = QStringLiteral("keyboard");
static const QString XML_KEYMAP     = QStringLiteral("keyMap");
static const QString XML_MOUSE      = QStringLiteral("mouse");
static const QString XML_BUTTONMAP  = QStringLiteral("buttonMap");
static const QString XML_KEY        = QStringLiteral("key");
static const QString XML_KEYNAME    = QStringLiteral("keyName");
static const QString XML_SCANCODE   = QStringLiteral("scanCode");
static const QString XML_BUTTON     = QStringLiteral("button");
static const QString XML_BITMASK    = QStringLiteral("bitMask");


static void readAttribute(QXmlStreamAttributes& attributes, QString name, QString& value) {
	QStringLiteral("a");

	if (attributes.hasAttribute(name)) {
		value = attributes.value(name).toString();
	} else {
		logger.error("Unexpected no attribute %s!", name.toLocal8Bit().constData());
		ERROR();
	}
}

static quint32 toInt(QString string) {
	bool ok;
	quint32 ret = string.toInt(&ok, 0); // to handle string starts with 0x, use 0 for base

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


class LevelVKeys {
public:
	class Key {
	public:
		QString name;
		quint32 keyName;

		Key() : name(""), keyName(0) {}
		Key(QXmlStreamReader& reader);
	};

	QList<Key> keyList;

	LevelVKeys() {}
	LevelVKeys(QXmlStreamReader& reader);
};
LevelVKeys::Key::Key(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_KEY) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_NAME,    name);
	readAttribute(attributes, XML_KEYNAME, keyName);
	reader.skipCurrentElement();
}
LevelVKeys::LevelVKeys(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_LEVELVKEYS) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	while(reader.readNextStartElement()) {
		Key key(reader);
		keyList.append(key);
	}
}


class Keyboard {
public:
	class Key {
	public:
		QString name;
		quint32 scanCode;

		Key() : name(""), scanCode(0) {}
		Key(QXmlStreamReader& reader);
	};

	QList<Key> keyList;
	Keyboard() {}
	Keyboard(QXmlStreamReader& reader);
};
Keyboard::Key::Key(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_KEY) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_NAME,     name);
	readAttribute(attributes, XML_SCANCODE, scanCode);
	reader.skipCurrentElement();
}
Keyboard::Keyboard(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_KEYBOARD) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	while(reader.readNextStartElement()) {
		Key key(reader);
		keyList.append(key);
	}
}


class KeyMap {
public:
	class Key {
	public:
		QString levelVKeys;
		QString keyboard;

		Key() : levelVKeys(""), keyboard("") {}
		Key(QXmlStreamReader& reader);
	};

	QList<Key> keyList;

	KeyMap() {}
	KeyMap(QXmlStreamReader& reader);
};
KeyMap::Key::Key(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_KEY) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_LEVELVKEYS, levelVKeys);
	readAttribute(attributes, XML_KEYBOARD,   keyboard);
	reader.skipCurrentElement();
}
KeyMap::KeyMap(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_KEYMAP) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	while(reader.readNextStartElement()) {
		Key key(reader);
		keyList.append(key);
	}
}


class Mouse {
public:
	class Button {
	public:
		QString name;
		quint32 bitMask;

		Button() : name(""), bitMask(0) {}
		Button(QXmlStreamReader& reader);
	};

	QList<Button> buttonList;

	Mouse() {}
	Mouse(QXmlStreamReader& reader);
};
Mouse::Button::Button(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_BUTTON) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_NAME,    name);
	readAttribute(attributes, XML_BITMASK, bitMask);
	reader.skipCurrentElement();
}
Mouse::Mouse(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_MOUSE) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	while(reader.readNextStartElement()) {
		Button button(reader);
		buttonList.append(button);
	}
}


class ButtonMap {
public:
	class Button {
	public:
		QString levelVKeys;
		QString button;

		Button() : levelVKeys(""), button("") {}
		Button(QXmlStreamReader& reader);
	};

	QList<Button> buttonList;

	ButtonMap() {}
	ButtonMap(QXmlStreamReader& reader);
};
ButtonMap::Button::Button(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_BUTTON) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_LEVELVKEYS, levelVKeys);
	readAttribute(attributes, XML_BUTTON,     button);
	reader.skipCurrentElement();
}
ButtonMap::ButtonMap(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_BUTTONMAP) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	while(reader.readNextStartElement()) {
		Button button(reader);
		buttonList.append(button);
	}
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


static void initialize() {
	QString path(PATH_FILE);

	QFile file(path);
	if (!file.exists()) {
		logger.error("Unexpected path %s", path.toLocal8Bit().constData());
		ERROR();
	}

	// For keyMap and buttonMap
	LevelVKeys levelVKeys;
	Keyboard   keyboard;
	KeyMap     keyMap;
	Mouse      mouse;
	ButtonMap  buttonMap;

	// Open file
	file.open(QFile::ReadOnly);
	QXmlStreamReader reader(&file);

	if (reader.readNextStartElement()) {
		// sanity check
		if (reader.name() != XML_SETTING) {
			logger.error("Unexpected %s!", reader.name().toLocal8Bit().constData());
			ERROR();
		}

		while(reader.readNextStartElement()) {
			if (reader.name() == XML_ENTRY) {
				Setting::Entry entry(reader);
				Setting::entryMap[entry.name] = entry;

				logger.info("entry %s", entry.name.toLocal8Bit().constData());
			} else if (reader.name() == XML_LEVELVKEYS) {
				LevelVKeys newValue(reader);
				levelVKeys = newValue;
			} else if (reader.name() == XML_KEYBOARD) {
				Keyboard newValue(reader);
				keyboard = newValue;
			} else if (reader.name() == XML_KEYMAP) {
				KeyMap newValue(reader);
				keyMap = newValue;
			} else if (reader.name() == XML_MOUSE) {
				Mouse newValue(reader);
				mouse = newValue;
			} else if (reader.name() == XML_BUTTONMAP) {
				ButtonMap newValue(reader);
				buttonMap = newValue;
			} else {
				logger.warn("Unexpected %s!", reader.name().toLocal8Bit().constData());
				reader.skipCurrentElement();
			}
		}
	} else {
		logger.error("Unexpected token %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	// Close files
	file.close();


	// sanity check
	{
		bool foundError = false;

		// levelVKeys
		{
			QMap<quint32, QString> map;
			for(auto e: levelVKeys.keyList) {
				if (map.contains(e.keyName)) {
					logger.error("levelVkeys duplicate keyName %d  %s  %s", e.keyName, e.name.toLocal8Bit().constData(), map[e.keyName].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.keyName] = e.name;
				}
			}
		}
		{
			QMap<QString, quint32> map;
			for(auto e: levelVKeys.keyList) {
				if (map.contains(e.name)) {
					logger.error("levelVkeys duplicate name %s  %d  %d", e.name.toLocal8Bit().constData(), e.keyName, map[e.name]);
					foundError = true;
				} else {
					map[e.name] = e.keyName;
				}
			}
		}

		// keyboard
		{
			QMap<quint32, QString> map;
			for(auto e: keyboard.keyList) {
				if (map.contains(e.scanCode)) {
					logger.error("keyboard duplicate scanCode %d  %s  %s", e.scanCode, e.name.toLocal8Bit().constData(), map[e.scanCode].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.scanCode] = e.name;
				}
			}
		}
		{
			QMap<QString, quint32> map;
			for(auto e: keyboard.keyList) {
				if (map.contains(e.name)) {
					logger.error("keyboard duplicate name %s  %d  %d", e.name.toLocal8Bit().constData(), e.scanCode, map[e.name]);
					foundError = true;
				} else {
					map[e.name] = e.scanCode;
				}
			}
		}

		// keyMap
		{
			QMap<QString, QString> map;
			for(auto e: keyMap.keyList) {
				if (e.keyboard.isEmpty()) continue;

				if (map.contains(e.keyboard)) {
					logger.error("keyMap duplicate keyboard %s  %s  %s", e.keyboard.toLocal8Bit().constData(), e.levelVKeys.toLocal8Bit().constData(), map[e.keyboard].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.keyboard] = e.levelVKeys;
				}
			}
		}
		{
			QMap<QString, QString> map;
			for(auto e: keyMap.keyList) {
				if (e.keyboard.isEmpty()) continue;

				if (map.contains(e.levelVKeys)) {
					logger.error("keyMap duplicate levelVKeys %s  %s  %s", e.levelVKeys.toLocal8Bit().constData(), e.keyboard.toLocal8Bit().constData(), map[e.keyboard].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.levelVKeys] = e.keyboard;
				}
			}
		}

		if (foundError) {
			ERROR();
		}
	}

	{
		// build Setting::keyMap
		QMap<QString, quint32> nameToScanCode;
		for(auto e: keyboard.keyList) {
			nameToScanCode[e.name] = e.scanCode;
		}
		QMap<QString, quint32> nameToKeyName;
		for(auto e: levelVKeys.keyList) {
			nameToKeyName[e.name] = e.keyName;
		}

		for(auto e: keyMap.keyList) {
			if (e.keyboard.isEmpty()) continue;

			quint32 scanCode = nameToScanCode[e.keyboard];
			quint32 keyName  = nameToKeyName[e.levelVKeys];

			Setting::keyMap[scanCode] = keyName;

//				logger.info("keyMap    %-16s %02X => %-16s %3d", e.keyboard.toLocal8Bit().constData(), scanCode, e.levelVKeys.toLocal8Bit().constData(), keyName);
		}

		// build Setting::buttonMap
		QMap<QString, quint32> nameToBitmask;
		for(auto e: mouse.buttonList) {
			nameToBitmask[e.name] = e.bitMask;
		}

		for(auto e: buttonMap.buttonList) {
			Qt::MouseButton bitMask = (Qt::MouseButton)nameToBitmask[e.button];
			quint32         keyName = nameToKeyName[e.levelVKeys];

			Setting::buttonMap[bitMask] = keyName;

//				logger.info("buttonMap %-16s %02X => %-16s %3d", e.button.toLocal8Bit().constData(), bitMask, e.levelVKeys.toLocal8Bit().constData(), keyName);
		}
	}

	logger.info("entry      %3d", Setting::entryMap.size());
	logger.info("keyMap     %3d", Setting::keyMap.size());
	logger.info("buttonMap  %3d", Setting::buttonMap.size());

//	logger.info("levelVKeys %3d", levelVKeys.keyList.size());
//	logger.info("keyboard   %3d", keyboard.keyList.size());
//	logger.info("keyMap     %3d", keyMap.keyList.size());
//	logger.info("mouse      %3d", mouse.buttonList.size());
//	logger.info("buttonMap  %3d", buttonMap.buttonList.size());

}

QMap<QString,          Setting::Entry> Setting::entryMap;
QHash<quint32,         quint32>        Setting::keyMap;
//    scanCode         keyName
QHash<Qt::MouseButton, quint32>        Setting::buttonMap;
//    Qt::MouseButton  keyName

Setting::Entry Setting::getInstance(QString name) {
	if (entryMap.isEmpty()) {
		initialize();
	}

	if (entryMap.contains(name)) {
		return entryMap[name];
	} else {
		logger.error("Unexpected name %s", name.toLocal8Bit().constData());
		ERROR();
	}
}

