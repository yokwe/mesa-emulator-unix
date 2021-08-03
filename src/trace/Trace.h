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

	class Func {
	public:
		CARD16 gfi;
		CARD16 pc;

		Func() : gfi(0), pc(0) {}
		Func(const Func& that) : gfi(that.gfi), pc(that.pc) {}
		Func& operator= (const Func& that) {
			this->gfi = that.gfi;
			this->pc  = that.pc;
			return *this;
		}

		bool operator== (const Func& that) const {
			return this->gfi == that.gfi && this->pc == that.pc;
		}
		bool operator< (const Func& that) const {
			if (this->gfi == that.gfi) {
				return this->pc < that.pc;
			} else {
				return this->gfi < that.gfi;
			}
		}

		static Func getInstance(CARD16 gfi, CARD16 pc) {
			Func ret(gfi, pc);
			all.insert(ret);
			return ret;
		}

		static QList<Func> toList() {
			QList<Func> ret = all.values();
			std::sort(ret.begin(), ret.end());
			return ret;
		}

		QString toString() const;

		static void addName(CARD16 gfi, const QString& moduleName);
		static void addName(const QString& moduleName, const QString& funcName, const CARD16 pc);

		static void readLoadmapFile(const QString& path);
		static void readMapFile(const QString& path);
	private:
		static QSet<Func> all;

		Func(CARD16 gfi_, CARD16 pc_) : gfi(gfi_), pc(pc_) {}
	};

	// For QSet/QHash
	inline uint qHash(const Trace::Func &key, uint seed = 0) {
		return ::qHash(((key.gfi << 16) | key.pc), seed ^ 2941);
	}


	class Frame {
	public:
		CARD16 mds;
		CARD16 lf; // LF

		Frame() : mds(0), lf(0) {}
		Frame(const Frame& that) : mds(that.mds), lf(that.lf) {}
		Frame& operator= (const Frame& that) {
			this->mds  = that.mds;
			this->lf   = that.lf;
			return *this;
		}

		bool operator== (const Frame& that) const {
			return this->mds == that.mds && this->lf == that.lf;
		}
		bool operator< (const Frame& that) const {
			if (this->mds == that.mds) {
				return this->lf < that.lf;
			} else {
				return this->mds < that.mds;
			}
		}

		QString toString() const {
			return QString::asprintf("%d+%04X", mds, lf);
		}

		static Frame getInstance(CARD32 mds, CARD16 lf) {
			return Frame(mds >> 16, lf);
		}
	private:
		Frame(CARD16 mds_, CARD16 lf_) : mds(mds_), lf(lf_) {}
	};


	class Context {
	public:
		enum CallType {CT_XFER, CT_LFC};

		CallType         callType;

		// XFER parameter
		ControlLink dst;
		CARD16      src;
		XferType    xferType;
		int         freeFlag;

		// actual LinkType of XFER
		LinkType    linkType;

		// current PSB and MDS
		CARD16      oldPSB;

		// values before XFER
		Func        oldFunc;
		Frame       oldFrame;

		// destination
		Func        newFunc;
		Frame       newFrame;

		Context() : callType(CT_XFER), dst(0), src(0), xferType(XT_return), freeFlag(0), linkType(LT_frame), oldPSB(0), oldFunc(), oldFrame(), newFunc(), newFrame() {}
		Context(const Context& that) :
			callType(that.callType), dst(that.dst), src(that.src), xferType(that.xferType), freeFlag(that.freeFlag),
			linkType(that.linkType), oldPSB(that.oldPSB),
			oldFunc(that.oldFunc), oldFrame(that.oldFrame), newFunc(that.newFunc), newFrame(that.newFrame) {}
		Context& operator= (const Context& that) {
			this->callType = that.callType;
			this->dst      = that.dst;
			this->src      = that.src;
			this->xferType = that.xferType;
			this->freeFlag = that.freeFlag;
			this->linkType = that.linkType;
			this->oldPSB   = that.oldPSB;
			this->oldFunc  = that.oldFunc;
			this->oldFrame = that.oldFrame;
			this->newFunc  = that.newFunc;
			this->newFrame = that.newFrame;
			return *this;
		}

		QString toString() const;

		void setXFER(
			ControlLink dst, CARD16 src, XferType xferType, int freeFlag,
			LinkType linkType, CARD16 oldPSB,
			GFTHandle oldGFI, CARD16 oldPC, CARD32 oldMDS, CARD16 oldLF
			) {
			if (TRACE_ENABLE_TRACE) {
				set(CT_XFER, dst, src, xferType, freeFlag, linkType, oldPSB, oldGFI, oldPC, oldMDS, oldLF);
			}
		}
		void setLFC(
			ControlLink dst, CARD16 src, XferType xferType, int freeFlag,
			LinkType linkType, CARD16 oldPSB,
			CARD16 oldGFI, CARD16 oldPC, CARD32 oldMDS, CARD16 oldLF
			) {
			if (TRACE_ENABLE_TRACE) {
				set(CT_LFC, dst, src, xferType, freeFlag, linkType, oldPSB, oldGFI, oldPC, oldMDS, oldLF);
			}
		}
		void setContext(CARD16 newGFI, CARD16 newPC, CARD32 newMDS, CARD16 newLF) {
			if (TRACE_ENABLE_TRACE) {
				set(newGFI, newPC, newMDS, newLF);
			}
		}
		void process() {
			if (TRACE_ENABLE_TRACE) process_();
		}

	private:
		void set(
			CallType callType, ControlLink dst, ShortControlLink src, XferType xferType, int freeFlag,
			LinkType linkType, CARD16 oldPSB,
			CARD16 oldGFI, CARD16 oldPC, CARD32 oldMDS, CARD16 oldLF
			) {
			this->callType = callType;
			this->dst      = dst;
			this->src      = src;
			this->xferType = xferType;
			this->freeFlag = freeFlag;

			this->linkType = linkType;
			this->oldPSB   = oldPSB;

			this->oldFunc  = Func::getInstance(oldGFI, oldPC);
			this->oldFrame = Frame::getInstance(oldMDS, oldLF);
		}
		void set(CARD16 newGFI, CARD16 newPC, CARD32 newMDS, CARD16 newLF) {
			this->newFunc = Func::getInstance(newGFI, newPC);
			this->newFrame = Frame::getInstance(newMDS, newLF);
		}

		void process_();
	};
}

#endif
