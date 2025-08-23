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
// Clearinghouse2.h
//

#pragma once

#include "../util/NameMap.h"

#include "../xns/XNS.h"

#include "Type.h"

#include "Authentication1.h"
#include "BulkData.h"


namespace Courier::Clearinghouse2 {
	const uint32_t PROGRAM = 2;
	const uint16_t VERSION = 2;

	using XNS::Net;
	using XNS::Host;
	using XNS::Socket;


	//  Clearinghouse: PROGRAM 2 VERSION 2
	//	DEPENDS UPON
	//			BulkData(0) VERSION 1,
	//			Authentication (14) VERSION 1;

	namespace Auth = Courier::Authentication1;

	//	-- TYPES AND CONSTANTS DESCRIBING NAMES --
	//
	//	Organization: TYPE = STRING;
	typedef STRING Organization;

	//	Domain: TYPE = STRING;
	typedef STRING Domain;

	//	Object: TYPE = STRING;
	typedef STRING Object;

	//	maxOrganizationsLength: CARDINAL = 20; -- in bytes --
	static const int MAX_ORGANIZATION_LENGTH = 20;

	//	maxDomainLength: CARDINAL = 20; -- in bytes --
	static const int MAX_DOMAIN_LENGTH = 20;

	//	maxObjectLength: CARDINAL = 40; -- in bytes --
	static const int MAX_OBJECT_LENGTH = 40;

	//	-- There can be no wildcard characters in any of the following types. --
	//	OrganizationName: TYPE = Organization;
	typedef Organization OrganizationName;

	//	TwoPartName: TYPE = RECORD [
	//		organization: Organization,
	//		domain: Domain];
	class TwoPartName : public Base {
	public:
		Organization organization;
		Domain       domain;

		std::string toString() const;

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb);
		void toByteBuffer  (ByteBuffer& bb) const;
	};

	//	DomainName: TYPE = TwoPartName;
	typedef TwoPartName DomainName;

	//	ThreePartName: TYPE = RECORD [
	//		organization: Organization,
	//		domain: Domain,
	//		object: Object];
	class ThreePartName : Base {
	public:
		Organization organization;
		Domain       domain;
		Object       object;

		std::string toString() const {
			return std::string("%1-%2-%3").arg(organization.toString()).arg(domain.toString()).arg(object.toString());
		}

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb) {
			FROM_BYTE_BUFFER(bb, organization);
			FROM_BYTE_BUFFER(bb, domain);
			FROM_BYTE_BUFFER(bb, object);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			TO_BYTE_BUFFER(bb, organization);
			TO_BYTE_BUFFER(bb, domain);
			TO_BYTE_BUFFER(bb, object);
		}
	};

	//	ObjectName: TYPE = ThreePartName;
	typedef ThreePartName ObjectName;

	//	Name: TYPE = ThreePartName;
	typedef ThreePartName Name;

	//	-- Wildcard characters are permitted in OrganizationNamePatterns. --
	//	OrganizationNamePattern: TYPE = Organization;
	typedef Organization OrganizationNamePattern;

	//	-- Wildcard characters are permitted in the domain component of this type,
	//	-- but not in the organization component.
	//	DomainNamePattern: TYPE = TwoPartName;
	typedef TwoPartName DomainNamePattern;

	//	-- Wildcard characters are permitted in the object component of this type,
	//	-- but not in the organization and domain components.
	//	ObjectNamePattern: TYPE = ThreePartName;
	typedef ThreePartName ObjectNamePattern;

	//	-- TYPES AND CONSTANTS DESCRIBING BULK PARAMETERS --
	class StreamOfChoice : public UINT16 {
	public:
		enum Value : uint16_t {
			NEXT_SEGMENT = 0,
			LAST_SEGMENT = 1,
		};

		// define operator =
		uint16_t operator =(const uint16_t& newValue) const {
			value(newValue);
			return newValue;
		}

		std::string toString() const {
			return nameMap.toString(value());
		}
	private:
		static NameMap::Map<uint16_t> nameMap;
	};

	template <class T>
	class StreamOf : public Base {
		std::vector<T> list;
	public:
		StreamOf& operator = (const std::vector<T>& that) {
			list.clear();
			list.append(that);
			return *this;
		}
		void clear() {
			list.clear();
		}
		int size() const {
			return list.size();
		}
		void append(const T& newValue) {
			list.append(newValue);
		}


		std::string toString() const {
			std::stringList myList;
			for(auto e: list) {
				myList.append(std::string("{%1}").arg(e.toString()));
			}
			return std::string("(%1) {%2}").arg(myList.length()).arg(myList.join(", "));
		}

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb) {
			StreamOfChoice choice;
			SEQUENCE<T>    sequence;

			list.clear();
			for(;;) {
				FROM_BYTE_BUFFER(bb, choice);
				FROM_BYTE_BUFFER(bb, sequence);

				if ((uint16_t)choice == StreamOfChoice::LAST_SEGMENT) break;
				if ((uint16_t)choice == StreamOfChoice::NEXT_SEGMENT) continue;

				logger.error("choice %d", (uint16_t)choice);
				ERROR();
			}
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			StreamOfChoice choice;
			SEQUENCE<T>    sequence;

			choice = StreamOfChoice::LAST_SEGMENT;
			sequence.append(list);

			TO_BYTE_BUFFER(bb, choice);
			TO_BYTE_BUFFER(bb, sequence);
		}
	};

	//	StreamOfDomain: TYPE = CHOICE OF {
	//		nextSegment (0) => RECORD [
	//			segment: SEQUENCE OF Domain,
	//			restOfStream: StreamOfDomain],
	//		lastSegment (1) => SEQUENCE OF Domain};
	typedef StreamOf<Domain> StreamOfDomain;

	//	StreamOfDomainName: TYPE = CHOICE OF {
	//		nextSegment (0) => RECORD [
	//			segment: SEQUENCE OF DomainName,
	//			restOfStream: StreamOfDomainName],
	//		lastSegment (1) => SEQUENCE OF DomainName};
	typedef StreamOf<DomainName> StreamOfDomainName;

	//	StreamOfObject: TYPE = CHOICE OF {
	//		nextSegment (0) => RECORD [
	//			segment: SEQUENCE OF Object,
	//			restOfStream: StreamOfObject],
	//		lastSegment (1) => SEQUENCE OF Object};
	typedef StreamOf<Object> StreamOfObject;

	//	StreamOfObjectName: TYPE = CHOICE OF {
	//		nextSegment (0) => RECORD [
	//			segment: SEQUENCE OF ObjectName,
	//			restOfStream: StreamOfObjectName],
	//		lastSegment (1) => SEQUENCE OF ObjectName};
	typedef StreamOf<ObjectName> StreamOfObjectName;

	//	StreamOfOrganization: TYPE = CHOICE OF {
	//		nextSegment (0) => RECORD [
	//			segment: SEQUENCE OF Organization,
	//			restOfStream: StreamOfOrganization],
	//		lastSegment (1) => SEQUENCE OF Organization};
	typedef StreamOf<Organization> StreamOfOrganization;

	//	StreamOfThreePartName: TYPE = CHOICE OF {
	//		nextSegment (0) => RECORD [
	//			segment: SEQUENCE OF ThreePartName,
	//			restOfStream: StreamOfThreePartName],
	//		lastSegment (1) => SEQUENCE OF ThreePartName};
	typedef StreamOf<ThreePartName> StreamOfThreePartName;

	//	-- TYPES AND CONSTANTS DESCRIBING PROPERTIES --
	//
	//	Property: TYPE = LONG CARDINAL;
	typedef UINT32 Property;

	//	-- A Name can have up to 250 Properties associated with it. --
	//	Properties: TYPE = SEQUENCE 250 OF Property;
	typedef SEQUENCE<Property, 250> Properties;

	//	all: Property = 0;
	const uint32_t ALL = 0;

	//	nullProperty: Property = 37777777777B;
	const uint32_t NULL_PROPERTY = ~0;

	//	-- The value associated with an item property. --
	//	Item: TYPE = SEQUENCE 500 OF UNSPECIFIED;
	typedef SEQUENCE<UINT16, 500> Item;

	//	-- TYPES AND CONSTANTS DESCRIBING NETWORK ADDRESSES --
	//
	//	-- Clearinghouse addresses aer stored in this form. --
	//
	//	NetworkAddress: TYPE = RECORD [
	//		network: ARRAY 2 OF UNSPECIFIED,
	//		host: ARRAY 3 OF UNSPECIFIED,
	//		socket: UNSPECIFIED ];
	class NetworkAddress : public Base {
	public:
		Net    net;
		Host   host;
		Socket socket;

		std::string toString() const {
			return std::string("{%1-%2-%3}").arg(net.toString()).arg(host.toString()).arg(socket.toString());
		}

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb) {
			FROM_BYTE_BUFFER(bb, net);
			FROM_BYTE_BUFFER(bb, host);
			FROM_BYTE_BUFFER(bb, socket);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			TO_BYTE_BUFFER(bb, net);
			TO_BYTE_BUFFER(bb, host);
			TO_BYTE_BUFFER(bb, socket);
		}
	};

	//
	//	NetworkAddressList: TYPE = SEQUENCE 40 OF NetworkAddress;
	typedef SEQUENCE<NetworkAddress, 40> NetworkAddressList;

	//
	//	-- OTHER TYPES AND CONSTANTS --
	//
	//	-- How the client identifies itself to the service --
	//	Authenticator: TYPE = RECORD [
	//		credentials: Authentication.Credentials,
	//		verifier: Authentication.Verifier];
	class Authenticator : public Base {
	public:
		Auth::Credentials credentials;
		Auth::Verifier    verifier;

		std::string toString() const {
			return std::string("(%1)-(%2)").arg(credentials.toString()).arg(verifier.toString());
		}

		// Courier::Base
		void fromByteBuffer(ByteBuffer& bb) {
			FROM_BYTE_BUFFER(bb, credentials);
			FROM_BYTE_BUFFER(bb, verifier);
		}
		void toByteBuffer  (ByteBuffer& bb) const {
			TO_BYTE_BUFFER(bb, credentials);
			TO_BYTE_BUFFER(bb, verifier);
		}
	};

	//	wildcard: STRING = "*"; -- the wildcard character (asterisk) --
	constexpr const char* WILDCARD = "*";

	//	-- ERRORS --
	//
	//	WhichArgument: TYPE = {
	//		first(1), -- concerns the first name or property argument --
	//		second(2) }; -- concerns the second name or property argument --
	//
	//	ArgumentProblem: TYPE = {
	//		illegalProperty(10), -- property is not usable by a client --
	//		illegalOrganizationName(11), -- the organization component of the name
	//			-- is incorrect, e.g., too long or short, or has wild card
	//			-- characters when not allowed --
	//		illegalDomainName(12), -- the domain component of the name
	//			-- is incorrect, e.g., too long or short, or has wild card
	//			-- characters when not allowed --
	//		illegalObjectName(13), -- the object component of the name
	//			-- is incorrect, e.g., too long or short, or has wild card
	//			-- characters when not allowed --
	//		noSuchOrganization(14), -- the name's organization component does not exist --
	//		noSuchDomain(15), -- the name's domain component does not exist --
	//		noSuchObject(16) }; -- the name's object component does not exist --
	//	ArgumentError: ERROR [problem: ArgumentProblem, which: WhichArgument] = 2;
	//
	//	AuthenticationError: ERROR [problem: Authentication.Problem] = 6;
	//
	//	CallProblem: TYPE = {
	//		accessRightsInsufficient(1), -- operation prevented by access controls --
	//		tooBusy(2), -- server is too busy to service this request --
	//		serverDown(3), -- a remote Clearinghouse server was down and was needed for this request --
	//		useCourier(4), -- server insists that Courier be used for this particular request --
	//		other(5) };
	//	CallError: ERROR [problem: CallProblem] = 1;
	//
	//	PropertyProblem: TYPE = {
	//		missing(20), -- the object exists, but the property doesn't --
	//		wrongType(21)}; -- client wanted a Group but it was an Item, or vice versa --
	//	PropertyError: ERROR [problem: PropertyProblem,
	//		distinguishedObject: ObjectName] = 3;
	//	UpdateProblem: TYPE = {
	//		noChange(30), -- operation wouldn't change the database --
	//		outOfDate(31), -- more recent information was in database --
	//		objectOverflow(32), -- the particular object will have too much data
	//			-- associated with it --
	//		databaseOverflow(33)}; -- the server has run out of room --
	//	UpdateError: ERROR [problem: UpdateProblem, found: BOOLEAN,
	//		which: WhichArgument, distinguishedObject: ObjectName] = 4;
	//
	//	WrongServer: ERROR [hint: ObjectName] = 5;
	//
	//	-- PROCEDURES --
	//
	//	-- DEALING WITH OBJECTS --
	//
	//	CreateObject: PROCEDURE [name: ObjectName, agent: Authenticator]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, UpdateError,
	//		WrongServer] = 2;
	//
	//	DeleteObject: PROCEDURE [name: ObjectName, agent: Authenticator]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, UpdateError,
	//		WrongServer] = 3;
	//
	//	LookupObject: PROCEDURE [name: ObjectNamePattern, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 4;
	//
	//	ListOrganizations: PROCEDURE [pattern: OrganizationNamePattern,
	//		list: BulkData.Sink, agent: Authenticator]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 5;
	//
	//	ListDomain: PROCEDURE [pattern: DomainNamePattern, list: BulkData.Sink,
	//		agent: Authenticator]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 6;
	//
	//	ListObjects: PROCEDURE [pattern: ObjectNamePattern, property: Property,
	//		list: BulkData.Sink, agent: Authenticator]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 7;
	//
	//	ListAliasesOf: PROCEDURE [pattern: ObjectNamePattern, list: BulkData.Sink,
	//		agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 9;
	//
	//	-- PROCEDURES DEALING WITH ALIASES --
	//
	//	CreateAlias: PROCEDURE [alias, sameAs: ObjectName, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, UpdateError,
	//		WrongServer] = 10;
	//
	//	DeleteAlias: PROCEDURE [alias: ObjectName, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, UpdateError,
	//		WrongServer] = 11;
	//
	//	ListAliases: PROCEDURE [pattern: ObjectNamePattern, list: BulkData.Sink,
	//		agent: Authenticator]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 8;
	//
	//	-- PROCEDURES DEALING WITH PROPERTIES --
	//
	//	DeleteProperty: PROCEDURE [name: ObjectName, property: Property,
	//		agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 14;
	//
	//	ListProperties: PROCEDURE [pattern: ObjectNamePattern, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName, properties: Properties]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, WrongServer] = 15;
	//
	//	-- PROCEDURES DEALING WITH THE ITEM PROPERTY --
	//
	//	AddItemProperty: PROCEDURE [name: ObjectName, newProperty: Property,
	//		value: Item, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 13;
	//
	//	RetrieveItem: PROCEDURE [pattern: ObjectNamePattern, property: Property,
	//		agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName, value: Item]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		WrongServer] = 16;
	//
	//	ChangeItem: PROCEDURE [name: ObjectName, property: Property, newValue: Item,
	//		agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 17;
	//
	//	-- PROCEDURES DEALING WITH THE GROUP PROPERTY --
	//
	//	AddGroupProperty: PROCEDURE [name: ObjectName, newProperty: Property,
	//		membership: BulkData.Source, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 12;
	//
	//	RetrieveMembers: PROCEDURE [pattern: ObjectNamePattern, property: Property,
	//		membership: BulkData.Sink, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		WrongServer] = 18;
	//
	//	AddMember: PROCEDURE [name: ObjectName, property: Property,
	//		newMember: ThreePartName, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 19;
	//
	//	AddSelf: PROCEDURE [name: ObjectName, property: Property, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 20;
	//
	//	DeleteMember: PROCEDURE [name: ObjectName, property: Property,
	//		member: ThreePartName, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 21;
	//
	//	DeleteSelf: PROCEDURE [name: ObjectName, property: Property, agent: Authenticator]
	//	RETURNS [distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		UpdateError, WrongServer] = 22;
	//
	//	IsMember: PROCEDURE [memberOf: ObjectNamePattern,
	//		property, secondaryProperty: Property, name: ThreePartName,
	//		agent: Authenticator]
	//	RETURNS [isMember: BOOLEAN, distinguishedObject: ObjectName]
	//	REPORTS [ArgumentError, AuthenticationError, CallError, PropertyError,
	//		WrongServer] = 23;
	//
	//	-- PROCEDURES DEALING WITH SERVERS --
	//
	//	RetrieveAddresses: PROCEDURE
	//	RETURNS [address: NetworkAddressList]
	//	REPORTS [CallError] = 0;
	class RetrieveAddress {
	public:
		const bool    USE_BULK = false;

		class Return : Base {
		public:
			NetworkAddressList address;

			// Courier::Base
			std::string toString() const {
				return address.toString();
			}
			void fromByteBuffer(ByteBuffer& bb) {
				FROM_BYTE_BUFFER(bb, address);
			}
			void toByteBuffer  (ByteBuffer& bb) const {
				TO_BYTE_BUFFER(bb, address);
			}
		};
	};


	//	ListDomainServed: PROCEDURE [domains: BulkData.Sink, agent: Authenticator]
	//	REPORTS [AuthenticationError, CallError] = 1;
	class ListDomainServed {
	public:
		class Call : public Base {
		public:
			BulkData::Sink domains;
			Authenticator  agent;

			// Courier::Base
			std::string toString() const {
				return std::string("(%1)-(%2)").arg(domains.toString()).arg(agent.toString());
			}
			void fromByteBuffer(ByteBuffer& bb) {
				FROM_BYTE_BUFFER(bb, domains);
				FROM_BYTE_BUFFER(bb, agent);
			}
			void toByteBuffer  (ByteBuffer& bb) const {
				TO_BYTE_BUFFER(bb, domains);
				TO_BYTE_BUFFER(bb, agent);
			}
		};
		// return data with SST == BULK
		// return data type is StreamOfDomainName
		class Return : public Base {
		public:
			StreamOfDomainName value;

			// Courier::Base
			std::string toString() const {
				return value.toString();
			}
			void fromByteBuffer(ByteBuffer& bb) {
				FROM_BYTE_BUFFER(bb, value);
			}
			void toByteBuffer  (ByteBuffer& bb) const {
				TO_BYTE_BUFFER(bb, value);
			}
		};
	};

}
