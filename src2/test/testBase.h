/*******************************************************************************
 * Copyright (c) 2024, Yasuhiro Hasegawa
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
// testBase.h
//

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include "../mesa/Type.h"
#include "../mesa/Constant.h"
#include "../mesa/Variable.h"
#include "../mesa/Function.h"

#include "../mesa/Memory.h"

#include "../opcode/Opcode.h"
#include "../opcode/Interpreter.h"

class testBase : public CppUnit::TestFixture {
protected:
	void initRegister();
	void initAV(mesa::CARD16 origin, mesa::CARD16 limit);
	void initGFT();
	void initSD();
	void initETT();
	void initPDA();

	mesa::CARD16 *page_PDA;
	mesa::CARD16 *page_GFT;
	mesa::CARD16 *page_CB;
	mesa::CARD16 *page_MDS;
	mesa::CARD16 *page_AV;
	mesa::CARD16 *page_SD;
	mesa::CARD16 *page_ETT;
	mesa::CARD16 *page_LF;
	mesa::CARD16 *page_GF;

	mesa::CARD16 GFI_GF;
	mesa::CARD16 GFI_SD;
	mesa::CARD16 GFI_ETT;
	mesa::CARD16 GFI_EFC;

	mesa::CARD16 pc_SD;
	mesa::CARD16 pc_ETT;

public:
	void setUp();
	void tearDown();
};
