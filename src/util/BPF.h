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
// BPF.h
//

#ifndef BPF_H__
#define BPF_H__

#include <unistd.h>
#include <sys/time.h>
#include <net/bpf.h>

#include <QtCore>

#include "Network.h"

class BPF {
public:
	static const struct bpf_program* PROGRAM_IP;
	static const struct bpf_program* PROGRAM_XNS;

	int     fd;
	QString path;
	int     bufferSize;
	quint8* buffer;

	QList<ByteBuffer> readData;

	BPF() : fd(-1), bufferSize(-1), buffer(nullptr) {}
	~BPF() { delete buffer; }

	// openDevice sets fd and path
	void openDevice();

	void open() {
		openDevice();
		bufferSize = getBufferSize();
		buffer     = new quint8[bufferSize];
	}
	void close() {
		if (0 <= fd) {
			int ret;
			LOG_SYSCALL(ret, ::close(fd))
			fd = -1;
		}
		delete buffer;
		buffer = 0;
	}

	void write(const Network::Packet& value);
	const QList<ByteBuffer>& read();
	// read() returns QList<ByteBuffer> readData.
	// Backing data of ByteBuffer is field buffer.
	// Copy ByteBuffer to Network::Packet for later use.

	// BIOCGBLEN
	//   Returns the required buffer length	for reads on bpf files
	quint32 getBufferSize();

	// BIOCSBLEN
	//   Sets the buffer length for	reads on bpf files

	// BIOCGDLT
	//   Returns the type of the data link layer underlying the attached interface

	// BIOCGDLTLIST
	//   Returns an array of the available types of the data link layer underlying the attached interface

	// BIOCSDLT
	//   Changes the type of the data link layer underlying the attached interface

	// BIOCPROMISC
	//   Forces the interface into promiscuous mode
	void setPromiscious();

	// BIOCFLUSH
	//   Flushes the buffer	of incoming packets, and resets	the statistics
	void flush();

	// BIOCGETIF
	//   Returns the name of the hardware interface that the file is listening
	QString getInterface();

	// BIOCSETIF
	//   Sets the hardware interface associate with the file.
	void setInterface(const QString& value);

	// BIOCSRTIMEOUT
	//   Sets the read timeout parameter
	//   Default value is 0. Which means no timeout.
	void setReadTimeout(const struct timeval& value);
	void setReadTimeout(const quint32 seconds) {
		struct timeval time;
		time.tv_sec   = seconds;
		time.tv_usec = 0;
		setReadTimeout(time);
	}

	// BIOCGRTIMEOUT
	//   Gets the read timeout parameter
	void getReadTimeout(struct timeval& value);
	quint32 getReadTimeout() {
		struct timeval time;
		getReadTimeout(time);
		return (quint32)time.tv_sec;
	}

	// BIOCGSTATS
	//   Returns packet statistics

	// BIOCIMMEDIATE
	//   Enables or	disables "immediate mode"
	//   When immediate more is enabled, reads return immediately upon packet reception
	//   When immediate mode is disabled, read will block until buffer become full or timeout.
	void setImmediate(quint32 value);

	// BIOCSETF
	//   Sets the read filter program with BIOCFLUSH

	// BIOCSETFNR
	//   Sets the read filter program
	void setReadFilter(const struct bpf_program* value);

	// BIOCSETWF
	//   Sets the write filter program

	// BIOCVERSION
	//   Returns the major and	minor version numbers of the filter language

	// BIOCGRSIG
	//   Sets the status of	the "header complete" flag.
	quint32 getHeaderComplete();

	// BIOCSRSIG
	//   Gets the status of	the "header complete" flag.
	//   When value is 0, source address is filled automatically
	//   When value is 1, source address is not filled automatically
	//   Default value is 0
	void setHeaderComplete(quint32 value);

	// BIOCGDIRECTION
	//   Gets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
	quint32 getDirection();

	// BIOCSDIRECTION
	//   Sets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
	//   Vfalue must be BPF_D_IN, BPF_D_OUT or BPF_D_INOUT
	//   Default is BPF_D_INOUT
	void setDirection(quint32 value);

	// BIOCSTSTAMP
	//   Set format and resolution of the time stamps returned by BPF

	// BIOCGTSTAMP
	//   Get format and resolution of the time stamps returned by BPF

	// BIOCFEEDBACK
	//   Set packet feedback mode

	// BIOCLOCK
	//   Set the locked flag on the	bpf descriptor

	// BIOCGETBUFMODE
	//   Get the current bpf buffering mode

	// BIOCSETBUFMODE
	//   Set the current bpf buffering mode
	//   value must be BPF_BUFMODE_BUFFER or BPF_BUFMODE_ZBUF

	// BIOCSETZBUF
	//   Set the current zero-copy buffer locations

	// BIOCGETZMAX
	//   Get the largest individual zero-copy buffer size allowed

	// BIOCROTZBUF
	//   Force ownership of	the next buffer

	// FIONREAD
	//   Returns the number of bytes that are immediately available for	reading
	int getNonBlockingReadBytes();

	// SIOCGIFADDR
	//   Returns the address associated	with the interface.

	// FIONBIO
	//   Sets or clears non-blocking I/O
	//   If arg is non-zero, then doing a read(2) when no data is available will return -1 and errno will be set to EAGAIN
	void setNonBlockingIO(int value);

	// FIOASYNC
	//   Enables or disables async I/O

	// FIOSETOWN
	//   Sets the process or process group

	// FIOGETOWN
	//   Gets the process or process group

};

#endif
