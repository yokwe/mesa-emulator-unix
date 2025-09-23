/*******************************************************************************
 * Copyright (c) 2025, Yasuhiro Hasegawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

//
// MesaGuam_event.cpp
//

#include "../util/Util.h"
static const Logger logger(__FILE__);

#include "../util/tcl.h"

#include "../mesa/Pilot.h"
#include "../mesa/guam.h"

#include "keymap.h"

#include "tclMesa.h"


// event handler
static void keyPress     (int keySymNumber, const std::string& keySymString);
static void keyRelease   (int keySymNumber, const std::string& keySymString);
static void buttonPress  (int buttonNumber);
static void buttonRelease(int buttonNumber);
static void motion       (int x, int y);

int MesaEvent(ClientData cdata, Tcl_Interp *interp_, int objc, Tcl_Obj *const objv[]) {
    tcl::Interp interp(interp_);
    std::string command = tcl::toString(objv[0]);
    std::string subCommand;
    if (2 <= objc) subCommand.append(tcl::toString(objv[1]));

    // bind .mesa.display <Motion>        { mesa::event motion %x %y }
    // bind .mesa.display <KeyPress>      { mesa::event keyPress %N %K }
    // bind .mesa.display <KeyRelease>    { mesa::event keyRelease %N %K }
    // bind .mesa.display <ButtonPress>   { mesa::event buttonPress %b }
    // bind .mesa.display <ButtonRelease> { mesa::event buttonRelease %b }
    if (subCommand == "motion" && objc == 4) {
        // mesa::guam motion x y
        // 0          1      2 3
        int status;
        auto x = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto y = toInt(interp, objv[3], status);
        if (status != TCL_OK) return status;
        motion(x, y);
        return TCL_OK;
    }
    if (subCommand == "keyPress" && objc == 4) {
        // mesa::guam keyPress keySymNumber keySymString
        // 0          1        2            3
        int status;
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
        keyPress(keySymNumber, keySymString);
        return TCL_OK;
    }
    if (subCommand == "keyRelease" && objc == 4) {
        // mesa::guam keyRelease keySymNumber keySymString
        // 0          1        2            3
        int status;
        auto keySymNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        auto keySymString = tcl::toString(objv[3]);
        keyRelease(keySymNumber, keySymString);
        return TCL_OK;
    }
    if (subCommand == "buttonPress" && objc == 3) {
        // mesa::guam buttonPress buttonNumber
        // 0          1           2
        int status;
        auto buttonNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        buttonPress(buttonNumber);
        return TCL_OK;
    }
    if (subCommand == "buttonRelease" && objc == 3) {
        // mesa::guam buttonRelease buttonNumber
        // 0          1             2
        int status;
        auto buttonNumber = toInt(interp, objv[2], status);
        if (status != TCL_OK) return status;
        buttonRelease(buttonNumber);
        return TCL_OK;
    }

    return invalidCommand(cdata, interp_, objc, objv);
}



//
// key event
//
void keyPress(int number, const std::string& string) {
	(void)string;
	const keymap::LevelVKey& levelVKey = keymap::getLevelVKey(number);
//	logger.info("keyPress    %4X  %-12s  %3d %s", number, string, levelVKey.keyName, levelVKey.name);
	auto keyName = levelVKey.keyName;
	if (keyName != LevelVKeys::null) guam::keyPress(keyName);
}
void keyRelease(int number, const std::string& string) {
	(void)string;
	const keymap::LevelVKey& levelVKey = keymap::getLevelVKey(number);
//	logger.info("keyRelease  %4X  %-12s  %3d %s", number, string, levelVKey.keyName, levelVKey.name);
	auto keyName = levelVKey.keyName;
	if (keyName != LevelVKeys::null) guam::keyRelease(keyName);
}

//
// mouse event
//
// mouse button number start from 1
static std::array<LevelVKeys::KeyName, 6> buttonMap = {
	LevelVKeys::null,
	LevelVKeys::Point,
	LevelVKeys::Adjust,
	LevelVKeys::Menu,
	LevelVKeys::null,
	LevelVKeys::null,
};
static void buttonPress(int number) {
	LevelVKeys::KeyName keyName = buttonMap[number];
//	logger.info("buttonPress    %d  %3d", number, keyName);
	if (keyName != LevelVKeys::null) {
		guam::keyPress(keyName);
	} else {
		logger.error("Unexpeced mouse button  %d", number);
		ERROR();
	}
}
static void buttonRelease(int number) {
	LevelVKeys::KeyName keyName = buttonMap[number];
//	logger.info("buttonRelease  %d  %3d", number, keyName);
	if (keyName != LevelVKeys::null) {
		guam::keyRelease(keyName);
	} else {
		logger.error("Unexpeced mouse button  %d", number);
		ERROR();
	}
}
static void motion(int x, int y) {
//	logger.info("motion      %4d  %4d", x, y);
	guam::setPosition(x, y);
}
