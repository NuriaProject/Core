/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#include "test.hpp"

#include <iostream>
#include <map>

struct TestData {
	std::string name;
	Nuria::TestFunction func;
};

typedef std::multimap< std::string, TestData > TestMap;
typedef TestMap::value_type TestPair;

static TestMap g_tests;

// 
void Nuria::addTestCase (const std::string &file, const std::string &name, Nuria::TestFunction func) {
	g_tests.insert (std::pair< std::string, TestData > (file, { name, func }));
}

static void printFileLine (const std::string &fileName) {
	std::cout << "- " << fileName << std::endl;
}

static void printTestLine (const std::string &testName, bool success) {
	std::cout << "   " << testName << ": ";
	std::cout << (success ? "OK" : "FAIL");
	std::cout << std::endl;
}

static bool runTestWithOutput (const TestData &testData) {
	bool success = testData.func ();
	printTestLine (testData.name, success);
	return success;
}

bool Nuria::runSingleTest (const std::string &file, const std::string &name) {
	TestMap::const_iterator it = g_tests.lower_bound (file);
	TestMap::const_iterator end = g_tests.upper_bound (file);
	
	for (; it != end; ++it) {
		if (it->first == file && it->second.name == name) {
			return runTestWithOutput (it->second);
		}
		
	}
	
	return false;
}

bool Nuria::runTestCases (int argc, char **argv) {
	bool success = true;
	int testCount = 0;
	int failCount = 0;
	std::string lastFile;
	
	for (const TestPair &cur : g_tests) {
		testCount++;
		
		if (cur.first != lastFile) {
			lastFile = cur.first;
			printFileLine (lastFile);
		}
		
		if (!runTestWithOutput (cur.second)) {
			failCount++;
			success = false;
		}
		
	}
	
	// 
	std::cout << "== Test run complete (" << testCount << " total, "
		  << failCount << " failed)" << std::endl;
	std::cout << "=> Result: " << (success ? "success" : "fail") << std::endl;
	
	return success;
}
