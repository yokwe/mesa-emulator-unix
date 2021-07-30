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
	QString toString(QJsonValue::Type type);
	QString dump(const QJsonValue& jsonValue);
	QString dump(const QJsonArray& jsonArray);
	QString dump(const QJsonObject& jsonObject);

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


	// reference of jsonObject[key] at LHS
	QJsonValueRef    toJsonValueRef(QJsonObject& jsonObject, const QString& key);
	// reference of jsonObject[key] at RHS
	// Add const to prevent use at LHS of expression
	const QJsonValue toJsonValue(const QJsonObject& jsonObject, const QString& key);

	// class T must extends JSONBase fro toJsonValue method
	template <class T> inline void set(QJsonArray& array, QList<T> list) {
		for(auto e: list) {
			QJsonValue element(e.toJsonObject());
			array.append(element);
		}
	}
	template <> inline void set(QJsonArray& array, QList<int> list) {
		for(auto e: list) {
			QJsonValue element(e);
			array.append(element);
		}
	}
	template <> inline void set(QJsonArray& array, QList<bool> list) {
		for(auto e: list) {
			QJsonValue element(e);
			array.append(element);
		}
	}
	template <> inline void set(QJsonArray& array, QList<QString> list) {
		for(auto e: list) {
			QJsonValue element(e);
			array.append(element);
		}
	}

	template <class T> inline void set(QList<T>& array, const QJsonArray& jsonArray) {
		for(auto e: jsonArray) {
			T element;
			element.fromJsonObject(toObject(e));
			array.append(element);
		}
	}
	template <> inline void set(QList<int>& array, const QJsonArray& jsonArray) {
		for(auto e: jsonArray) {
			int element = toInt(e);
			array.append(element);
		}
	}
	template <> inline void set(QList<bool>& array, const QJsonArray& jsonArray) {
		for(auto e: jsonArray) {
			bool element = toBool(e);
			array.append(element);
		}
	}
	template <> inline void set(QList<QString>& array, const QJsonArray& jsonArray) {
		for(auto e: jsonArray) {
			QString element = toString(e);
			array.append(element);
		}
	}

};



#endif
