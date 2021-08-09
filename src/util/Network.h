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
// Network.h
//

#ifndef NETWORK_H__
#define NETWORK_H__

#include <string.h>
#include <stdlib.h>

#include <QtCore>

namespace Network {
	// op independent network driver for Xerox IDP
	class Ethernet {
	public:
		static constexpr int ADDRESS_SIZE = 6;
		static constexpr int FRAME_SIZE   = 1514;

		class Address {
		public:
			quint8 value[ADDRESS_SIZE];

			Address() {
				memset(value, 0, sizeof(value));
			}
			Address(const Address& that) {
				memcpy(value, that.value, sizeof(value));
			}
			Address& operator =(const Address& that) {
				memcpy(value, that.value, sizeof(value));
				return *this;
			}

			Address(const quint8* p) {
				memcpy(value, p, sizeof(value));
			}
			Address(const char* p) {
				memcpy(value, p, sizeof(value));
			}
			quint64 toInt64() const {
				quint64 ret = 0;
				for(size_t i = 0; i < sizeof(value); i++) {
					ret <<= 8;
					ret |= value[i];
				}
				return ret;
			}
			QString toSting() {
				QString ret = QString::asprintf("%02X", value[0]);
				for(size_t i = 1; i < sizeof(value); i++) {
					ret += QString::asprintf("-%02X", value[i]);
				}
				return ret;
			}
		};

		QString name;
		Address address;

		Ethernet() {}
		Ethernet(const Ethernet& that) : name(that.name), address(that.address) {}
		Ethernet& operator =(const Ethernet& that) {
			this->name    = that.name;
			this->address = that.address;
			return *this;
		}

		QString toString() {
			return QString("{%1 %2}").arg(name).arg(address.toSting());
		}
	};
	QList<Ethernet> getEthernetList();

	class Data {
	public:
		int     len;                         // valid length in byte
		quint8  value[Ethernet::FRAME_SIZE]; // no endian conversion

		Data() : len(0) {
			memset(value, 0, sizeof(value));
		}
		Data(const Data& that) {
			this->len = that.len;
			memcpy(value, that.value, that.len);
		}
		Data& operator =(const Data& that) {
			this->len = that.len;
			memcpy(this->value, that.value, that.len);
			return *this;
		}
		Data(int len_, qint8* value_) {
			this->len = len_;
			memcpy(value, value_, len_);
		}

		void copyFrom(int len_, quint8* value_) {
			this->len = len_;
			memcpy(value, value_, len_);
		}
		// endian conversion
		void swab() {
			::swab(value, value, sizeof(value));
		}
	};

	class Interface {
		Interface() : ethernet(), fd(0) {}

		int select  (int& opError, quint32 timeout); // timeout in seconds
		int transmit(int& opError, Data& data);      // blocking operation
		int receive (int& opError, Data& data);      // blocking operation. use select to check data availability

		// discard received packet
		void discard();

		// get instance of inteface
		static Interface* getInstance(const QString& name);

	private:
		Ethernet ethernet;
		int      fd;
	};

}

#endif
