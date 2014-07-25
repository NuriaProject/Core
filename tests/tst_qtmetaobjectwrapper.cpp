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

#include <nuria/qtmetaobjectwrapper.hpp>

#include <QtTest/QtTest>
#include <QObject>
#include <memory>

using namespace Nuria;

class QtMetaObjectWrapperTest : public QObject {
	Q_OBJECT
private slots:
	
	void initTestCase ();
	
	void verifyBasicTypeData ();
	void testClassInfoToAnnotations ();
	void testInstanceDeleter ();
	
	void verifyMethods ();
	void testMethodSafeInvoke ();
	void testMethodUnsafeInvoke ();
	void testMethodArgumentTest ();
	
	void verifyEnum ();
	
	void verifyFields ();
	void testFieldGetter ();
	void testFieldSetter ();
	void testReadOnlyField ();
	
private:
	QtMetaObjectWrapper *wrapper;
	
};

class TestObject : public QObject {
	Q_OBJECT
	Q_ENUMS(MyEnum)
	Q_PROPERTY(int a READ aGetter WRITE aSetter)
	Q_PROPERTY(int ro READ aGetter)
	Q_CLASSINFO("b", "2")
	Q_CLASSINFO("a", "1")
public:
	int m_a;
	
	enum MyEnum {
		First = 1,
		Second = 2
	};
	
	Q_INVOKABLE // Two methods due to optional argument
	TestObject (int a = 0) {
		m_a = a;
	}
	
	~TestObject () { qDebug("dtor"); }
	
	Q_INVOKABLE
	QString invokable (int a) { qDebug("invokable"); return ""; }
	
	int aGetter () { return m_a; }
	void aSetter (int a) { m_a = a; }
	
public slots:
	bool slotMethod  (QString b) { qDebug("slot"); return true; }
	
signals:
	void ignored ();
};

void QtMetaObjectWrapperTest::initTestCase () {
	wrapper = new QtMetaObjectWrapper (&TestObject::staticMetaObject);
}

void QtMetaObjectWrapperTest::verifyBasicTypeData () {
	QCOMPARE(wrapper->className (), QByteArray ("TestObject"));
	QCOMPARE(wrapper->parents (), { "QObject" });
}

void QtMetaObjectWrapperTest::testClassInfoToAnnotations () {
	QCOMPARE(wrapper->annotationCount (), 2);
	QCOMPARE(wrapper->annotation (0).name (), QByteArray ("a"));
	QCOMPARE(wrapper->annotation (0).value (), QVariant ("1"));
	QCOMPARE(wrapper->annotation (1).name (), QByteArray ("b"));
	QCOMPARE(wrapper->annotation (1).value (), QVariant ("2"));
}

void QtMetaObjectWrapperTest::testInstanceDeleter () {
	TestObject *obj = new TestObject;
	
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	wrapper->destroyInstance (obj);
	
}

void QtMetaObjectWrapperTest::verifyMethods () {
	QCOMPARE(wrapper->methodCount (), 2);
	
	QCOMPARE(wrapper->method (0).name (), QByteArray ("invokable"));
	QCOMPARE(wrapper->method (0).argumentNames ().length (), 1);
	QCOMPARE(wrapper->method (0).returnType (), QByteArray ("QString"));
	QCOMPARE(wrapper->method (0).type (), MetaMethod::Method);
	
	QCOMPARE(wrapper->method (1).name (), QByteArray ("slotMethod"));
	QCOMPARE(wrapper->method (1).argumentNames ().length (), 1);
	QCOMPARE(wrapper->method (1).returnType (), QByteArray ("bool"));
	QCOMPARE(wrapper->method (1).type (), MetaMethod::Method);
	
}

void QtMetaObjectWrapperTest::testMethodSafeInvoke () {
	TestObject obj;
	Callback cb = wrapper->method ({ "invokable", "int" }).callback (&obj);
	
	QTest::ignoreMessage (QtDebugMsg, "invokable");
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	QVERIFY(cb.isValid ());
	
	cb ();
}

void QtMetaObjectWrapperTest::testMethodUnsafeInvoke () {
	TestObject obj;
	Callback cb = wrapper->method ({ "invokable", "int" }).unsafeCallback (&obj);
	
	QTest::ignoreMessage (QtDebugMsg, "invokable");
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	QVERIFY(cb.isValid ());
	
	cb ();
}

void QtMetaObjectWrapperTest::testMethodArgumentTest () {
	TestObject obj;
	Callback cb = wrapper->method ({ "invokable", "int" }).testCallback (&obj);
	
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	QVERIFY(cb.isValid ());
	QCOMPARE(cb ().toBool (), true);
}

void QtMetaObjectWrapperTest::verifyEnum () {
	QCOMPARE(wrapper->enumCount (), 1);
	
	MetaEnum e = wrapper->enumAt (0);
	QCOMPARE(e.elementCount (), 2);
	QCOMPARE(e.key (0), QByteArray ("First"));
	QCOMPARE(e.value (0), 1);
	QCOMPARE(e.key (1), QByteArray ("Second"));
	QCOMPARE(e.value (1), 2);
	
}

void QtMetaObjectWrapperTest::verifyFields () {
	QCOMPARE(wrapper->fieldCount (), 2);
	
	MetaField a = wrapper->field (0);
	MetaField b = wrapper->field (1);
	
	QCOMPARE(a.name (), QByteArray ("a"));
	QCOMPARE(a.typeName (), QByteArray ("int"));
	QCOMPARE(a.access (), MetaField::ReadWrite);
	
	QCOMPARE(b.name (), QByteArray ("ro"));
	QCOMPARE(b.typeName (), QByteArray ("int"));
	QCOMPARE(b.access (), MetaField::ReadOnly);
}

void QtMetaObjectWrapperTest::testFieldGetter () {
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	TestObject obj (123);
	
	QCOMPARE(wrapper->field (0).read (&obj), QVariant (123));
}

void QtMetaObjectWrapperTest::testFieldSetter () {
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	TestObject obj;
	
	QVERIFY(wrapper->field (0).write (&obj, 123));
	QCOMPARE(obj.m_a, 123);
}

void QtMetaObjectWrapperTest::testReadOnlyField () {
	QTest::ignoreMessage (QtDebugMsg, "dtor");
	TestObject obj;
	
	QVERIFY(!wrapper->field (1).write (&obj, 123));
	QCOMPARE(obj.m_a, 0);
	
}

QTEST_MAIN(QtMetaObjectWrapperTest)
#include "tst_qtmetaobjectwrapper.moc"
