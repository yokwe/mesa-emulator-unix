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


#include "../util/Util.h"
static const Logger logger = Logger::getLogger("testUtil");

#include "testBase.h"

#include "../util/Util.h"

class testUtil : public testBase {

	CPPUNIT_TEST_SUITE(testUtil);

	CPPUNIT_TEST(testToIntMesaNumber);

	CPPUNIT_TEST_SUITE_END();


public:
	void testToIntMesaNumber() {
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(std::string("0x10")));
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(std::string("10H")));
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(std::string("20B")));
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(std::string("16")));

		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(QString("0x10")));
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(QString("10H")));
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(QString("20B")));
		CPPUNIT_ASSERT_EQUAL(16, toIntMesaNumber(QString("16")));
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(testUtil);
