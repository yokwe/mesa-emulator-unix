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
// Authentication1.h
//

#pragma once

#include "../xns/XNS.h"

#include "Service.h"
#include "Type.h"

namespace Courier::Authentication1 {
	const quint32 PROGRAM = 14;
	const quint16 VERSION = 1;

	//  Authentication: PROGRAM 14 VERSION 1
	//  DEPENDS UPON Clearinghouse(2) VERSION 2

	//	CredentialsType: TYPE = CARDINAL;
	//
	//	Credentials: TYPE = RECORD[
	//		type: CredentialsType,
	//		value: SEQUENCE OF UNSPECIFIED];
	//
	//	simpleCredentials: CredentialsType = 0;
	//
	//	SimpleCredentials: TYPE = Clearinghouse.Name;
	//
	//	Verifier: TYPE = SEQUENCE 12 OF UNSPECIFIED;
	//
	//	HashedPassword: TYPE = CARDINAL;
	//
	//	SimpleVerifier: TYPE = HashedPassword;
	//
	//	-- remote errors --
	//
	//	Which: TYPE = {notApplicable(0), initiator(1), recipient(2), client(3)};
	//	CallProblem: TYPE = CARDINAL;
	//	CallError: ERROR [problem: CallProblem, whichArg: Which] = 1;
	//
	//	Problem: TYPE = {credentialsInvalid(0), verifierInvalid(1),
	//		verifierExpired(2), verifierReused(3), credentialsExpired(4),
	//		inappropriateCredentials(5)
	//		};
	//	AuthenticationError: ERROR [problem: Problem] = 2;

}
