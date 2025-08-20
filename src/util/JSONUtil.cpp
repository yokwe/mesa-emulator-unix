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
// JSONUtil.cpp
//

#include "Util.h"
static const util::Logger logger(__FILE__);

#include "JSONUtil.h"


std::string JSONUtil::toJsonString(QJsonDocument jsonDocument) {
	return std::string::fromUtf8(jsonDocument.toJson(QJsonDocument::JsonFormat::Indented));
}

QJsonDocument JSONUtil::fromJsonString(const QByteArray &byteArray) {
	QJsonParseError jsonParseError;

	QJsonDocument jsonDocument = QJsonDocument::fromJson(byteArray, &jsonParseError);
	if (jsonParseError.error != QJsonParseError::NoError) {
		logger.error("Json Parse error");
		logger.error("  errorString  = %s", jsonParseError.errorString().toStdString());
		logger.error("  offset       = %d", jsonParseError.offset);
		logger.error("  jsonDocument = %s!", jsonDocument.toJson(QJsonDocument::JsonFormat::Indented).toStdString());
		ERROR();
	}
	return jsonDocument;
}


QJsonDocument JSONUtil::load(const std::string& path) {
	QByteArray byteArray;
	{
		QFile file(path);
		if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
			logger.fatal("File open error %s", file.errorString().toStdString());
			ERROR();
		}
		byteArray = file.readAll();
		file.close();
	}
	QJsonDocument jsonDocument = JSONUtil::fromJsonString(byteArray);
	return jsonDocument;
}

void JSONUtil::save(const std::string& path, const QJsonDocument& jsonDocument) {
	QByteArray byteArray = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
	{
		QFile file(path);
		if (!file.open(QIODevice::OpenModeFlag::WriteOnly)) {
			logger.fatal("File open error %s", file.errorString().toStdString());
			ERROR();
		}
		file.write(byteArray);
		file.close();
	}
}



QJsonObject JSONUtil::loadObject(const std::string& path) {
	QJsonDocument jsonDocument = load(path);
	if (jsonDocument.isObject()) {
		return jsonDocument.object();
	} else if (jsonDocument.isArray()) {
		logger.fatal("jsonDocument is Array");
		ERROR();
	} else {
		logger.fatal("Unexpected");
		logger.error("  jsonDocument = %s!", jsonDocument.toJson(QJsonDocument::JsonFormat::Indented).toStdString());
		ERROR();
	}
}
QJsonArray  JSONUtil::loadArray (const std::string& path) {
	QJsonDocument jsonDocument = load(path);
	if (jsonDocument.isObject()) {
		logger.fatal("jsonDocument is Object");
		logger.error("  jsonDocument = %s!", jsonDocument.toJson(QJsonDocument::JsonFormat::Indented).toStdString());
		ERROR();
	} else if (jsonDocument.isArray()) {
		return jsonDocument.array();
	} else {
		logger.fatal("Unexpected");
		logger.error("  jsonDocument = %s!", jsonDocument.toJson(QJsonDocument::JsonFormat::Indented).toStdString());
		ERROR();
	}
}


QJsonValue JSONUtil::toJsonValue(const std::string&     value) {
	return QJsonValue(value);
}
QJsonValue JSONUtil::toJsonValue(const qint32&      value) {
	return QJsonValue(value);
}
QJsonValue JSONUtil::toJsonValue(const bool&        value) {
	return QJsonValue(value);
}
QJsonValue JSONUtil::toJsonValue(const QJsonObject& value) {
	return QJsonValue(value);
}
QJsonValue JSONUtil::toJsonValue(const QJsonArray & value) {
	return QJsonValue(value);
}

std::string     JSONUtil::toString(const QJsonValue& jsonValue) {
	if (jsonValue.isString()) {
		return jsonValue.toString();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = String");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toStdString());
		logger.fatal("  jsonValue = %s", JSONUtil::toJsonString(jsonValue).toStdString());
		ERROR();
	}
}
qint32      JSONUtil::toInt   (const QJsonValue& jsonValue) {
	if (jsonValue.isDouble()) {
		return jsonValue.toInt();
	} else if (jsonValue.isString()) {
		return toIntMesaNumber(jsonValue.toString());
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Double");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toStdString());
		logger.fatal("  jsonValue = %s", JSONUtil::toJsonString(jsonValue).toStdString());
		ERROR();
	}
}
bool        JSONUtil::toBool  (const QJsonValue& jsonValue) {
	if (jsonValue.isBool()) {
		return jsonValue.toBool();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Bool");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toStdString());
		logger.fatal("  jsonValue = %s", JSONUtil::toJsonString(jsonValue).toStdString());
		ERROR();
	}
}
QJsonObject JSONUtil::toObject(const QJsonValue& jsonValue) {
	if (jsonValue.isObject()) {
		return jsonValue.toObject();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Object");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toStdString());
		logger.fatal("  jsonValue = %s", JSONUtil::toJsonString(jsonValue).toStdString());
		ERROR();
	}
}
QJsonArray  JSONUtil::toArray (const QJsonValue& jsonValue) {
	if (jsonValue.isArray()) {
		return jsonValue.toArray();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Array");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toStdString());
		logger.fatal("  jsonValue = %s", JSONUtil::toJsonString(jsonValue).toStdString());
		ERROR();
	}
}

QJsonValueRef JSONUtil::toJsonValueRef(QJsonObject& jsonObject, const std::string& key) {
	if (!jsonObject.contains(key)) {
		jsonObject[key] = QJsonValue();
	}
	return jsonObject[key];
}
const QJsonValue    JSONUtil::toJsonValue(const QJsonObject& jsonObject, const std::string& key) {
	if (jsonObject.contains(key)) {
		return jsonObject[key];
	} else {
		logger.fatal("Unexpected key");
		logger.fatal("  key = %s!", key.toStdString());
		ERROR();
	}
}

