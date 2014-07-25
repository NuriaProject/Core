#include <QString>
#include <QtTest>

#include <nuria/session.hpp>

class SessionTest : public QObject {
	Q_OBJECT
private slots:
	void verifyDefaultConstructedSession ();
	void newKeyCreatesValue ();
	void gettingNonExistantValueDoesNotAlterSession ();
	void writeAccessSetsTheDirtyFlag ();
	void verifyReferenceCounting ();
	void verifyCleanDirtyMethods ();
};

void SessionTest::verifyDefaultConstructedSession () {
	Nuria::Session session;
	
	QVERIFY(!session.isValid ());
	QVERIFY(!session.isDirty ());
	QCOMPARE(session.id (), QByteArray ());
	QVERIFY(!session.manager ());
}

void SessionTest::newKeyCreatesValue () {
	Nuria::Session session;
	
	QVERIFY(!session.isDirty ());
	QVERIFY(!session.contains ("a"));
	session["a"] = 1;
	
	QVERIFY(session.isDirty ());
	QCOMPARE(session["a"].toInt (), 1);
}

void SessionTest::gettingNonExistantValueDoesNotAlterSession () {
	Nuria::Session session;
	
	session["foo"] = 123;
	session.markClean ();
	
	// 
	QVERIFY(!session.isDirty ());
	QVERIFY(session.contains ("foo"));
	QVERIFY(session.value ("foo").isValid ());
	QVERIFY(!session.isDirty ());
	
}

void SessionTest::writeAccessSetsTheDirtyFlag () {
	Nuria::Session session;
	
	session["foo"] = 123;
	session.markClean ();
	
	QVERIFY(!session.isDirty ());
	session["foo"] = 321;
	QVERIFY(session.isDirty ());
	
}

void SessionTest::verifyReferenceCounting () {
	Nuria::Session session;
	
	QCOMPARE(session.refCount (), 1);
	
	Nuria::Session second = session;
	QCOMPARE(session.refCount (), 2);
	QCOMPARE(second.refCount (), 2);
	
	// 
	{
		Nuria::Session third = session;
		QCOMPARE(session.refCount (), 3);
		QCOMPARE(second.refCount (), 3);
		QCOMPARE(third.refCount (), 3);
	}
	
	// 
	QCOMPARE(session.refCount (), 2);
	QCOMPARE(second.refCount (), 2);
}

void SessionTest::verifyCleanDirtyMethods () {
	Nuria::Session session;
	QVERIFY(!session.isDirty ());
	
	session.markDirty ();
	QVERIFY(session.isDirty ());
	
	session.markClean ();
	QVERIFY(!session.isDirty ());
	
}

QTEST_MAIN(SessionTest)
#include "tst_session.moc"
