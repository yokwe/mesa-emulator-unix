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


static const char* PATH_FILE = "tmp/run/setting.json";


// Setting::Entry::Display
void Setting::Entry::Display::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(width);
	GET_JSON_OBJECT(height);
}
QJsonObject Setting::Entry::Display::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(width);
	SET_JSON_OBJECT(height);
	return jsonObject;
}

// Setting::Entry::File
void Setting::Entry::File::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(disk);
	GET_JSON_OBJECT(germ);
	GET_JSON_OBJECT(boot);
	GET_JSON_OBJECT(floppy);
}
QJsonObject Setting::Entry::File::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(disk);
	SET_JSON_OBJECT(germ);
	SET_JSON_OBJECT(boot);
	SET_JSON_OBJECT(floppy);
	return jsonObject;
}

// Setting::Entry::Boot
void Setting::Entry::Boot::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT2(switch, switch_);
	GET_JSON_OBJECT(device);
}
QJsonObject Setting::Entry::Boot::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT2(switch, switch_);
	SET_JSON_OBJECT(device);
	return jsonObject;
}

// Setting::Entry::Memory
void Setting::Entry::Memory::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(vmbits);
	GET_JSON_OBJECT(rmbits);
}
QJsonObject Setting::Entry::Memory::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(vmbits);
	SET_JSON_OBJECT(rmbits);
	return jsonObject;
}

// Setting::Entry::Network
void Setting::Entry::Network::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(interface);
}
QJsonObject Setting::Entry::Network::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(interface);
	return jsonObject;
}

// Setting::Entry
void Setting::Entry::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT(display);
	GET_JSON_OBJECT(file);
	GET_JSON_OBJECT(boot);
	GET_JSON_OBJECT(memory);
	GET_JSON_OBJECT(network);
}
QJsonObject Setting::Entry::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT(display);
	SET_JSON_OBJECT(file);
	SET_JSON_OBJECT(boot);
	SET_JSON_OBJECT(memory);
	SET_JSON_OBJECT(network);
	return jsonObject;
}

// Setting::LevelVKeys
void Setting::LevelVKeys::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT(keyName);
}
QJsonObject Setting::LevelVKeys::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT(keyName);
	return jsonObject;
}

// Setting::Keyboard
void Setting::Keyboard::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT(scanCode);
}
QJsonObject Setting::Keyboard::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT(scanCode);
	return jsonObject;
}

// Setting::KeyMap
void Setting::KeyMap::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(levelVKeys);
	GET_JSON_OBJECT(keyboard);
}
QJsonObject Setting::KeyMap::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(levelVKeys);
	SET_JSON_OBJECT(keyboard);
	return jsonObject;
}

// Setting::Mouse
void Setting::Mouse::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(name);
	GET_JSON_OBJECT(bitMask);
}
QJsonObject Setting::Mouse::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(name);
	SET_JSON_OBJECT(bitMask);
	return jsonObject;
}

// Setting::ButtonMap
void Setting::ButtonMap::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(levelVKeys);
	GET_JSON_OBJECT(button);
}
QJsonObject Setting::ButtonMap::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(levelVKeys);
	SET_JSON_OBJECT(button);
	return jsonObject;
}


// Setting
void Setting::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT2(entry,      entryList);
	GET_JSON_OBJECT2(levelVKeys, levelVKeysList);
	GET_JSON_OBJECT2(keyboard,   keyboardList);
	GET_JSON_OBJECT2(keyMap,     keyMapList);
	GET_JSON_OBJECT2(mouse,      mouseList);
	GET_JSON_OBJECT2(buttonMap,  buttonMapList);
}
QJsonObject Setting::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT2(entry,      entryList);
	SET_JSON_OBJECT2(levelVKeys, levelVKeysList);
	SET_JSON_OBJECT2(keyboard,   keyboardList);
	SET_JSON_OBJECT2(keyMap,     keyMapList);
	SET_JSON_OBJECT2(mouse,      mouseList);
	SET_JSON_OBJECT2(buttonMap,  buttonMapList);
	return jsonObject;
}

Setting Setting::getInstance() {
	logger.info("path       %s", PATH_FILE);

	// sanity check
	{
		if (!QFile::exists(PATH_FILE)) {
			logger.fatal("file does not exist");
			logger.fatal("  path = %s!", PATH_FILE);
			ERROR();
		}
	}

	Setting setting;
	{
		QJsonObject jsonObject = JSONUtil::loadObject(PATH_FILE);
		setting.fromJsonObject(jsonObject);
	}

	logger.info("entryList      %3d", setting.entryList.size());
	logger.info("levelVKeysList %3d", setting.levelVKeysList.size());
	logger.info("keyboardList   %3d", setting.keyboardList.size());
	logger.info("keyMapList     %3d", setting.keyMapList.size());
	logger.info("mouseList      %3d", setting.mouseList.size());
	logger.info("buttonMapList  %3d", setting.buttonMapList.size());

	initMap(setting);

	return setting;
}

Setting::Entry Setting::getEntry(QString name) {
	for(Setting::Entry e: entryList) {
		if (e.name == name) {
			return e;
		}
	}
	logger.fatal("Unexpected");
	logger.fatal("  name = %s!", toCString(name));
	ERROR();
}


QHash<quint32,         quint32>        Setting::keyMap;
//    scanCode         keyName
QHash<Qt::MouseButton, quint32>        Setting::buttonMap;
//    Qt::MouseButton  keyName

void Setting::initMap(const Setting& setting) {
	// sanity check
	{
		bool foundError = false;

		// levelVKeys
		{
			// check duplicate of levelVKeys keyname
			QMap<int, QString> map;
			for(auto e: setting.levelVKeysList) {
				if (map.contains(e.keyName)) {
					logger.error("levelVkeys duplicate keyName %3d  %s  %ss", e.keyName, toCString(e.name), toCString(map[e.keyName]));
					foundError = true;
				} else {
					map[e.keyName] = e.name;
				}
			}
		}
		{
			// check duplicate of levelVKeys name
			QMap<QString, QString> map;
			for(auto e: setting.levelVKeysList) {
				if (map.contains(e.name)) {
					logger.error("levelVkeys duplicate name %s  %3d  %s", toCString(e.name), e.keyName, toCString(map[e.name]));
					foundError = true;
				} else {
					map[e.name] = e.keyName;
				}
			}
		}

		// keyboard
		{
			// check duplicate of keyboard scanCode
			QMap<int, QString> map;
			for(auto e: setting.keyboardList) {
				if (map.contains(e.scanCode)) {
					logger.error("keyboard duplicate scanCode %3d  %s  %s", e.scanCode, toCString(e.name), toCString(map[e.scanCode]));
					foundError = true;
				} else {
					map[e.scanCode] = e.name;
				}
			}
		}
		{
			// check duplicate of keyboard name
			QMap<QString, QString> map;
			for(auto e: setting.keyboardList) {
				if (map.contains(e.name)) {
					logger.error("keyboard duplicate name %s  %3d  %s", toCString(e.name), e.scanCode, toCString(map[e.name]));
					foundError = true;
				} else {
					map[e.name] = e.scanCode;
				}
			}
		}

		// keyMap
		{
			// check duplicate of keyMap keyboard
			QMap<QString, QString> map;
			for(auto e: setting.keyMapList) {
				if (e.keyboard.isEmpty()) continue;

				if (map.contains(e.keyboard)) {
					logger.error("keyMap duplicate keyboard %s  %s  %s", toCString(e.keyboard), toCString(e.levelVKeys), toCString(map[e.keyboard]));
					foundError = true;
				} else {
					map[e.keyboard] = e.levelVKeys;
				}
			}
		}
		{
			// check duplicate of keyMap levelVKeys
			QMap<QString, QString> map;
			for(auto e: setting.keyMapList) {
				if (e.keyboard.isEmpty()) continue;

				if (map.contains(e.levelVKeys)) {
					logger.error("keyMap duplicate levelVKeys %s  %s  %s", toCString(e.levelVKeys), toCString(e.keyboard), toCString(map[e.keyboard]));
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

	// build keyMap and buttonMap
	{
		keyMap.clear();
		buttonMap.clear();

		// build Setting::keyMap
		QMap<QString, quint32> nameToScanCode;
		for(auto e: setting.keyboardList) {
			nameToScanCode[e.name] = e.scanCode;
		}
		QMap<QString, quint32> nameToKeyName;
		for(auto e: setting.levelVKeysList) {
			nameToKeyName[e.name] = e.keyName;
		}

		for(auto e: setting.keyMapList) {
			if (e.keyboard.isEmpty()) continue;

			quint32 scanCode = nameToScanCode[e.keyboard];
			quint32 keyName  = nameToKeyName[e.levelVKeys];

			Setting::keyMap[scanCode] = keyName;

//			logger.info("keyMap    %-16s %02X => %-16s %3d", toCString(e.keyboard), scanCode, toCString(e.levelVKeys), keyName);
		}

		// build Setting::buttonMap
		QMap<QString, quint32> nameToBitmask;
		for(auto e: setting.mouseList) {
			nameToBitmask[e.name] = e.bitMask;
		}

		for(auto e: setting.buttonMapList) {
			Qt::MouseButton bitMask = (Qt::MouseButton)nameToBitmask[e.button];
			quint32         keyName = nameToKeyName[e.levelVKeys];

			Setting::buttonMap[bitMask] = keyName;

//			logger.info("buttonMap %-16s %02X => %-16s %3d", toCString(e.button), bitMask, toCString(e.levelVKeys), keyName);
		}
	}

	logger.info("keyMap         %3d", Setting::keyMap.size());
	logger.info("buttonMap      %3d", Setting::buttonMap.size());
}
