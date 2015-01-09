/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDateTime>
#include <QRegExp>
#include <QString>
#include <QtTest>

#define NURIA_MODULE "Test"
#include <nuria/debug.hpp>

class DebugTest : public QObject {
	Q_OBJECT
	
public:
	
private slots:
	
	void testDefaultOutput ();
	void testCustomFormatOutput ();
	void testCustomOutputHandler ();
	void testDisableAllOutput ();
	void testDisablePartialOutput ();
	void testModuleDisableAll ();
	void testModuleDisablePartial ();
	void testQtMessageHandler ();
	
};

void DebugTest::testDefaultOutput () {
	// "[%TIME%] %TYPE%/%MODULE%: %FILE%:%LINE% - %CLASS%::%METHOD%: %MESSAGE%"
	QString time = QDateTime::currentDateTime ().toString ("[HH:mm:ss] ");
	
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Nuria::Debug::setDestination (buffer);
	QString theLine = QString::number (__LINE__ + 2); // Line of nDebug()
	QString expected("Debug/Test: tst_debug.cpp:" + theLine + " - DebugTest::testDefaultOutput: hi\n");
	nDebug() << "hi";
	
	expected.prepend (time.toLatin1 ());
	QCOMPARE(QString (buffer->data ()), expected);
}

void DebugTest::testCustomFormatOutput () {
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Nuria::Debug::setDestination (buffer);
	
	QString time = QDateTime::currentDateTime ().toString (" MM/dd/yyyy HH:mm:ss ");
	Nuria::Debug::setOutputFormat (" %DATE% %TIME% %FILE% %MODULE% %LINE% %METHOD% %CLASS% %BODY%");
	QString theLine = QString::number (__LINE__ + 2); // Line of nWarn()
	QString expected(time + "tst_debug.cpp Test " + theLine + " testCustomFormatOutput DebugTest hi\n");
	nWarn() << "hi";
	
	QCOMPARE(QString (buffer->data ()), expected);
}

void DebugTest::testCustomOutputHandler () {
	Nuria::Debug::Type type;
	QByteArray typeName;
	QByteArray moduleName;
	QByteArray file;
	QByteArray className;
	QByteArray methodName;
	QByteArray message;
	int line;
	
	auto func = [&](Nuria::Debug::Type type_, QByteArray typeName_, QByteArray moduleName_,
			QByteArray file_, int line_, QByteArray className_, QByteArray methodName_,
			QByteArray message_) {
		type = type_;
		typeName = typeName_;
		moduleName = moduleName_;
		file = file_;
		line = line_;
		className = className_;
		methodName = methodName_;
		message = message_;
	};
	
	Nuria::Callback cb = Nuria::Callback::fromLambda (func);
	Nuria::Debug::installOutputHandler (cb);
	Nuria::Debug::setOutputDisabled (true);
	
	const char *expectedMessage = "NuriaFramework";
	
	int expectedLine = __LINE__; nLog() << expectedMessage;
	
	QCOMPARE(type, Nuria::Debug::LogMsg);
	QCOMPARE(typeName.data (), "Log");
	QCOMPARE(moduleName.data (), NURIA_MODULE);
	QCOMPARE(file.data (), "tst_debug.cpp");
	QCOMPARE(line, expectedLine);
	QCOMPARE(className.data (), "DebugTest");
	QCOMPARE(methodName.data (), "testCustomOutputHandler");
	QCOMPARE(message.data (), expectedMessage);
	
	Nuria::Debug::uninstallOutputHandler (cb);
	Nuria::Debug::setOutputDisabled (false);
}

void DebugTest::testDisableAllOutput () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Debug::setDestination (buffer);
	
	Debug::setModuleLevel (nullptr, Debug::AllLevels);
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QVERIFY(buffer->data ().isEmpty ());
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::DebugMsg));
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::LogMsg));
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::WarnMsg));
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::ErrorMsg));
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::CriticalMsg));
	
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::DebugMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::LogMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::WarnMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::ErrorMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::CriticalMsg));
	Debug::setModuleLevel (nullptr, Debug::DefaultLowestMsgLevel);
}

void DebugTest::testDisablePartialOutput () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Debug::setDestination (buffer);
	
	Debug::setModuleLevel (nullptr, Debug::ErrorMsg);
	Debug::setOutputFormat ("%TYPE%");
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QCOMPARE(buffer->data ().data (), "Error\nCritical\n");
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::DebugMsg));
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::LogMsg));
	QVERIFY(Debug::isModuleDisabled (nullptr, Debug::WarnMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::ErrorMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::CriticalMsg));
	
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::DebugMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::LogMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::WarnMsg));
	QVERIFY(!Debug::isModuleDisabled (NURIA_MODULE, Debug::ErrorMsg));
	QVERIFY(!Debug::isModuleDisabled (NURIA_MODULE, Debug::CriticalMsg));
	Debug::setModuleLevel (nullptr, Debug::DefaultLowestMsgLevel);
	
}

void DebugTest::testModuleDisableAll () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Debug::setDestination (buffer);
	
	Debug::setModuleLevel (NURIA_MODULE, Debug::AllLevels);
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QVERIFY(buffer->data ().isEmpty ());
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::DebugMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::LogMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::WarnMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::ErrorMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::CriticalMsg));
	
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::DebugMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::LogMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::WarnMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::ErrorMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::CriticalMsg));
	Debug::setModuleLevel (NURIA_MODULE, Debug::DefaultLowestMsgLevel);
}

void DebugTest::testModuleDisablePartial () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Debug::setDestination (buffer);
	
	Debug::setModuleLevel (NURIA_MODULE, Debug::WarnMsg);
	Debug::setOutputFormat ("%TYPE%");
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QCOMPARE(buffer->data ().data (), "Warning\nError\nCritical\n");
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::DebugMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::LogMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::WarnMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::ErrorMsg));
	QVERIFY(!Debug::isModuleDisabled (nullptr, Debug::CriticalMsg));
	
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::DebugMsg));
	QVERIFY(Debug::isModuleDisabled (NURIA_MODULE, Debug::LogMsg));
	QVERIFY(!Debug::isModuleDisabled (NURIA_MODULE, Debug::WarnMsg));
	QVERIFY(!Debug::isModuleDisabled (NURIA_MODULE, Debug::ErrorMsg));
	QVERIFY(!Debug::isModuleDisabled (NURIA_MODULE, Debug::CriticalMsg));
	Debug::setModuleLevel (NURIA_MODULE, Debug::DefaultLowestMsgLevel);
}

void DebugTest::testQtMessageHandler () {
	Nuria::Debug::Type type;
	QByteArray typeName;
	QByteArray moduleName;
	QByteArray file;
	QByteArray className;
	QByteArray methodName;
	QByteArray message;
	int line;
	
	auto func = [&](Nuria::Debug::Type type_, QByteArray typeName_, QByteArray moduleName_,
			QByteArray file_, int line_, QByteArray className_, QByteArray methodName_,
			QByteArray message_) {
		type = type_;
		typeName = typeName_;
		moduleName = moduleName_;
		file = file_;
		line = line_;
		className = className_;
		methodName = methodName_;
		message = message_;
	};
	
	Nuria::Callback cb = Nuria::Callback::fromLambda (func);
	Nuria::Debug::installOutputHandler (cb);
	Nuria::Debug::setOutputDisabled (true);
	
	const char *expectedMessage = "NuriaFramework";
	
	Nuria::Debug::installMessageHandler ();
	int expectedLine = __LINE__; qCritical() << expectedMessage;
	
	QCOMPARE(type, Nuria::Debug::CriticalMsg);
	QCOMPARE(typeName.data (), "Critical");
	QCOMPARE(moduleName.data (), "default");
	QCOMPARE(message.data (), expectedMessage);
	
#ifdef QT_DEBUG
	QCOMPARE(methodName.data (), "testQtMessageHandler");
	QCOMPARE(className.data (), "DebugTest");
	QCOMPARE(file.data (), "tst_debug.cpp");
	QCOMPARE(line, expectedLine);
#else
	QCOMPARE(methodName.data (), "");
	QCOMPARE(className.data (), "");
	QCOMPARE(file.data (), "");
	QCOMPARE(line, 0);
#endif
	
	Nuria::Debug::uninstallOutputHandler (cb);
	Nuria::Debug::setOutputDisabled (false);
}

QTEST_APPLESS_MAIN(DebugTest)
#include "tst_debug.moc"
