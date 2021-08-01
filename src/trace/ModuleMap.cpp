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
// ModuleMap.cpp
//

#include "../util/Util.h"
//static log4cpp::Category& logger = Logger::getLogger("modulemap"); // FIXME

#include "ModuleMap.h"


void ModuleMap::Module::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(module);
	GET_JSON_OBJECT(gf);
	GET_JSON_OBJECT(cb);
	GET_JSON_OBJECT(gfi);
}
QJsonObject ModuleMap::Module::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(module);
	SET_JSON_OBJECT(gf);
	SET_JSON_OBJECT(cb);
	SET_JSON_OBJECT(gfi);
	return jsonObject;
}

void ModuleMap::Map::fromJsonObject(const QJsonObject& jsonObject) {
	GET_JSON_OBJECT(module);
	GET_JSON_OBJECT(proc);
	GET_JSON_OBJECT(bytes);
	GET_JSON_OBJECT(evi);
	GET_JSON_OBJECT(pc);
}
QJsonObject ModuleMap::Map::toJsonObject() const {
	QJsonObject jsonObject;
	SET_JSON_OBJECT(module);
	SET_JSON_OBJECT(proc);
	SET_JSON_OBJECT(bytes);
	SET_JSON_OBJECT(evi);
	SET_JSON_OBJECT(pc);
	return jsonObject;
}

QString loadFile(QString path) {
	QByteArray byteArray;
	{
		QFile file(path);
		if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
			logger.fatal("File open error %s", file.errorString().toLocal8Bit().constData());
			ERROR();
		}
		byteArray = file.readAll();
		file.close();
	}
	return QString::fromUtf8(byteArray);
}

QList<ModuleMap::Module> ModuleMap::Module::load(QString path) {
	QString string = loadFile(path);

	// GermOpsImpl 0AB0H 1209H 1CH
	QRegularExpression re("[A-Za-z0-9]+ [0-9A-FH]+ [0-9A-FH]+ [0-9A-FH]+");

	QList<ModuleMap::Module> ret;
	for(auto e: string.split(QChar('\r'), Qt::SkipEmptyParts)) {
		QString simplified = e.simplified();
		auto m = re.match(simplified);
		if (m.hasMatch()) {
			QStringList token = simplified.split(" ");
			// GermOpsImpl               0AB0H    1209H     1CH
			QString module = token[0];
			int     gf     = toIntMesaNumber(token[1]);
			int     cb     = toIntMesaNumber(token[2]);
			int     gfi    = toIntMesaNumber(token[3]);
			Module element(module, gf, cb, gfi);
			ret += element;
		}
	}

	return ret;
}

QList<ModuleMap::Map> ModuleMap::Map::load(QString path) {
	QString string = loadFile(path);

	// Bytes   EVI  Offset    IPC   Module               Procedure
	//    42B   13   1030B     20B  ProcessorHeadGuam    GetNextAvailableVM
	QRegularExpression re("[0-7]+B? [0-9]+ [0-7]+B? [0-7]+B? [A-Za-z0-9]+ [A-Za-z0-9]+");

	QList<ModuleMap::Map> ret;
	for(auto e: string.split(QChar('\r'), Qt::SkipEmptyParts)) {
		QString simplified = e.simplified();
		auto m = re.match(simplified);
		if (m.hasMatch()) {
			QStringList token = simplified.split(" ");
			int     bytes  = toIntMesaNumber(token[0]);
			int     evi    = toIntMesaNumber(token[1]);
//			int     offset = toIntMesaNumber(token[2]);
			int     ipc    = toIntMesaNumber(token[3]);
			QString module = token[4];
			QString proc   = token[5];

			ModuleMap::Map element(module, proc, bytes, evi, ipc);
			ret += element;
		}
	}

	return ret;
}

