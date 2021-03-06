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

#include <nuria/dependencymanager.hpp>

#include <QtTest/QtTest>
#include <QObject>
#include <QThread>

using namespace Nuria;

class DependencyManagerTest : public QObject {
	Q_OBJECT
public:
	DependencyManagerTest ();
	
private slots:
	
	// 
	void storeAndRetrieveApplicationGlobal ();
	void storeAndRetrieveSingleThread ();
	void storeAndRetrieveThreadLocal ();
	void retrieveDefaultInstance ();
	
	// 
	void verifyMultithreading ();
	void useCreator ();
	void dependencyTemplateSimple ();
	void dependencyTemplateNamed ();
	void verifyOperators ();
	
};

Q_DECLARE_METATYPE(DependencyManagerTest*)

// 
class TestClass : public QObject {
	Q_OBJECT
public:
	
	Q_INVOKABLE
	TestClass (const QString &msg = "Works") : message (msg) { }
	
	QString message;
};

Q_DECLARE_METATYPE(TestClass*)

// 
DependencyManagerTest::DependencyManagerTest () {
	
}

void DependencyManagerTest::storeAndRetrieveApplicationGlobal () {
	DependencyManager *inst = DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (DependencyManager::ApplicationGlobal);
	inst->storeObject ("Global", this);
	
	QCOMPARE(inst->get< DependencyManagerTest > ("Global"), this);
}

void DependencyManagerTest::storeAndRetrieveSingleThread () {
	DependencyManager *inst = DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (DependencyManager::SingleThread);
	inst->storeObject ("Single", this);
	
	QCOMPARE(inst->get< DependencyManagerTest > ("Single"), this);
}

void DependencyManagerTest::storeAndRetrieveThreadLocal () {
	DependencyManager *inst = DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (DependencyManager::ThreadLocal);
	inst->storeObject ("Thread", this);
	
	QCOMPARE(inst->get< DependencyManagerTest > ("Thread"), this);
}

void DependencyManagerTest::retrieveDefaultInstance () {
	TestClass *test = DependencyManager::get< TestClass > ("TestClass");
	
	QVERIFY(test);
	QCOMPARE(test->message, QString("Works"));
	QCOMPARE(test, DependencyManager::get< TestClass > ("TestClass"));
}

class Thread : public QThread {
	Q_OBJECT
public:
	
	QString message;
	Dependency< TestClass > read;
	
	Thread (QString m) : QThread (qApp), message (m) {}
	
	void run () override {
		this->read->message = this->message;
	}
	
};

void DependencyManagerTest::verifyMultithreading () {
	DependencyManager *inst = DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (DependencyManager::ThreadLocal);
	
	Thread *a = new Thread ("a");
	Thread *b = new Thread ("b");
	
	// Run and wait ..
	a->start ();
	b->start ();
	
	a->wait ();
	b->wait ();
	
	// 
	QVERIFY(a->read);
	QVERIFY(b->read);
	QVERIFY(a->read.get () != b->read.get ());
	QCOMPARE(a->read->message, QString("a"));
	QCOMPARE(b->read->message, QString("b"));
	
}

void DependencyManagerTest::useCreator () {
	DependencyManager *inst = DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (DependencyManager::SingleThread);
	
	// 
	QTest::ignoreMessage (QtDebugMsg, "creator");
	inst->setCreator ("test", []() { qDebug("creator"); return new TestClass ("Test"); });
	QVERIFY(inst->get< TestClass > ("test"));
	QCOMPARE(inst->get< TestClass > ("test")->message, QString ("Test"));
	
}

void DependencyManagerTest::dependencyTemplateSimple () {
	Dependency< TestClass > obj;
	
	QVERIFY(obj.get ());
	QCOMPARE(obj->message, QString ("Works"));
}

void DependencyManagerTest::dependencyTemplateNamed () {
	DependencyManager *inst = DependencyManager::instance ();
	inst->storeObject ("named", new TestClass ("Named"));
	
	Dependency< TestClass > obj ("named");
	
	QVERIFY(obj.get ());
	QCOMPARE(obj->message, QString ("Named"));
	
}

void DependencyManagerTest::verifyOperators () {
	Dependency< TestClass > a;
	Dependency< TestClass > b;
	Dependency< TestClass > c ("named");
	
	QVERIFY(a == b);
	QVERIFY(a != c);
	QVERIFY(a);
	
}

QTEST_MAIN(DependencyManagerTest)
#include "tst_dependencymanager.moc"
