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
	
	void valueDoesNotSetDirtyFlag ();
	void insertStoresAndSetsDirty ();
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

void SessionTest::valueDoesNotSetDirtyFlag () {
	Nuria::Session session;
	QVERIFY(!session.isDirty ());
	
	session.insert ("foo", 123);
	session.markClean ();
	
	QCOMPARE(session.value ("foo"), QVariant (123));
	QVERIFY(!session.isDirty ());
}

void SessionTest::insertStoresAndSetsDirty () {
	Nuria::Session session;
	QVERIFY(!session.isDirty ());
	
	session.insert ("foo", 123);
	
	QVERIFY(session.isDirty ());
	QCOMPARE(session.value ("foo"), QVariant (123));
	QVERIFY(session.isDirty ()); // Not altered by value().
	
}

QTEST_MAIN(SessionTest)
#include "tst_session.moc"
