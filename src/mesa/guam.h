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
// guam.h
//

#pragma once

#include <string>

#include "Pilot.h"

namespace guam {

void setDiskPath(const std::string& diskPath_);
void setGermPath(const std::string& germPath_);
void setBootPath(const std::string& bootPath_);
void setFloppyPath(const std::string& floppyPath_);
void setBootSwitch(const std::string& bootSwitch_);
void setBootDevice(const std::string& bootDevice_);
void setMemorySize(int vmBits_, int rmBits_);
void setDisplaySize(CARD16 displayWidth_, CARD16 displayHeight_);
void setNetworkInterfaceName(const std::string& networkInterfaceName_);

void setBootRequestPV    (Boot::Request* request, CARD16 deviceOrdinal = 0);
void setBootRequestEther (Boot::Request* request, CARD16 deviceOrdinal = 0);
void setBootRequestStream(Boot::Request* request);

void setSwitches(System::Switches& switches, const char *string);

void initialize();
void boot(); // don't return until all child thread stopped
void finalize();

int64_t getElapsedTime();

}
