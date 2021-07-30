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
static log4cpp::Category& logger = Logger::getLogger("jsonutil");

#include "JSONUtil.h"


static QMap<QJsonValue::Type, QString> initTypeNameMap() {
	QMap<QJsonValue::Type, QString> ret;
	ret[QJsonValue::Null]      = "NULL";
	ret[QJsonValue::Bool]      = "Bool";
	ret[QJsonValue::Double]    = "Double";
	ret[QJsonValue::String]    = "String";
	ret[QJsonValue::Array]     = "Array";
	ret[QJsonValue::Object]    = "Object";
	ret[QJsonValue::Undefined] = "Undefined";

	return ret;
}
static QMap<QJsonValue::Type, QString> typeNameMap = initTypeNameMap();

QString JSONUtil::toString(QJsonValue::Type type) {
	if (typeNameMap.contains(type)) {
		return typeNameMap[type];
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  type = %d", type);
		ERROR();
	}
}

QString JSONUtil::dump(const QJsonValue& value) {
	QJsonValue::Type type = value.type();
	switch(type) {
	case QJsonValue::Type::Null:
		return "{Null}";
	case QJsonValue::Type::Bool:
		return QString::asprintf("{Bool %s}", value.toBool() ? "true" : "false");
	case QJsonValue::Type::Double:
		return QString("{Double %1}").arg(QString::number(value.toDouble()));
	case QJsonValue::Type::String:
		return QString("{String \"%1\"}").arg(value.toString());
	case QJsonValue::Type::Array:
	{
		QStringList list;
		for(auto e: value.toArray()) {
			list << JSONUtil::dump(e);
		}
		return QString("{Array %1 [%2]}").arg(list.size()).arg(list.join(", "));
	}
	case QJsonValue::Type::Object:
	{
		QStringList list;

		QJsonObject jsonObject = value.toObject();
		for(auto key: jsonObject.keys()) {
			QJsonValue value = jsonObject[key];
			list << QString("{\"%1\": %2}").arg(key).arg(JSONUtil::dump(value));
		}
		return QString("{Object %1 [%2]}").arg(list.size()).arg(list.join(", "));
	}
	case QJsonValue::Type::Undefined:
		return "{Undefined}";
	default:
		logger.fatal("Unexpected type");
		logger.fatal("  type      = %d", type);
		ERROR();
	}
}

QString JSONUtil::dump(const QJsonArray& jsonArray) {
	return JSONUtil::dump(QJsonValue(jsonArray));
}
QString JSONUtil::dump(const QJsonObject& jsonObject) {
	return JSONUtil::dump(QJsonValue(jsonObject));
}

QJsonValue JSONUtil::toJsonValue(const QString&     value) {
	return QJsonValue(value);
}
QJsonValue JSONUtil::toJsonValue(const int&         value) {
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

QString     JSONUtil::toString(const QJsonValue& jsonValue) {
	if (jsonValue.isString()) {
		return jsonValue.toString();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = String");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toLocal8Bit().constData());
		logger.fatal("  jsonValue = %s", JSONUtil::dump(jsonValue).toLocal8Bit().constData());
		ERROR();
	}
}
int         JSONUtil::toInt   (const QJsonValue& jsonValue) {
	if (jsonValue.isDouble()) {
		return jsonValue.toInt();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Double");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toLocal8Bit().constData());
		logger.fatal("  jsonValue = %s", JSONUtil::dump(jsonValue).toLocal8Bit().constData());
		ERROR();
	}
}
bool        JSONUtil::toBool  (const QJsonValue& jsonValue) {
	if (jsonValue.isBool()) {
		return jsonValue.toBool();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Bool");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toLocal8Bit().constData());
		logger.fatal("  jsonValue = %s", JSONUtil::dump(jsonValue).toLocal8Bit().constData());
		ERROR();
	}
}
QJsonObject JSONUtil::toObject(const QJsonValue& jsonValue) {
	if (jsonValue.isObject()) {
		return jsonValue.toObject();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Object");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toLocal8Bit().constData());
		logger.fatal("  jsonValue = %s", JSONUtil::dump(jsonValue).toLocal8Bit().constData());
		ERROR();
	}
}
QJsonArray  JSONUtil::toArray (const QJsonValue& jsonValue) {
	if (jsonValue.isArray()) {
		return jsonValue.toArray();
	} else {
		logger.fatal("Unexpected type");
		logger.fatal("  expect    = Array");
		logger.fatal("  type      = %s", toString(jsonValue.type()).toLocal8Bit().constData());
		logger.fatal("  jsonValue = %s", JSONUtil::dump(jsonValue).toLocal8Bit().constData());
		ERROR();
	}
}

QJsonValueRef JSONUtil::toJsonValueRef(QJsonObject& jsonObject, const QString& key) {
	if (!jsonObject.contains(key)) {
		jsonObject[key] = QJsonValue();
	}
	return jsonObject[key];
}
const QJsonValue    JSONUtil::toJsonValue(const QJsonObject& jsonObject, const QString& key) {
	if (jsonObject.contains(key)) {
		return jsonObject[key];
	} else {
		logger.fatal("Unexpected key");
		logger.fatal("  key = %s!", key.toLocal8Bit().constData());
		ERROR();
	}
}

