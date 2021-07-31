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
// JSONUtil.h
//

#ifndef JSONUTIL_H__
#define JSONUTIL_H__

#include <QtCore>

class JSONBase {
public:
	// this <= jsonObject
	virtual void fromJsonObject(const QJsonObject& jsonObject) = 0;
	// jsonObject <= this
	virtual QJsonObject toJsonObject() const = 0;

	virtual ~JSONBase() {}
};

namespace JSONUtil {
	QString toJsonString(QJsonDocument jsonDocument);
	inline QString toJsonString(QJsonObject   jsonObject) {
		QJsonDocument jsonDocument(jsonObject);
		return toJsonString(jsonDocument);
	}
	inline QString toJsonString(QJsonArray jsonArray) {
		QJsonDocument jsonDocument(jsonArray);
		return toJsonString(jsonDocument);
	}
	inline QString toJsonString(QJsonValue jsonValue) {
		QJsonObject jsonObject;
		jsonObject["jsonValue"] = jsonValue;
		return toJsonString(jsonObject);
	}

	QJsonDocument fromJsonString(const QByteArray &byteArray);
	inline QJsonDocument fromJsonString(QString string) {
		return fromJsonString(string.toUtf8());
	}

	QJsonDocument load(const QString& path);
	QJsonObject   loadObject(const QString& path);
	QJsonArray    loadArray (const QString& path);


	QJsonValue toJsonValue(const QString&     value);
	QJsonValue toJsonValue(const int&         value);
	QJsonValue toJsonValue(const bool&        value);
	QJsonValue toJsonValue(const QJsonObject& value);
	QJsonValue toJsonValue(const QJsonArray & value);

	QString     toString(const QJsonValue& jsonValue);
	int         toInt   (const QJsonValue& jsonValue);
	bool        toBool  (const QJsonValue& jsonValue);
	QJsonObject toObject(const QJsonValue& jsonValue);
	QJsonArray  toArray (const QJsonValue& jsonValue);


	// Reference of jsonObject[key] at LHS
	QJsonValueRef    toJsonValueRef(QJsonObject& jsonObject, const QString& key);
	// Reference of jsonObject[key] at RHS
	// Add const to function return to prevent use at LHS of expression
	const QJsonValue toJsonValue(const QJsonObject& jsonObject, const QString& key);

	// helper macro to invoke setJsonObject / getJsonObject
#define SET_JSON_OBJECT(name) SET_JSON_OBJECT2(name, name)
#define SET_JSON_OBJECT2(key, field) JSONUtil::setJsonObject(jsonObject, #key, field);

#define GET_JSON_OBJECT(name) GET_JSON_OBJECT2(name, name)
#define GET_JSON_OBJECT2(key, field) JSONUtil::getJsonObject(jsonObject, #key, field);

	// jsonObject[key] = value
	template <class T> inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const T& value) {
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(value.toJsonObject());
	}
	void inline setJsonObject(QJsonObject& jsonObject, const QString& key, const QString& value) {
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(value);
	}
	inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const int      value) {
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(value);
	}
	inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const bool     value) {
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(value);
	}

	// value = jsonObject[key]
	template <class T> inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, T& value) {
		value.fromJsonObject(JSONUtil::toObject(JSONUtil::toJsonValue(jsonObject, key)));
	}
	inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, QString& value) {
		value = JSONUtil::toString(JSONUtil::toJsonValue(jsonObject, key));
	}
	inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, int&     value) {
		value = JSONUtil::toInt(JSONUtil::toJsonValue(jsonObject, key));
	}
	inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, bool&    value) {
		value = JSONUtil::toBool(JSONUtil::toJsonValue(jsonObject, key));
	}


	// jsonObject[key] = list
	template <class T> inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const QList<T>& list) {
		QJsonArray jsonArray;
		for(auto e: list) {
			jsonArray.append(JSONUtil::toJsonValue(e.toJsonObject()));
		}
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(jsonArray);
	}
	inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const QList<QString>& list) {
		QJsonArray jsonArray;
		for(auto e: list) {
			jsonArray.append(JSONUtil::toJsonValue(e));
		}
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(jsonArray);
	}
	inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const QList<int>&     list) {
		QJsonArray jsonArray;
		for(auto e: list) {
			jsonArray.append(JSONUtil::toJsonValue(e));
		}
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(jsonArray);
	}
	inline void setJsonObject(QJsonObject& jsonObject, const QString& key, const QList<bool>&    list) {
		QJsonArray jsonArray;
		for(auto e: list) {
			jsonArray.append(JSONUtil::toJsonValue(e));
		}
		JSONUtil::toJsonValueRef(jsonObject, key) = JSONUtil::toJsonValue(jsonArray);
	}

	// list = jsonObject[key]
	template <class T> inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, QList<T>& list) {
		for(auto e: JSONUtil::toArray(JSONUtil::toJsonValue(jsonObject, key))) {
			T element;
			element.fromJsonObject(JSONUtil::toObject(e));
			list.append(element);
		}
	}
	inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, QList<QString>& list) {
		for(auto e: JSONUtil::toArray(JSONUtil::toJsonValue(jsonObject, key))) {
			QString element = JSONUtil::toString(e);
			list.append(element);
		}
	}
	inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, QList<int>&     list) {
		for(auto e: JSONUtil::toArray(JSONUtil::toJsonValue(jsonObject, key))) {
			int element = JSONUtil::toInt(e);
			list.append(element);
		}
	}
	inline void getJsonObject(const QJsonObject& jsonObject, const QString& key, QList<bool>&    list) {
		for(auto e: JSONUtil::toArray(JSONUtil::toJsonValue(jsonObject, key))) {
			bool element = JSONUtil::toBool(e);
			list.append(element);
		}
	}
};

#endif
