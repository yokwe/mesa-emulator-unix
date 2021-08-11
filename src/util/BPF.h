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

#include "../xns/XNS.h"


class BPF {
public:
	int     fd;
	int     bufferSize;
	quint8* buffer;

	QString path;
	QString name;

	BPF() : fd(-1), bufferSize(-1), buffer(nullptr) {}
	~BPF() { delete buffer; }

	void open();
	void attach(const QString& name);
	void close();

	// BIOCGBLEN
	//   Returns the required buffer length	for reads on bpf files
	void getBufferLength(u_int& value);
	// BIOCSBLEN
	//   Sets the buffer length for	reads on bpf files
	void setBufferLength(u_int value);
	// BIOCGDLT
	//   Returns the type of the data link layer underlying the attached interface
	// BIOCGDLTLIST
	//   Returns an array of the available types of the data link layer underlying the attached interface
	// BIOCSDLT
	//   Changes the type of the data link layer underlying the attached interface
	// BIOCPROMISC
	//   Forces the interface into promiscuous mode
	void setPromisciousMode();
	// BIOCFLUSH
	//   Flushes the buffer	of incoming packets, and resets	the statistics
	void flush();
	// BIOCGETIF
	//   Returns the name of the hardware interface that the file is listening
	// BIOCSETIF
	//   Sets the hardware interface associate with the file.
	void setInterface(const QString& value);
	// BIOCSRTIMEOUT
	//   Sets or gets the read timeout parameter
	void setTimeout(struct timeval value);
	// BIOCGRTIMEOUT
	//   Gets or gets the read timeout parameter
	//   Default value is 0. Which means no timeout.
	void setTimeout(timeval& value);
	// BIOCGSTATS
	//   Returns packet statistics
	// BIOCIMMEDIATE
	//   Enables or	disables "immediate mode"
	//   When immediate more is enabled, reads return immediately upon packet reception
	//   When immediate mode is disabled, read will block until buffer become full or timeout.
	void immediateMode(u_int value);
	// BIOCSETF
	//   Sets the read filter program with BIOCFLUSH
	// BIOCSETFNR
	//   Sets the read filter program
	void setReadFilter(struct bpf_program& value);
	// BIOCSETWF
	//   Sets the write filter program
	void setWriteFilter(struct bpf_program& value);
	// BIOCVERSION
	//   Returns the major and	minor version numbers of the filter language
	// BIOCGRSIG
	//   Sets the status of	the "header complete" flag.
	// BIOCSRSIG
	//   Gets the status of	the "header complete" flag.
	//   When value is 0, source address is filled automatically
	//   When value is 1, source address is not filled automatically
	//   Default value is 0
	void setHeaderCoplete(u_int value);
	// BIOCSDIRECTION
	//   Sets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
	//   Vfalue must be BPF_D_IN, BPF_D_OUT or BPF_D_INOUT
	//   Default is BPF_D_INOUT
	void setDirection(u_int value);
	// BIOCSDIRECTION
	//   Gets the setting determining whether incoming, outgoing, or all packets on the interface should be returned by BPF
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
	void setBufferMode(u_int value);
	// BIOCSETBUFMODE
	//   Set the current bpf buffering mode
	//   value must be BPF_BUFMODE_BUFFER or BPF_BUFMODE_ZBUF
	void getBufferMode(u_int& value);
	// BIOCSETZBUF
	//   Set the current zero-copy buffer locations
	// BIOCGETZMAX
	//   Get the largest individual zero-copy buffer size allowed
	// BIOCROTZBUF
	//   Force ownership of	the next buffer
	// FIONREAD
	//   Returns the number of bytes that are immediately available for	reading
	void nonBlockingRead(int& value);
	// SIOCGIFADDR
	//   Returns the address associated	with the interface.
	// FIONBIO
	//   Sets or clears non-blocking I/O
	//   If arg is non-zero, then doing a read(2) when no data is available will return -1 and errno will be set to EAGAIN
	void setNonBlocking(int value);
	// FIOASYNC
	//   Enables or disables async I/O
	// FIOSETOWN
	//   Sets the process or process group
	// FIOGETOWN
	//   Gets the process or process group

};

#endif
