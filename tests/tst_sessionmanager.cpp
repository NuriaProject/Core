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
