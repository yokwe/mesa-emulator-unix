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


#include "Util.h"
static log4cpp::Category& logger = Logger::getLogger("setting");

#include "Setting.h"


static const QString PATH_FILE = QStringLiteral("data/setting.json");

static const QString JSON_NAME       = QStringLiteral("name");
static const QString JSON_KEYNAME    = QStringLiteral("keyName");
static const QString JSON_SCANCODE   = QStringLiteral("scanCode");
static const QString JSON_LEVELVKEYS = QStringLiteral("levelVKeys");
static const QString JSON_KEYBOARD   = QStringLiteral("keyboard");
static const QString JSON_KEYMAP     = QStringLiteral("keyMap");
static const QString JSON_BITMASK    = QStringLiteral("bitMask");
static const QString JSON_MOUSE      = QStringLiteral("mouse");
static const QString JSON_BUTTON     = QStringLiteral("button");
static const QString JSON_BUTTONMAP  = QStringLiteral("buttonMap");
static const QString JSON_WIDTH      = QStringLiteral("width");
static const QString JSON_HEIGHT     = QStringLiteral("height");
static const QString JSON_DISK       = QStringLiteral("disk");
static const QString JSON_GERM       = QStringLiteral("germ");
static const QString JSON_BOOT       = QStringLiteral("boot");
static const QString JSON_FLOPPY     = QStringLiteral("floppy");
static const QString JSON_SWITCH     = QStringLiteral("switch");
static const QString JSON_DEVICE     = QStringLiteral("device");
static const QString JSON_VMBITS     = QStringLiteral("vmbits");
static const QString JSON_RMBITS     = QStringLiteral("rmbits");
static const QString JSON_INTERFACE  = QStringLiteral("interface");
static const QString JSON_ENTRY      = QStringLiteral("entry");
static const QString JSON_DISPLAY    = QStringLiteral("display");
static const QString JSON_FILE       = QStringLiteral("file");
static const QString JSON_MEMORY     = QStringLiteral("memory");
static const QString JSON_NETWORK    = QStringLiteral("network");


// Setting::Entry::Display
void Setting::Entry::Display::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(width, jsonObject,  JSON_WIDTH);
	JSONUtil::set(height, jsonObject, JSON_HEIGHT);
}
QJsonObject Setting::Entry::Display::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_WIDTH,  width);
	JSONUtil::set(target, JSON_HEIGHT, height);
	return target;
}

// Setting::Entry::File
void Setting::Entry::File::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(disk,   jsonObject, JSON_DISK);
	JSONUtil::set(germ,   jsonObject, JSON_GERM);
	JSONUtil::set(boot,   jsonObject, JSON_BOOT);
	JSONUtil::set(floppy, jsonObject, JSON_FLOPPY);
}
QJsonObject Setting::Entry::File::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_DISK,   disk);
	JSONUtil::set(target, JSON_GERM,   germ);
	JSONUtil::set(target, JSON_BOOT,   boot);
	JSONUtil::set(target, JSON_FLOPPY, floppy);
	return target;
}

// Setting::Entry::Boot
void Setting::Entry::Boot::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(switch_, jsonObject, JSON_SWITCH);
	JSONUtil::set(device,  jsonObject, JSON_DEVICE);
}
QJsonObject Setting::Entry::Boot::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_SWITCH, switch_);
	JSONUtil::set(target, JSON_DEVICE, device);
	return target;
}

// Setting::Entry::Memory
void Setting::Entry::Memory::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(vmbits, jsonObject, JSON_VMBITS);
	JSONUtil::set(rmbits, jsonObject, JSON_RMBITS);
}
QJsonObject Setting::Entry::Memory::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_VMBITS, vmbits);
	JSONUtil::set(target, JSON_RMBITS, rmbits);
	return target;
}

// Setting::Entry::Network
void Setting::Entry::Network::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(interface, jsonObject, JSON_INTERFACE);
}
QJsonObject Setting::Entry::Network::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_INTERFACE, interface);
	return target;
}

// Setting::Entry
void Setting::Entry::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(name,    jsonObject, JSON_NAME);
	JSONUtil::set(display, jsonObject, JSON_DISPLAY);
	JSONUtil::set(file,    jsonObject, JSON_FILE);
	JSONUtil::set(boot,    jsonObject, JSON_BOOT);
	JSONUtil::set(memory,  jsonObject, JSON_MEMORY);
	JSONUtil::set(network, jsonObject, JSON_NETWORK);
}
QJsonObject Setting::Entry::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_NAME,    name);
	JSONUtil::set(target, JSON_DISPLAY, display);
	JSONUtil::set(target, JSON_FILE,    file);
	JSONUtil::set(target, JSON_BOOT,    boot);
	JSONUtil::set(target, JSON_MEMORY,  memory);
	JSONUtil::set(target, JSON_NETWORK, network);
	return target;
}

// Setting::LevelVKeys
void Setting::LevelVKeys::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(name,    jsonObject, JSON_NAME);
	JSONUtil::set(keyName, jsonObject, JSON_KEYNAME);
}
QJsonObject Setting::LevelVKeys::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_NAME,    name);
	JSONUtil::set(target, JSON_KEYNAME, keyName);
	return target;
}

// Setting::Keyboard
void Setting::Keyboard::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(name,     jsonObject, JSON_NAME);
	JSONUtil::set(scanCode, jsonObject, JSON_SCANCODE);
}
QJsonObject Setting::Keyboard::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_NAME,     name);
	JSONUtil::set(target, JSON_SCANCODE, scanCode);
	return target;
}

// Setting::KeyMap
void Setting::KeyMap::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(levelVKeys, jsonObject, JSON_LEVELVKEYS);
	JSONUtil::set(keyboard,   jsonObject, JSON_KEYBOARD);
}
QJsonObject Setting::KeyMap::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_LEVELVKEYS, levelVKeys);
	JSONUtil::set(target, JSON_KEYBOARD,   keyboard);
	return target;
}

// Setting::Mouse
void Setting::Mouse::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(name,    jsonObject, JSON_NAME);
	JSONUtil::set(bitMask, jsonObject, JSON_BITMASK);
}
QJsonObject Setting::Mouse::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_NAME,    name);
	JSONUtil::set(target, JSON_BITMASK, bitMask);
	return target;
}

// Setting::ButtonMap
void Setting::ButtonMap::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::set(levelVKeys, jsonObject, JSON_LEVELVKEYS);
	JSONUtil::set(button,     jsonObject, JSON_BUTTON);
}
QJsonObject Setting::ButtonMap::toJsonObject() const {
	QJsonObject target;
	JSONUtil::set(target, JSON_LEVELVKEYS,    levelVKeys);
	JSONUtil::set(target, JSON_BUTTON, button);
	return target;
}

static void initialize() {
	QByteArray fileContents;

	{
		QFile file(PATH_FILE);
		if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
			logger.fatal("File open error %s", file.errorString().toLocal8Bit().constData());
			ERROR();
		}
		fileContents = file.readAll();
		file.close();
	}
	logger.info("fileContents = %d", fileContents.size());

	QList<Setting::Entry>      entryList;
	QList<Setting::LevelVKeys> levelVKeysList;
	QList<Setting::Keyboard>   keyboardList;
	QList<Setting::KeyMap>     keyMapList;
	QList<Setting::Mouse>      mouseList;
	QList<Setting::ButtonMap>  buttonMapList;

	{
		QJsonParseError jsonParseError;

		DEBUG_TRACE();
		QJsonDocument jsonDocument = QJsonDocument::fromJson(fileContents, &jsonParseError);
		DEBUG_TRACE();
		if (jsonParseError.error != QJsonParseError::NoError) {
			logger.error("Json Parse error");
			logger.error("  errorString  = %s", jsonParseError.errorString().toLocal8Bit().constData());
			logger.error("  offset       = %d", jsonParseError.offset);
			logger.error("  jsonDocument = %s!", jsonDocument.toJson(QJsonDocument::JsonFormat::Indented).constData());
			ERROR();
		}
		DEBUG_TRACE();
		QJsonObject jsonObject = jsonDocument.object();
		DEBUG_TRACE();

		JSONUtil::set(entryList,      jsonObject, JSON_ENTRY);
		JSONUtil::set(levelVKeysList, jsonObject, JSON_LEVELVKEYS);
		JSONUtil::set(keyboardList,   jsonObject, JSON_KEYBOARD);
		JSONUtil::set(keyMapList,     jsonObject, JSON_KEYMAP);
		JSONUtil::set(mouseList,      jsonObject, JSON_MOUSE);
		JSONUtil::set(buttonMapList,  jsonObject, JSON_BUTTONMAP);

		logger.info("entryList      %3d", entryList.size());
		logger.info("levelVKeysList %3d", levelVKeysList.size());
		logger.info("keyboardList   %3d", keyboardList.size());
		logger.info("keyMapList     %3d", keyMapList.size());
		logger.info("mouseList      %3d", mouseList.size());
		logger.info("buttonMapList  %3d", buttonMapList.size());
	}

	// sanity check
	{
		bool foundError = false;

		// levelVKeys
		{
			QMap<QString, QString> map;
			for(auto e: levelVKeysList) {
				if (map.contains(e.keyName)) {
					logger.error("levelVkeys duplicate keyName %d  %s  %s", e.keyName.toLocal8Bit().constData(), e.name.toLocal8Bit().constData(), map[e.keyName].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.keyName] = e.name;
				}
			}
		}
		{
			QMap<QString, QString> map;
			for(auto e: levelVKeysList) {
				if (map.contains(e.name)) {
					logger.error("levelVkeys duplicate name %s  %s  %s", e.name.toLocal8Bit().constData(), e.keyName.toLocal8Bit().constData(), map[e.name].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.name] = e.keyName;
				}
			}
		}

		// keyboard
		{
			QMap<QString, QString> map;
			for(auto e: keyboardList) {
				if (map.contains(e.scanCode)) {
					logger.error("keyboard duplicate scanCode %s  %s  %s", e.scanCode.toLocal8Bit().constData(), e.name.toLocal8Bit().constData(), map[e.scanCode].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.scanCode] = e.name;
				}
			}
		}
		{
			QMap<QString, QString> map;
			for(auto e: keyboardList) {
				if (map.contains(e.name)) {
					logger.error("keyboard duplicate name %s  %s  %s", e.name.toLocal8Bit().constData(), e.scanCode.toLocal8Bit().constData(), map[e.name].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.name] = e.scanCode;
				}
			}
		}

		// keyMap
		{
			QMap<QString, QString> map;
			for(auto e: keyMapList) {
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
			for(auto e: keyMapList) {
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

//	{
//		// build Setting::keyMap
//		QMap<QString, quint32> nameToScanCode;
//		for(auto e: keyboard.keyList) {
//			nameToScanCode[e.name] = e.scanCode;
//		}
//		QMap<QString, quint32> nameToKeyName;
//		for(auto e: levelVKeys.keyList) {
//			nameToKeyName[e.name] = e.keyName;
//		}
//
//		for(auto e: keyMap.keyList) {
//			if (e.keyboard.isEmpty()) continue;
//
//			quint32 scanCode = nameToScanCode[e.keyboard];
//			quint32 keyName  = nameToKeyName[e.levelVKeys];
//
//			Setting::keyMap[scanCode] = keyName;
//
////				logger.info("keyMap    %-16s %02X => %-16s %3d", e.keyboard.toLocal8Bit().constData(), scanCode, e.levelVKeys.toLocal8Bit().constData(), keyName);
//		}
//
//		// build Setting::buttonMap
//		QMap<QString, quint32> nameToBitmask;
//		for(auto e: mouse.buttonList) {
//			nameToBitmask[e.name] = e.bitMask;
//		}
//
//		for(auto e: buttonMap.buttonList) {
//			Qt::MouseButton bitMask = (Qt::MouseButton)nameToBitmask[e.button];
//			quint32         keyName = nameToKeyName[e.levelVKeys];
//
//			Setting::buttonMap[bitMask] = keyName;
//
////				logger.info("buttonMap %-16s %02X => %-16s %3d", e.button.toLocal8Bit().constData(), bitMask, e.levelVKeys.toLocal8Bit().constData(), keyName);
//		}
//	}

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
QHash<int,             int>            Setting::keyMap;
//    scanCode         keyName
QHash<Qt::MouseButton, int>            Setting::buttonMap;
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

