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

class LoggerTest : public QObject {
	Q_OBJECT
private slots:
	
	void testDefaultOutput ();
	void testCustomFormatOutput ();
	void testCustomOutputHandler ();
	void testDisableAllOutput ();
	void testDisablePartialOutput ();
	void testModuleDisableAll ();
	void testModuleDisablePartial ();
	void testQtMessageHandler ();
	
	void transactionsAreThreadLocal ();
	void verifyLoggerTransactionBehaviour ();
	
	void benchmark ();
	
};

void LoggerTest::testDefaultOutput () {
	// "[%TIME%] %TYPE%/%MODULE%: %FILE%:%LINE% - %CLASS%::%METHOD%: %MESSAGE%"
	QString time = QDateTime::currentDateTime ().toString ("[HH:mm:ss] ");
	
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Nuria::Logger::setOutputDevice (buffer);
	QString theLine = QString::number (__LINE__ + 2); // Line of nDebug()
	QString expected(" Debug/Test: tst_logger.cpp:" + theLine + " - LoggerTest::testDefaultOutput: hi\n");
	nDebug() << "hi";
	
	expected.prepend (time.toLatin1 ());
	QCOMPARE(buffer->data (), expected.toLatin1 ());
}

void LoggerTest::testCustomFormatOutput () {
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Nuria::Logger::setOutputDevice (buffer);
	Nuria::Logger::setTransaction ("Foo");
	
	QString time = QDateTime::currentDateTime ().toString (" MM/dd/yyyy HH:mm:ss ");
	Nuria::Logger::setOutputFormat (" %DATE% %TIME% %FILE% %MODULE% %LINE% %METHOD% %CLASS% %TRANSACTION% %BODY%");
	
	QString theLine = QString::number (__LINE__ + 2); // Line of nWarn()
	QString expected (time + "tst_logger.cpp Test " + theLine + " testCustomFormatOutput LoggerTest Foo hi\n");
	nWarn() << "hi";
	
	QCOMPARE(buffer->data (), expected.toLatin1 ());
	Nuria::Logger::setTransaction (QByteArray ());
}

void LoggerTest::testCustomOutputHandler () {
	Nuria::Logger::Type type;
	QByteArray typeName;
	QByteArray transaction;
	QByteArray moduleName;
	QByteArray file;
	QByteArray className;
	QByteArray methodName;
	QString message;
	int line;
	
	auto func = [&](Nuria::Logger::Type type_, const QByteArray &transaction_, const QByteArray &typeName_,
	                const QByteArray &moduleName_, const QByteArray &file_, int line_,
	                const QByteArray &className_, const QByteArray &methodName_, const QString &message_) {
		type = type_;
		typeName = typeName_;
		transaction = transaction_;
		moduleName = moduleName_;
		file = file_;
		line = line_;
		className = className_;
		methodName = methodName_;
		message = message_;
	};
	
	Nuria::Logger::setOutputHandler (func);
	Nuria::Logger::setOutputDisabled (true);
	
	const char *expectedMessage = "NuriaFramework";
	int expectedLine = __LINE__; nLog() << expectedMessage;
	
	QCOMPARE(type, Nuria::Logger::LogMsg);
	QCOMPARE(typeName, QByteArray ("Log"));
	QCOMPARE(transaction, QByteArray ());
	QCOMPARE(moduleName, QByteArray (NURIA_MODULE));
	QCOMPARE(file, QByteArray ("tst_logger.cpp"));
	QCOMPARE(line, expectedLine);
	QCOMPARE(className, QByteArray ("LoggerTest"));
	QCOMPARE(methodName, QByteArray ("testCustomOutputHandler"));
	QCOMPARE(message, QString (expectedMessage));
	
	Nuria::Logger::setOutputHandler (Nuria::Logger::Handler ());
	Nuria::Logger::setOutputDisabled (false);
}

void LoggerTest::testDisableAllOutput () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Logger::setOutputDevice (buffer);
	
	Logger::setModuleLevel (nullptr, Logger::AllLevels);
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QVERIFY(buffer->data ().isEmpty ());
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::DebugMsg));
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::LogMsg));
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::WarnMsg));
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::ErrorMsg));
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::CriticalMsg));
	
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::DebugMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::LogMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::WarnMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::ErrorMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::CriticalMsg));
	Logger::setModuleLevel (nullptr, Logger::DefaultLowestMsgLevel);
}

void LoggerTest::testDisablePartialOutput () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Logger::setOutputDevice (buffer);
	
	Logger::setModuleLevel (nullptr, Logger::ErrorMsg);
	Logger::setOutputFormat ("%TYPE%");
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QCOMPARE(buffer->data ().data (), "Error\nCritical\n");
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::DebugMsg));
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::LogMsg));
	QVERIFY(Logger::isModuleDisabled (nullptr, Logger::WarnMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::ErrorMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::CriticalMsg));
	
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::DebugMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::LogMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::WarnMsg));
	QVERIFY(!Logger::isModuleDisabled (NURIA_MODULE, Logger::ErrorMsg));
	QVERIFY(!Logger::isModuleDisabled (NURIA_MODULE, Logger::CriticalMsg));
	Logger::setModuleLevel (nullptr, Logger::DefaultLowestMsgLevel);
	
}

void LoggerTest::testModuleDisableAll () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Logger::setOutputDevice (buffer);
	
	Logger::setModuleLevel (NURIA_MODULE, Logger::AllLevels);
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QVERIFY(buffer->data ().isEmpty ());
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::DebugMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::LogMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::WarnMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::ErrorMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::CriticalMsg));
	
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::DebugMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::LogMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::WarnMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::ErrorMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::CriticalMsg));
	Logger::setModuleLevel (NURIA_MODULE, Logger::DefaultLowestMsgLevel);
}

void LoggerTest::testModuleDisablePartial () {
	using namespace Nuria;
	QBuffer *buffer = new QBuffer;
	buffer->open (QIODevice::WriteOnly);
	Logger::setOutputDevice (buffer);
	
	Logger::setModuleLevel (NURIA_MODULE, Logger::WarnMsg);
	Logger::setOutputFormat ("%TYPE%");
	nDebug() << "Debug";
	nLog() << "Log";
	nWarn() << "Warn";
	nError() << "Error";
	nCritical() << "Critical";
	
	QCOMPARE(buffer->data ().data (), "Warning\nError\nCritical\n");
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::DebugMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::LogMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::WarnMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::ErrorMsg));
	QVERIFY(!Logger::isModuleDisabled (nullptr, Logger::CriticalMsg));
	
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::DebugMsg));
	QVERIFY(Logger::isModuleDisabled (NURIA_MODULE, Logger::LogMsg));
	QVERIFY(!Logger::isModuleDisabled (NURIA_MODULE, Logger::WarnMsg));
	QVERIFY(!Logger::isModuleDisabled (NURIA_MODULE, Logger::ErrorMsg));
	QVERIFY(!Logger::isModuleDisabled (NURIA_MODULE, Logger::CriticalMsg));
	Logger::setModuleLevel (NURIA_MODULE, Logger::DefaultLowestMsgLevel);
}

void LoggerTest::testQtMessageHandler () {
	Nuria::Logger::Type type;
	QByteArray typeName;
	QByteArray transaction;
	QByteArray moduleName;
	QByteArray file;
	QByteArray className;
	QByteArray methodName;
	QString message;
	int line;
	
	auto func = [&](Nuria::Logger::Type type_, const QByteArray &transaction_, const QByteArray &typeName_,
	                const QByteArray &moduleName_, const QByteArray &file_, int line_,
	                const QByteArray &className_, const QByteArray &methodName_, const QString &message_) {
		type = type_;
		typeName = typeName_;
		transaction = transaction_;
		moduleName = moduleName_;
		file = file_;
		line = line_;
		className = className_;
		methodName = methodName_;
		message = message_;
	};
	
	Nuria::Logger::setOutputHandler (func);
	Nuria::Logger::setOutputDisabled (true);
	
	const char *expectedMessage = "NuriaFramework";
	
	Nuria::Logger::installMessageHandler ();
	int expectedLine = __LINE__; qCritical() << expectedMessage;
	
	QCOMPARE(type, Nuria::Logger::CriticalMsg);
	QCOMPARE(transaction, QByteArray ());
	QCOMPARE(typeName, QByteArray ("Critical"));
	QCOMPARE(moduleName, QByteArray ("default"));
	QCOMPARE(message, QString (expectedMessage));
	
#ifdef QT_DEBUG
	QCOMPARE(methodName, QByteArray ("testQtMessageHandler"));
	QCOMPARE(className, QByteArray ("LoggerTest"));
	QCOMPARE(file, QByteArray ("tst_logger.cpp"));
	QCOMPARE(line, expectedLine);
#else
	QCOMPARE(methodName, QByteArray ());
	QCOMPARE(className, QByteArray ());
	QCOMPARE(file, QByteArray ());
	QCOMPARE(line, 0);
#endif
	
	Nuria::Logger::setOutputHandler (Nuria::Logger::Handler ());
	Nuria::Logger::setOutputDisabled (false);
}

class TransactionThread : public QThread {
public:
	QByteArray transaction;
	
protected:
	void run () {
		Nuria::Logger::setTransaction ("Bar");
		this->transaction = Nuria::Logger::transaction ();
		Nuria::Logger::setTransaction ("Baz");
	}
	
};

void LoggerTest::transactionsAreThreadLocal () {
	TransactionThread thread;
	
	Nuria::Logger::setTransaction ("Foo");
	
	// Start thread ..
	thread.start ();
	thread.wait ();
	
	// Check state
	QCOMPARE(Nuria::Logger::transaction (), QByteArray ("Foo"));
	QCOMPARE(thread.transaction, QByteArray ("Bar"));
	
	Nuria::Logger::setTransaction (QByteArray ());
}

void LoggerTest::verifyLoggerTransactionBehaviour () {
	Nuria::Logger::setTransaction (QByteArray ("Outer"));
	
	{
		Nuria::LoggerTransaction t ("Foo");
		QCOMPARE(Nuria::Logger::transaction (), QByteArray ("Foo"));
	}
	
	QCOMPARE(Nuria::Logger::transaction (), QByteArray ("Outer"));
	Nuria::Logger::setTransaction (QByteArray ());
}

void LoggerTest::benchmark () {
	
	// Remove time from format
	Nuria::Logger::setOutputFormat ("%TRANSACTION% %TYPE%/%MODULE%: %FILE%:%LINE% - %CLASS%::%METHOD%: %BODY%");
	Nuria::Logger::setTransaction ("Benchmark");
	
	// Use a in-memory buffer to not benchmark stdout
	QBuffer *buffer = new QBuffer;
	QByteArray data;
	data.reserve (16 * 1024 * 1024); // 16MiB should suffice.
	buffer->setData (data);
	buffer->open (QIODevice::WriteOnly);
	Nuria::Logger::setOutputDevice (buffer);
	
	// Run.
	QBENCHMARK {
		nDebug() << "Foo";
	}
	
	// 
	Nuria::Logger::setOutputFormat (nullptr);
	Nuria::Logger::setOutputDevice (stdout);
}

QTEST_APPLESS_MAIN(LoggerTest)
#include "tst_logger.moc"
