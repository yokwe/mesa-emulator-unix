/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
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
// StreamWWC.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../mesa/Pilot.h"
#include "../mesa/Memory.h"
#include "../mesa/InterruptThread.h"

#include "AgentStream.h"
#include "StreamWWC.h"


StreamWWC::StreamWWC() : Stream("WWC", CoProcessorServerIDs::workspaceWindowControlMSWindowsService) {
	logger.info("%3d %-8s", serverID, name);
}

uint16_t StreamWWC::idle   (CoProcessorIOFaceGuam::CoProcessorFCBType *fcb, CoProcessorIOFaceGuam::CoProcessorIOCBType *iocb) {
	logger.error("%-8s idle %d %d", name, fcb->headCommand, iocb->serverID);
	return CoProcessorIOFaceGuam::R_completed;
}
uint16_t StreamWWC::accept (CoProcessorIOFaceGuam::CoProcessorFCBType *fcb, CoProcessorIOFaceGuam::CoProcessorIOCBType *iocb) {
	logger.error("%-8s accept %d %d", name, fcb->headCommand, iocb->serverID);
	ERROR();
	return CoProcessorIOFaceGuam::R_error;
}
uint16_t StreamWWC::connect(CoProcessorIOFaceGuam::CoProcessorFCBType * /*fcb*/, CoProcessorIOFaceGuam::CoProcessorIOCBType *iocb) {
	logger.info("%-8s connect  mesaIsServer = %d  state mesa = %d  pc = %d", name, iocb->mesaIsServer, iocb->mesaConnectionState, iocb->pcConnectionState);

	iocb->pcConnectionState = CoProcessorIOFaceGuam::S_connected;
	// Need to assign non-zero to mesaGet.hTaskactually. See CoProcessorFace.Get
	iocb->mesaGet.hTask = 1;

	// With GVWin-XC, If returns R_error, GVWin generate UnboundTrap and goes 915.
	// With GVWin-FX, If returns R_error, GVWin generate UnboundTrap and nothing happened.

	//return CoProcessorIOFaceGuam::R_completed;
	return CoProcessorIOFaceGuam::R_error;
}
uint16_t StreamWWC::destroy(CoProcessorIOFaceGuam::CoProcessorFCBType *fcb, CoProcessorIOFaceGuam::CoProcessorIOCBType *iocb) {
	logger.info("%-8s destroy %d %d", name, fcb->headCommand, iocb->serverID);
	return CoProcessorIOFaceGuam::R_error;
}
uint16_t StreamWWC::read   (CoProcessorIOFaceGuam::CoProcessorFCBType *fcb, CoProcessorIOFaceGuam::CoProcessorIOCBType *iocb) {
	logger.error("%-8s write %d %d", name, fcb->headCommand, iocb->serverID);
	return CoProcessorIOFaceGuam::R_error;
}
uint16_t StreamWWC::write  (CoProcessorIOFaceGuam::CoProcessorFCBType *fcb, CoProcessorIOFaceGuam::CoProcessorIOCBType *iocb) {
	logger.error("%-8s write %d %d", name, fcb->headCommand, iocb->serverID);
	CoProcessorIOFaceGuam::TransferRec& tr = iocb->mesaGet;
	logger.info("mesaGet  sst: %d  end [Stream: %d  Record: %d  SST: %d]  written: %3d  read: %3d  hTask: %d  int: %d  buffer: %4X  bufferSize: %3d  lock: %d",
		tr.subSequence, tr.endStream + 0, tr.endRecord + 0, tr.endSST + 0,
		tr.bytesWritten, tr.bytesRead, tr.hTask, tr.interruptMesa, tr.buffer, tr.bufferSize, tr.writeLockedByMesa);

	if (tr.writeLockedByMesa) {
		logger.warn("writeLockedByMesa");
		return CoProcessorIOFaceGuam::R_inProgress;
	}
	if (tr.buffer == 0) {
		logger.fatal("tr.buffer = 0");
		ERROR();
	}
	if (tr.bytesRead != 0) {
		logger.fatal("tr.bytesRead = %d", tr.bytesRead);
		ERROR();
	}
	if (tr.bytesWritten != 0) {
		logger.fatal("tr.bytesWritten = %d", tr.bytesWritten);
		ERROR();
	}

	uint16_t* buffer = (uint16_t*)Store(tr.buffer);
	for(int i = 0; i < 9; i++) buffer[i] = 0;

	// TODO Need to figure out proper value to store buffer.
	// I guess 18 bytes may contains information below.
	//
	// Excerpt from GVWIN.INI
	// [Window]
	// FullScreen=TRUE
	// TitleBar=FALSE
	// WorkspaceWidth=640
	// WorkspaceHeight=480
	// Window=-4,-4,648,488,0


	tr.bytesWritten = 2 * (1 + 2 + 2 + 2 + 2); // It seems that read 18 bytes in first call.
	if (tr.interruptMesa) {
		if (fcb->interruptSelector) {
			fcb->headResult = CoProcessorIOFaceGuam::R_completed;
			InterruptThread::notifyInterrupt(fcb->interruptSelector);
		}
	}
	return CoProcessorIOFaceGuam::R_completed;
}
