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
// Authentication3.h
//

#pragma once

#include "../xns/XNS.h"

#include "Type.h"

namespace Courier::Authentication3 {
	const quint32 PROGRAM = 14;
	const quint16 VERSION = 3;

	//  Authentication: PROGRAM 14 VERSION 3
	//  DEPENDS UPON Time(15) VERSION 2,
	//               Clearinghouse(2) VERSION 3;

	//	-- TYPES --
	//
	//	-- Types supporting encoding --
	//
	//	Key: TYPE = RECORD [value: ARRAY 4 OF UNSPECIFIED];  -- lsb of each octet is odd parity bit --
	//
	//	Block: TYPE = RECORD [value: ARRAY 4 OF UNSPECIFIED];  -- cipher text or plain text block --
	//
	//	HashedPassword: TYPE = CARDINAL;
	//
	//	-- Types describing credentials and verifiers --
	//
	//	CredentialsType: TYPE = {simple(0), strong(1)};
	//
	//	simpleCredentials: CredentialsType = simple;
	//
	//	Credentials: TYPE = RECORD [type: CredentialsType,
	//				    value: SEQUENCE OF UNSPECIFIED];
	//
	//	-- nullCredentials doesn't work yet --
	//	-- nullCredentials: Credentials = [type: simple, value: []]; --
	//
	//	CredentialsPackage: TYPE = RECORD [
	//		credentials: Credentials,
	//		nonce: LONG CARDINAL,
	//		recipient: Clearinghouse.Name,
	//		conversationKey: Key ];
	//
	//	-- instances of the following type must be a multiple of 64 bits, padded --
	//	-- with zeros, before encryption --
	//
	//	StrongCredentials: TYPE = RECORD [
	//		conversationKey: Key,
	//		expirationTime: Time.Time,
	//		initiator: Clearinghouse.Name ];
	//
	//	SimpleCredentials: TYPE = Clearinghouse.Name;
	//
	//	Verifier: TYPE = SEQUENCE 12 OF UNSPECIFIED;
	//
	//	StrongVerifier: TYPE = RECORD [
	//		timeStamp: Time.Time,
	//		ticks: LONG CARDINAL ];
	//
	//	SimpleVerifier: TYPE = HashedPassword;
	//
	//	Proxy: TYPE = SEQUENCE OF UNSPECIFIED;
	//
	//	-- instances of the following type must be a multiple of 64 bits, padded --
	//	-- with zeros, before encryption --
	//
	//	StrongProxy: TYPE = RECORD [
	//		randomBits: Block,
	//		expirationTime: Time.Time,
	//		agent: Clearinghouse.Name];
	//
	//	SimpleProxy: TYPE = BOOLEAN;
	//
	//	-- ERRORS --
	//
	//	Problem: TYPE = {
	//	    credentialsInvalid(0),		-- credentials unacceptable --
	//	    verifierInvalid(1),			-- verifier unacceptable --
	//	    verifierExpired(2),			-- the verifier was too old --
	//	    verifierReused(3),			-- the verifier has been used before --
	//	    credentialsExpired(4),		-- the credentials have expired --
	//	    inappropriateCredentials(5),	-- passed strong, wanted simple, or vica versa --
	//	    proxyInvalid(6),			-- proxy has invalid format --
	//	    proxyExpired(7),			-- the proxy was too old --
	//	    otherProblem(8) };
	//	AuthenticationError: ERROR[problem: Problem] = 2;
	//
	//	CallProblem: TYPE = {
	//	    tooBusy(0),				-- server is too busy to service this request --
	//	    accessRightsInsufficient(1),	-- operation prevented by access controls --
	//	    keysUnavailable(2),			-- the server which holds the required key was inaccessible --
	//	    strongKeyDoesNotExist(3),		-- a strong key critical to this operation has not been registered --
	//	    simpleKeyDoesNotExist(4),		-- a simple key critical to this operation has not been registered --
	//	    strongKeyAlreadyRegistered(5),	-- cannot create a strong key for an entity which already has one --
	//	    simpleKeyAlreadyRegistered(6),	-- cannot create a simple key for an entity which already has one --
	//	    domainForNewKeyUnavailable(7),	-- cannot create a new key because the domain to hold it is unaccessible --
	//	    domainForNewKeyUnknown(8),		-- cannot create a new key because the domain to hold it is unknown --
	//	    badKey(9),				-- bad key passed to CreateStrongKey or ChangeStrongKey --
	//	    badName(10),			-- bad name passed to CreateStrongKey or ChangeStrongKey --
	//	    databaseFull(11),			-- no more data can be added to the Authentication database --
	//	    otherCallProblem(12) };
	//	Which: TYPE = {notApplicable(0), initiator(1), recipient(2), agent(3) };
	//	CallError: ERROR [problem: CallProblem, whichArg: Which] = 1;
	//
	//
	//	-- PROCEDURES --
	//
	//	-- Strong Authentication --
	//
	//	GetStrongCredentials: PROCEDURE [
	//			initiator, recipient: Clearinghouse.Name,
	//			nonce: LONG CARDINAL ]
	//		RETURNS [ credentialsPackage: SEQUENCE OF UNSPECIFIED ]
	//			-- encrypted with the initiator's strong key --
	//		REPORTS [ CallError ] = 1;
	//
	//	TradeProxyForCredentials: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			initiator: Clearinghouse.Name, proxy: Proxy,
	//			recipient: Clearinghouse.Name, nonce: LONG CARDINAL ]
	//		RETURNS [ credentialsPackage: SEQUENCE OF UNSPECIFIED,
	//			-- enxrypted with the agent's strong key --
	//			  proxyForRecipient: Proxy ]
	//		REPORTS [ AuthenticationError, CallError ] = 9;
	//
	//	CreateStrongKey: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			name: Clearinghouse.Name, encryptedKey: Block ]
	//		REPORTS [ AuthenticationError, CallError ] = 3;
	//
	//	ChangeStrongKey: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			encryptedNewKey: Block ]
	//		REPORTS [ AuthenticationError, CallError ] = 4;
	//
	//	DeleteStrongKey: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			name: Clearinghouse.Name ]
	//		REPORTS [ AuthenticationError, CallError ] = 5;
	//
	//
	//	-- Simple Authentication --
	//
	//	CheckSimpleCredentials: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier ]
	//		RETURNS [ ok: BOOLEAN, initiator: Clearinghouse.Name ]
	//		REPORTS [ AuthenticationError, CallError ] = 2;
	//
	//	CreateSimpleKey: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			name: Clearinghouse.Name, key: HashedPassword ]
	//		REPORTS [ AuthenticationError, CallError ] = 6;
	//
	//	ChangeSimpleKey: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			newKey: HashedPassword ]
	//		REPORTS [ AuthenticationError, CallError ] = 7;
	//
	//	DeleteSimpleKey: PROCEDURE [
	//			credentials: Credentials, verifier: Verifier,
	//			name: Clearinghouse.Name ]
	//		REPORTS [ AuthenticationError, CallError ] = 8;
	//
	//	-- Undocumented  I assume this is same as proc0 in Clearinghouse2/3
	//	RetrieveAddresses: PROCEDURE
	//		RETURNS [address: Clearinghouse.NetworkAddressList]
	//		REPORTS [CallError] = 0;

}
