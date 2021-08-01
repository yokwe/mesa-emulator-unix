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
// Trace.h
//

#ifndef TRACE_H__
#define TRACE_H__

#include "../mesa/Constant.h"
#include "../mesa/Type.h"

static const int TRACE_ENABLE_TRACE = 1;

namespace Trace {
	const char* getLinkType(LinkType type);
	const char* getXferType(XferType type);
	const char* getTrapName(ControlLink controlLink);

	class Context {
	public:
		enum CallType {CT_XFER, CT_LFC};

		CallType         callType;

		// XFER parameter
		ControlLink      dst;
		ShortControlLink src;
		XferType         xferType;
		int              freeFlag;

		// actual LinkType of XFER
		LinkType         linkType;

		// current PSB and MDS
		CARD16           oldPSB;
		CARD32           oldMDS;

		// values before XFER
		GFTHandle        oldGFI;
		CARD16           oldPC;
		LocalFrameHandle oldLF;

		// destination
		GFTHandle        newGFI;
		CARD16           newPC;
		LocalFrameHandle newLF;

		Context() : callType(CT_XFER), dst(0), src(0), xferType(XT_return), freeFlag(0), linkType(LT_frame), oldPSB(0), oldMDS(0), oldGFI(0), oldPC(0), oldLF(0), newGFI(0), newPC(0), newLF(0) {}
		Context(const Context& that) :
			callType(that.callType), dst(that.dst), src(that.src), xferType(that.xferType), freeFlag(that.freeFlag),
			linkType(that.linkType), oldPSB(that.oldPSB), oldMDS(that.oldMDS),
			oldGFI(that.oldGFI), oldPC(that.oldPC), oldLF(that.oldLF),
			newGFI(that.newGFI), newPC(that.newPC), newLF(that.newLF) {}
		Context& operator= (const Context& that) {
			this->callType = that.callType;
			this->dst      = that.dst;
			this->src      = that.src;
			this->xferType = that.xferType;
			this->freeFlag = that.freeFlag;
			this->linkType = that.linkType;
			this->oldPSB   = that.oldPSB;
			this->oldMDS   = that.oldMDS;
			this->oldGFI   = that.oldGFI;
			this->oldPC    = that.oldPC;
			this->oldLF    = that.oldLF;
			this->newGFI   = that.newGFI;
			this->newPC    = that.newPC;
			this->newLF    = that.newLF;
			return *this;
		}

		void setXFER(
			ControlLink dst, ShortControlLink src, XferType xferType, int freeFlag,
			LinkType linkType, CARD16 oldPSB, CARD32 oldMDS,
			GFTHandle oldGFI, CARD16 oldPC, LocalFrameHandle oldLF
			) {
			if (TRACE_ENABLE_TRACE) {
				set(CT_XFER, dst, src, xferType, freeFlag, linkType, oldPSB, oldMDS, oldGFI, oldPC, oldLF);
			}
		}
		void setLFC(
			ControlLink dst, ShortControlLink src, XferType xferType, int freeFlag,
			LinkType linkType, CARD16 oldPSB, CARD32 oldMDS,
			GFTHandle oldGFI, CARD16 oldPC, LocalFrameHandle oldLF
			) {
			if (TRACE_ENABLE_TRACE) {
				set(CT_LFC, dst, src, xferType, freeFlag, linkType, oldPSB, oldMDS, oldGFI, oldPC, oldLF);
			}
		}
		void setContext(CARD16 newGFI, CARD16 newPC, CARD16 newLF) {
			if (TRACE_ENABLE_TRACE) {
				this->newGFI = newGFI;
				this->newPC  = newPC;
				this->newLF  = newLF;
			}
		}
		void process() {
			if (TRACE_ENABLE_TRACE) process_();
		}
		void message() {
			if (TRACE_ENABLE_TRACE) message_();
		}

	private:
		void set(
			CallType callType, ControlLink dst, ShortControlLink src, XferType xferType, int freeFlag,
			LinkType linkType, CARD16 oldPSB, CARD32 oldMDS,
			GFTHandle oldGFI, CARD16 oldPC, LocalFrameHandle oldLF
			) {
			this->callType = callType;
			this->dst      = dst;
			this->src      = src;
			this->xferType = xferType;
			this->freeFlag = freeFlag;

			this->linkType = linkType;
			this->oldPSB   = oldPSB;
			this->oldMDS   = oldMDS;

			this->oldGFI   = oldGFI;
			this->oldPC    = oldPC;
			this->oldLF    = oldLF;

			this->newGFI   = 0;
			this->newPC    = 0;
			this->newLF    = 0;
		}

		void process_();
		void message_();
	};
};

#endif
