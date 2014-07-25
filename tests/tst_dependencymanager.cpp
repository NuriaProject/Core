/* This file is part of the NuriaProject Framework. Copyright 2012 NuriaProject
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * The NuriaProject Framework is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with the library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <nuria/dependencymanager.hpp>

#include <QtTest/QtTest>
#include <QObject>

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
	
};

Q_DECLARE_METATYPE(DependencyManagerTest*)

// 
class TestClass : public QObject {
	Q_OBJECT
public:
	
	Q_INVOKABLE
	TestClass () : message ("Works") { }
	
	QString message;
};

Q_DECLARE_METATYPE(TestClass*)

// 
DependencyManagerTest::DependencyManagerTest () {
	
}

void DependencyManagerTest::storeAndRetrieveApplicationGlobal () {
	Nuria::DependencyManager *inst = Nuria::DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (Nuria::DependencyManager::ApplicationGlobal);
	inst->storeObject ("DependencyManagerTest", this);
	
	QCOMPARE(inst->get< DependencyManagerTest > ("DependencyManagerTest"), this);
	QCOMPARE(NURIA_DEPENDENCY(DependencyManagerTest), this);
	
}

void DependencyManagerTest::storeAndRetrieveSingleThread () {
	Nuria::DependencyManager *inst = Nuria::DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (Nuria::DependencyManager::SingleThread);
	inst->storeObject ("DependencyManagerTest", this);
	
	QCOMPARE(inst->get< DependencyManagerTest > ("DependencyManagerTest"), this);
	QCOMPARE(NURIA_DEPENDENCY(DependencyManagerTest), this);
	
}

void DependencyManagerTest::storeAndRetrieveThreadLocal () {
	Nuria::DependencyManager *inst = Nuria::DependencyManager::instance ();
	inst->setDefaultThreadingPolicy (Nuria::DependencyManager::ThreadLocal);
	inst->storeObject ("DependencyManagerTest", this);
	
	QCOMPARE(inst->get< DependencyManagerTest > ("DependencyManagerTest"), this);
	QCOMPARE(NURIA_DEPENDENCY(DependencyManagerTest), this);
	
}

void DependencyManagerTest::retrieveDefaultInstance () {
	TestClass *test = NURIA_DEPENDENCY(TestClass);
	
	QVERIFY(test);
	QCOMPARE(test->message, QString("Works"));
	QCOMPARE(test, NURIA_DEPENDENCY(TestClass));
}

QTEST_MAIN(DependencyManagerTest)
#include "tst_dependencymanager.moc"
