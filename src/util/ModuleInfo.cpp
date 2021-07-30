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


#include "../util/Util.h"
static log4cpp::Category& logger = Logger::getLogger("module-info");

#include "ModuleInfo.h"


static const QString PATH_FILE = QStringLiteral("data/module-info.xml");

static const QString XML_MODULE_INFO = QStringLiteral("module-info");
static const QString XML_ENTRY       = QStringLiteral("entry");
static const QString XML_MODULE      = QStringLiteral("module");
static const QString XML_PROC        = QStringLiteral("proc");
static const QString XML_PC          = QStringLiteral("pc");
static const QString XML_NAME        = QStringLiteral("name");
static const QString XML_GFI         = QStringLiteral("gfi");
static const QString XML_GF          = QStringLiteral("gf");
static const QString XML_CB          = QStringLiteral("cb");


static void readAttribute(QXmlStreamAttributes& attributes, QString name, QString& value) {
	if (attributes.hasAttribute(name)) {
		value = attributes.value(name).toString();
	} else {
		logger.error("Unexpected no attribute %s!", name.toLocal8Bit().constData());
		for(auto i: attributes) {
			logger.error("  %s => %s!", i.name().toLocal8Bit().constData(), i.value().toLocal8Bit().constData());
		}
		ERROR();
	}
}

static void readAttribute(QXmlStreamAttributes& attributes, QString name, quint32& value) {
	QString stringValue;

	readAttribute(attributes, name, stringValue);

	value = toInt(stringValue);
}
static void readAttribute(QXmlStreamAttributes& attributes, QString name, quint16& value) {
	QString stringValue;

	readAttribute(attributes, name, stringValue);

	value = (quint16)toInt(stringValue);
}

ModuleInfo::Entry::Entry(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_ENTRY) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_MODULE, module);
	readAttribute(attributes, XML_PROC,   proc);
	readAttribute(attributes, XML_PC,     pc);
	reader.skipCurrentElement();
}

ModuleInfo::Module::Module(QXmlStreamReader& reader) {
	// sanity check
	if (!reader.isStartElement()) {
		logger.error("Unexpected %s!", reader.tokenString().toLocal8Bit().constData());
		ERROR();
	}
	if (reader.name() != XML_MODULE) {
		logger.error("Unexpected  %s!", reader.name().toLocal8Bit().constData());
		ERROR();
	}

	QXmlStreamAttributes attributes = reader.attributes();
	readAttribute(attributes, XML_NAME, name);
	readAttribute(attributes, XML_GF,   gf);
	readAttribute(attributes, XML_CB,   cb);
	readAttribute(attributes, XML_GFI,  gfi);
	reader.skipCurrentElement();
}


static QMap<quint16, QString> moduleMap;
//          gfi
static QMap<QString, QString> nameMap;
//          gfi-pc   name

static void readXMLFile(QList<ModuleInfo::Module> &moduleList, QList<ModuleInfo::Entry> &entryList) {
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
		// sanity check
		if (reader.name() != XML_MODULE_INFO) {
			logger.error("Unexpected %s!", reader.name().toLocal8Bit().constData());
			ERROR();
		}

		while(reader.readNextStartElement()) {
			if (reader.name() == XML_ENTRY) {
				ModuleInfo::Entry entry(reader);
				entryList += entry;
			} else if (reader.name() == XML_MODULE) {
				ModuleInfo::Module module(reader);
				moduleList += module;
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
}

static QString getKey(quint16 gfi, quint16 pc) {
	return QString::asprintf("%4X-%04X", gfi, pc);
}
static void initialize() {
	QList<ModuleInfo::Module> moduleList;
	QList<ModuleInfo::Entry>  entryList;

	readXMLFile(moduleList, entryList);

	for(auto e: moduleList) {
		moduleMap[e.gfi] = e.name;
	}

	QMap<QString, quint16> nameToGFIMap;
	//   module   gfi
	for(auto e: moduleList) {
		nameToGFIMap[e.name] = e.gfi;
	}

	QStringList list;
	for(auto e: entryList) {
		if (e.proc == "<nested>") continue;
		if (nameToGFIMap.contains(e.module)) {
			quint16 gfi = nameToGFIMap[e.module];
			quint16 pc  = e.pc;

			QString key   = getKey(gfi, pc);
			QString value = QString("%1::%2").arg(e.module).arg(e.proc);
			nameMap[key] = value;

			list += QString("%1 %2").arg(key).arg(value);
		}
	}
	std::sort(list.begin(), list.end(), std::less<QString>());
	for(auto e: list) {
		logger.info("nameMap %s", e.toLocal8Bit().constData());
	}
}

QString ModuleInfo::getName(quint16 gfi, quint16 pc) {
	if (nameMap.isEmpty()) {
		initialize();
	}

	QString key = getKey(gfi, pc);
	if (nameMap.contains(key)) {
		return nameMap[key];
	} else {
		if (moduleMap.contains(gfi)) {
			QString module   = moduleMap[gfi];
			QString pcString = QString::asprintf("%04X", pc);
			return QString("%1::%2").arg(module).arg(pcString);
		} else {
			return key;
		}
	}
}
