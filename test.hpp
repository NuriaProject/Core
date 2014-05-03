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

#ifndef NURIA_TEST_HPP
#define NURIA_TEST_HPP

#include <functional>
#include <string>

namespace Nuria {

typedef std::function< bool() > TestFunction;

/**
 * Adds \a func as test-case \a name in \a file to the list of to-be-run tests.
 * \note You're probably looking for NURIA_TEST.
 */
void addTestCase (const std::string &file, const std::string &name,
		  TestFunction func);

/**
 * Runs the test-case called \a name in \a file. Returns \c true if it
 * succeeded or \c false if it did not. If \a name doesn't exist, the call will
 * fail.
 */
bool runSingleTest (const std::string &file, const std::string &name);

/**
 * Runs all test-cases. Returns \c true if all succeeded. If a single one
 * failed \c false is returned.
 */
bool runTestCases (int argc, char **argv);

}

#if defined(NURIA_TEST_RUN)

// Helper macros
#define NURIA_TEST_CLASS_NAME(Name) TestClass_ ## Name
#define NURIA_TEST_CLASS_INSTANCE(Name) \
	static TestClass_ ## Name TestInstance_ ## Name

#if defined(QT_WIDGETS_LIB)
#include <QApplication>
#define NURIA_TEST_MAIN_PROLOGUE QApplication a (argc, argv);
#elif defined(QT_GUI_LIB)
#include <QGuiApplication>
#define NURIA_TEST_MAIN_PROLOGUE QGuiApplication a (argc, argv);
#elif defined(QT_CORE_LIB)
#include <QCoreApplication>
#define NURIA_TEST_MAIN_PROLOGUE QCoreApplication a (argc, argv);
#else
#define NURIA_TEST_MAIN_PROLOGUE
#endif


/**
 * Defines a test-case called \a Name with test code \a Code. \a Code must be
 * enclosed in curly braces ("{}") and must return \c true on success and
 * \c false on failure. \a Name must be a legal C++ symbol name.
 * 
 * \par Usage
 * NURIA_TEST is intended to be put directly into the code of the code that
 * you want to test, making it easy to write both white-box (Where you can also
 * see the internal structure) and black-box (Where you can't easily see
 * internal data) tests.
 * 
 * If you want to guard further test-code, test if NURIA_TEST_RUN is defined:
 * \codeline #ifdef NURIA_TEST_RUN ... #endif
 * 
 * \par Example
 * \code
 * int add (int a, int b) { return a + b; }
 * NURIA_TEST(verifyAddition, { return add (1, 2) == 3; });
 * NURIA_TEST_MAIN();
 * \endcode
 */
#define NURIA_TEST(Name, ...) \
	namespace NuriaTest { \
	struct NURIA_TEST_CLASS_NAME(Name) { \
	NURIA_TEST_CLASS_NAME(Name) () { \
		Nuria::addTestCase (__FILE__, #Name, test); \
	} \
	static bool test () __VA_ARGS__ \
	}; \
	NURIA_TEST_CLASS_INSTANCE(Name); \
	}

/**
 * Implements a minimalist main() which will run all test-cases in the project.
 * Uses Qt defines to determine which kind of Q*Application is needed by your
 * application - Or none. It supports applications using QtWidgets, QtGui and
 * QtCore libraries out of the box (All other Qt libraries don't come with a
 * Q*Application class, thus they're supported right away).
 */
#define NURIA_TEST_MAIN() \
	int main (int argc, char **argv) { \
	NURIA_TEST_MAIN_PROLOGUE \
	if (!Nuria::runTestCases (argc, argv)) { \
	return 1; \
	} \
	return 0; \
	}

#else

// Dummy macros
#define NURIA_TEST(Name, Code, ...)
#define NURIA_TEST_MAIN()

#endif

#endif // NURIA_TEST_HPP
