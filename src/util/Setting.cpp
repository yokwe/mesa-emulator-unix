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
	JSONUtil::getJsonObject(jsonObject, JSON_WIDTH,  width);
	JSONUtil::getJsonObject(jsonObject, JSON_HEIGHT, height);
}
QJsonObject Setting::Entry::Display::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_WIDTH,  width);
	JSONUtil::setJsonObject(jsonObject, JSON_HEIGHT, height);
	return jsonObject;
}

// Setting::Entry::File
void Setting::Entry::File::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_DISK,   disk);
	JSONUtil::getJsonObject(jsonObject, JSON_GERM,   germ);
	JSONUtil::getJsonObject(jsonObject, JSON_BOOT,   boot);
	JSONUtil::getJsonObject(jsonObject, JSON_FLOPPY, floppy);
}
QJsonObject Setting::Entry::File::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_DISK,   disk);
	JSONUtil::setJsonObject(jsonObject, JSON_GERM,   germ);
	JSONUtil::setJsonObject(jsonObject, JSON_BOOT,   boot);
	JSONUtil::setJsonObject(jsonObject, JSON_FLOPPY, floppy);
	return jsonObject;
}

// Setting::Entry::Boot
void Setting::Entry::Boot::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_SWITCH, switch_);
	JSONUtil::getJsonObject(jsonObject, JSON_DEVICE, device);
}
QJsonObject Setting::Entry::Boot::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_SWITCH, switch_);
	JSONUtil::setJsonObject(jsonObject, JSON_DEVICE, device);
	return jsonObject;
}

// Setting::Entry::Memory
void Setting::Entry::Memory::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_VMBITS, vmbits);
	JSONUtil::getJsonObject(jsonObject, JSON_RMBITS, rmbits);}
QJsonObject Setting::Entry::Memory::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_VMBITS, vmbits);
	JSONUtil::setJsonObject(jsonObject, JSON_RMBITS, rmbits);
	return jsonObject;
}

// Setting::Entry::Network
void Setting::Entry::Network::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_INTERFACE, interface);
}
QJsonObject Setting::Entry::Network::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_INTERFACE, interface);
	return jsonObject;
}

// Setting::Entry
void Setting::Entry::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_NAME,    name);
	JSONUtil::getJsonObject(jsonObject, JSON_DISPLAY, display);
	JSONUtil::getJsonObject(jsonObject, JSON_FILE,    file);
	JSONUtil::getJsonObject(jsonObject, JSON_BOOT,    boot);
	JSONUtil::getJsonObject(jsonObject, JSON_MEMORY,  memory);
	JSONUtil::getJsonObject(jsonObject, JSON_NETWORK, network);
}
QJsonObject Setting::Entry::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_NAME,    name);
	JSONUtil::setJsonObject(jsonObject, JSON_DISPLAY, display);
	JSONUtil::setJsonObject(jsonObject, JSON_FILE,    file);
	JSONUtil::setJsonObject(jsonObject, JSON_BOOT,    boot);
	JSONUtil::setJsonObject(jsonObject, JSON_MEMORY,  memory);
	JSONUtil::setJsonObject(jsonObject, JSON_NETWORK, network);
	return jsonObject;
}

// Setting::LevelVKeys
void Setting::LevelVKeys::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_NAME,    name);
	JSONUtil::getJsonObject(jsonObject, JSON_KEYNAME, keyName);
}
QJsonObject Setting::LevelVKeys::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_NAME,    name);
	JSONUtil::setJsonObject(jsonObject, JSON_KEYNAME, keyName);
	return jsonObject;
}

// Setting::Keyboard
void Setting::Keyboard::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_NAME,     name);
	JSONUtil::getJsonObject(jsonObject, JSON_SCANCODE, scanCode);
}
QJsonObject Setting::Keyboard::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_NAME,     name);
	JSONUtil::setJsonObject(jsonObject, JSON_SCANCODE, scanCode);
	return jsonObject;
}

// Setting::KeyMap
void Setting::KeyMap::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_LEVELVKEYS, levelVKeys);
	JSONUtil::getJsonObject(jsonObject, JSON_KEYBOARD,   keyboard);
}
QJsonObject Setting::KeyMap::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_LEVELVKEYS, levelVKeys);
	JSONUtil::setJsonObject(jsonObject, JSON_KEYBOARD,   keyboard);
	return jsonObject;
}

// Setting::Mouse
void Setting::Mouse::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_NAME,    name);
	JSONUtil::getJsonObject(jsonObject, JSON_BITMASK, bitMask);
}
QJsonObject Setting::Mouse::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_NAME,    name);
	JSONUtil::setJsonObject(jsonObject, JSON_BITMASK, bitMask);
	return jsonObject;
}

// Setting::ButtonMap
void Setting::ButtonMap::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_LEVELVKEYS, levelVKeys);
	JSONUtil::getJsonObject(jsonObject, JSON_BUTTON,     button);
}
QJsonObject Setting::ButtonMap::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_LEVELVKEYS, levelVKeys);
	JSONUtil::setJsonObject(jsonObject, JSON_BUTTON,     button);
	return jsonObject;
}


// Setting
void Setting::fromJsonObject(const QJsonObject& jsonObject) {
	JSONUtil::getJsonObject(jsonObject, JSON_ENTRY,      entryList);
	JSONUtil::getJsonObject(jsonObject, JSON_LEVELVKEYS, levelVKeysList);
	JSONUtil::getJsonObject(jsonObject, JSON_KEYBOARD,   keyboardList);
	JSONUtil::getJsonObject(jsonObject, JSON_KEYMAP,     keyMapList);
	JSONUtil::getJsonObject(jsonObject, JSON_MOUSE,      mouseList);
	JSONUtil::getJsonObject(jsonObject, JSON_BUTTONMAP,  buttonMapList);
}
QJsonObject Setting::toJsonObject() const {
	QJsonObject jsonObject;
	JSONUtil::setJsonObject(jsonObject, JSON_ENTRY,      entryList);
	JSONUtil::setJsonObject(jsonObject, JSON_LEVELVKEYS, levelVKeysList);
	JSONUtil::setJsonObject(jsonObject, JSON_KEYBOARD,   keyboardList);
	JSONUtil::setJsonObject(jsonObject, JSON_KEYMAP,     keyMapList);
	JSONUtil::setJsonObject(jsonObject, JSON_MOUSE,      mouseList);
	JSONUtil::setJsonObject(jsonObject, JSON_BUTTONMAP,  buttonMapList);
	return jsonObject;
}

Setting Setting::getInstance() {
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
//	logger.info("fileContents = %d", fileContents.size());

	Setting setting;

	{
		QJsonParseError jsonParseError;

		QJsonDocument jsonDocument = QJsonDocument::fromJson(fileContents, &jsonParseError);
		if (jsonParseError.error != QJsonParseError::NoError) {
			logger.error("Json Parse error");
			logger.error("  errorString  = %s", jsonParseError.errorString().toLocal8Bit().constData());
			logger.error("  offset       = %d", jsonParseError.offset);
			logger.error("  jsonDocument = %s!", jsonDocument.toJson(QJsonDocument::JsonFormat::Indented).constData());
			ERROR();
		}
		QJsonObject jsonObject = jsonDocument.object();

		Setting setting;
		setting.fromJsonObject(jsonObject);

		logger.info("entryList      %3d", setting.entryList.size());
		logger.info("levelVKeysList %3d", setting.levelVKeysList.size());
		logger.info("keyboardList   %3d", setting.keyboardList.size());
		logger.info("keyMapList     %3d", setting.keyMapList.size());
		logger.info("mouseList      %3d", setting.mouseList.size());
		logger.info("buttonMapList  %3d", setting.buttonMapList.size());
	}

	// sanity check
	{
		bool foundError = false;

		// levelVKeys
		{
			QMap<int, QString> map;
			for(auto e: setting.levelVKeysList) {
				if (map.contains(e.keyName)) {
					logger.error("levelVkeys duplicate keyName %3d  %s  %3d", e.keyName, e.name.toLocal8Bit().constData(), map[e.keyName].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.keyName] = e.name;
				}
			}
		}
		{
			QMap<QString, QString> map;
			for(auto e: setting.levelVKeysList) {
				if (map.contains(e.name)) {
					logger.error("levelVkeys duplicate name %s  %3d  %s", e.name.toLocal8Bit().constData(), e.keyName, map[e.name].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.name] = e.keyName;
				}
			}
		}

		// keyboard
		{
			QMap<int, QString> map;
			for(auto e: setting.keyboardList) {
				if (map.contains(e.scanCode)) {
					logger.error("keyboard duplicate scanCode %3d  %s  %4d", e.scanCode, e.name.toLocal8Bit().constData(), map[e.scanCode].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.scanCode] = e.name;
				}
			}
		}
		{
			QMap<QString, QString> map;
			for(auto e: setting.keyboardList) {
				if (map.contains(e.name)) {
					logger.error("keyboard duplicate name %s  %d  %s", e.name.toLocal8Bit().constData(), e.scanCode, map[e.name].toLocal8Bit().constData());
					foundError = true;
				} else {
					map[e.name] = e.scanCode;
				}
			}
		}

		// keyMap
		{
			QMap<QString, QString> map;
			for(auto e: setting.keyMapList) {
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
			for(auto e: setting.keyMapList) {
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

	return setting;
}

