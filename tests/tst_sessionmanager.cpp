#include <QString>
#include <QtTest>
#include <nuria/session.hpp>
#include <nuria/abstractsessionmanager.hpp>
#include <nuria/sessionmanager.hpp>

class SessionManagerTest : public QObject {
	Q_OBJECT
public:
	
private slots:
	void verifyStorageOfSessions ();
	void verifyRemoveSession ();
	void discardOldestSessionIfCacheLimitWasHit ();
};

void SessionManagerTest::verifyStorageOfSessions () {
	Nuria::SessionManager manager;
	Nuria::Session sessionFoo = manager.get (QByteArray ("Foo"));
	
	QVERIFY(sessionFoo.isValid ());
	QCOMPARE(sessionFoo.id (), QByteArray ("Foo"));
	QCOMPARE(sessionFoo.manager (), &manager);
	QCOMPARE(sessionFoo, manager.get (QByteArray ("Foo")));
	
	Nuria::Session sessionBar = manager.get (QByteArray ("Bar"));
	QVERIFY(sessionFoo != sessionBar);
}

void SessionManagerTest::verifyRemoveSession () {
	Nuria::SessionManager manager;
	Nuria::Session sessionBar = manager.get (QByteArray ("Bar"));
	sessionBar.remove ();
	
	QVERIFY(sessionBar != manager.get (QByteArray ("Bar")));
}

void SessionManagerTest::discardOldestSessionIfCacheLimitWasHit () {
	Nuria::SessionManager manager;
	manager.setMaxSessions (1);
	QCOMPARE(manager.maxSessions (), 1);

	Nuria::Session sessionFoo = manager.get (QByteArray ("Foo"));
	Nuria::Session sessionBar = manager.get (QByteArray ("Bar"));
	QCOMPARE(sessionBar, manager.get (QByteArray ("Bar")));
	QVERIFY(sessionFoo != manager.get (QByteArray ("Foo")));
}

QTEST_MAIN(SessionManagerTest)
#include "tst_sessionmanager.moc"
