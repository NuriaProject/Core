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

#include <nuria/unixsignalhandler.hpp>

#include <QtTest/QtTest>
#include <signal.h>
#include <QObject>

using namespace Nuria;

class UnixSignalHandlerTest : public QObject {
	Q_OBJECT
public:
	UnixSignalHandlerTest ();
	
public slots:
	void genericSignal (int id) { lastSignal = id; }
	void sigTerm () { signalMap[SIGTERM] = true; }
	void sigInt () { signalMap[SIGINT] = true; }
	void sigUsr1 () { signalMap[SIGUSR1] = true; }
	void sigUsr2 () { signalMap[SIGUSR2] = true; }
	
private slots:
	void initTestCase ();
	void init () { reset (); }
	
	void verifySignalEmitsSpecificSignal_data ();
	void verifySignalEmitsSpecificSignal ();
	void verifySIGALRMonlyEmitsSignalRaised ();
	void testUnregistration ();
	
private:
	bool wasOnlyTheGivenSignalInvoked (int sig) {
		if (!signalMap[sig]) {
			qDebug("** Signal %i not set", sig);
			return false;
		}
		
		signalMap[sig] = false;
		int i;
		for (i = 0; i < sizeof signalMap && !signalMap[i]; i++);
		if (i != sizeof signalMap) {
			qDebug("** Also invoked %i", i);
			return false;
		}
		
		return true;
	}
	
	void reset () {
		::memset (signalMap, 0x0, sizeof signalMap);
		lastSignal = 0;
	}
	
	// _NSIG = Max count of signals
	int lastSignal = 0;
	bool signalMap[_NSIG];
	
};

// 
UnixSignalHandlerTest::UnixSignalHandlerTest () {
	
}

void UnixSignalHandlerTest::initTestCase () {
	reset ();
	
	UnixSignalHandler *h = UnixSignalHandler::get ();
	connect (h, SIGNAL(sigTerm()), SLOT(sigTerm()));
	connect (h, SIGNAL(sigInterrupt()), SLOT(sigInt()));
	connect (h, SIGNAL(sigUser1()), SLOT(sigUsr1()));
	connect (h, SIGNAL(sigUser2()), SLOT(sigUsr2()));
	connect (h, SIGNAL(signalRaised(int)), SLOT(genericSignal(int)));
	
}

void UnixSignalHandlerTest::verifySignalEmitsSpecificSignal_data () {
	QTest::addColumn< int > ("signal");
	
	QTest::newRow("SIGTERM") << SIGTERM;
	QTest::newRow("SIGINT") << SIGINT;
	QTest::newRow("SIGUSR1") << SIGUSR1;
	QTest::newRow("SIGUSR2") << SIGUSR2;
	
}

void UnixSignalHandlerTest::verifySignalEmitsSpecificSignal () {
	QFETCH(int, signal);
	
	int cbSignal = 0;
	UnixSignalHandler::get ()->invokeOnSignal (signal, Callback::fromLambda ([&cbSignal](int id) {
		cbSignal = id;
	}));
	
	// 
	::raise (signal);
	qApp->processEvents ();
	
	QVERIFY(wasOnlyTheGivenSignalInvoked (signal));
	QCOMPARE(cbSignal, signal);
	QCOMPARE(lastSignal, signal);
}

void UnixSignalHandlerTest::verifySIGALRMonlyEmitsSignalRaised () {
	
	UnixSignalHandler::get ()->listenToUnixSignal (SIGALRM);
	
	::raise (SIGALRM);
	qApp->processEvents ();
	
	size_t i;
	for (i = 0; i < sizeof signalMap && !signalMap[i]; i++);
	
	QCOMPARE(lastSignal, SIGALRM);
	QCOMPARE(i, sizeof signalMap);
}

void UnixSignalHandlerTest::testUnregistration () {
	QVERIFY(UnixSignalHandler::get ()->listenToUnixSignal (SIGURG));
	QVERIFY(UnixSignalHandler::get ()->isListeningTo (SIGURG));
	QVERIFY(UnixSignalHandler::get ()->ignoreUnixSignal (SIGURG));
	QVERIFY(!UnixSignalHandler::get ()->isListeningTo (SIGURG));
	
	// There's no signal handler for SIGURG anymore, thus we won't know it happened.
	::raise (SIGURG);
	qApp->processEvents ();
	
	QCOMPARE(lastSignal, 0);
	
}

// 
QTEST_MAIN(UnixSignalHandlerTest)
#include "tst_unixsignalhandler.moc"
