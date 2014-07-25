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

#include <nuria/runtimemetaobject.hpp>

#include <QtTest/QtTest>
#include <QObject>
#include <memory>

using namespace Nuria;

class RuntimeMetaObjectTest : public QObject {
	Q_OBJECT
public:
	
private slots:
	
	void testMetaTypeIds ();
	void testClassAnnotations ();
	void testBaseClassesAreOrdered ();
	void testInstanceDeleter ();
	
	void verifyMethodOrdering ();
	void verifyMethodAnnotations ();
	void testMethodCreator ();
	void addMethodReplacesOldWithNewMethodIfNotUnique ();
	
	void verifyEnumOrdering ();
	void verifyEnumAnnotations ();
	void verifyEnumValueOrdering ();
	
	void verifyFieldOrdering ();
	void verifyFieldAnnotations ();
	void testFieldGetter ();
	void testFieldSetter ();
	void testReadOnlyField ();
	
};

void RuntimeMetaObjectTest::testMetaTypeIds () {
	RuntimeMetaObject meta ("A");
	
	meta.setQtMetaTypeId (123);
	meta.setQtMetaTypePointerId (124);
	
	// 
	meta.finalize ();
	QCOMPARE(meta.className (), QByteArray ("A"));
	QCOMPARE(meta.metaTypeId (), 123);
	QCOMPARE(meta.pointerMetaTypeId (), 124);
	
}

void RuntimeMetaObjectTest::testClassAnnotations () {
	RuntimeMetaObject meta ("A");
	
	// Make sure that annotations are ordered
	meta.setAnnotations ({ { "Second", 2 }, { "First", 1 } });
	
	// 
	meta.finalize ();
	QCOMPARE(meta.annotationCount (), 2);
	QCOMPARE(meta.annotation (0).name (), QByteArray ("First"));
	QCOMPARE(meta.annotation (0).value (), QVariant (1));
	QCOMPARE(meta.annotation (1).name (), QByteArray ("Second"));
	QCOMPARE(meta.annotation (1).value (), QVariant (2));
	
}

void RuntimeMetaObjectTest::testBaseClassesAreOrdered () {
	RuntimeMetaObject meta ("A");
	
	meta.setBaseClasses ({ "B", "A" });
	
	// 
	meta.finalize ();
	QCOMPARE(meta.parents ().length (), 2);
	QCOMPARE(meta.parents ().at (0), QByteArray ("A"));
	QCOMPARE(meta.parents ().at (1), QByteArray ("B"));
	
}

void RuntimeMetaObjectTest::testInstanceDeleter () {
	RuntimeMetaObject meta ("A");
	
	void *expectedPtr = this;
	void *receivedPtr = nullptr;
	
	meta.setInstanceDeleter ([&receivedPtr](void *ptr) { receivedPtr = ptr; });
	
	// 
	meta.finalize ();
	meta.destroyInstance (expectedPtr);
	
	QCOMPARE(receivedPtr, expectedPtr);
	
}

static Callback noopCreator (void *, RuntimeMetaObject::InvokeAction) {
	return Callback ();
}

void RuntimeMetaObjectTest::verifyMethodOrdering () {
	RuntimeMetaObject meta ("A");
	
	meta.addMethod (MetaMethod::Method, "a", "A", { "foo" }, { "bar" }, { }, noopCreator);
	meta.addMethod (MetaMethod::Method, "a", "A", { }, { }, { }, noopCreator);
	meta.addMethod (MetaMethod::Method, "a", "A", { "a", "b" }, { "bar", "baz" }, { }, noopCreator);
	
	// 
	meta.finalize ();
	
	QCOMPARE(meta.methodCount (), 3);
	QCOMPARE(meta.method (0).name (), QByteArray ("a"));
	QCOMPARE(meta.method (0).returnType (), QByteArray ("A"));
	QCOMPARE(meta.method (0).argumentNames ().length (), 0);
	QCOMPARE(meta.method (1).argumentNames ().length (), 1);
	QCOMPARE(meta.method (2).argumentNames ().length (), 2);
}

void RuntimeMetaObjectTest::verifyMethodAnnotations () {
	RuntimeMetaObject meta ("A");
	
	meta.addMethod (MetaMethod::Method, "a", "A", { }, { }, { { "Second", 2 }, { "First", 1 } }, noopCreator);
	
	// 
	meta.finalize ();
	
	MetaMethod m = meta.method (0);
	QCOMPARE(meta.methodCount (), 1);
	QCOMPARE(m.annotationCount (), 2);
	QCOMPARE(m.annotation (0).name (), QByteArray ("First"));
	QCOMPARE(m.annotation (0).value (), QVariant (1));
	QCOMPARE(m.annotation (1).name (), QByteArray ("Second"));
	QCOMPARE(m.annotation (1).value (), QVariant (2));
	
}

void RuntimeMetaObjectTest::testMethodCreator () {
	RuntimeMetaObject meta ("A");
	
	RuntimeMetaObject::InvokeAction last;
	Callback expectedCb (this, SLOT(deleteLater()));
	void *expectedPtr = this;
	void *lastPtr = nullptr;
	
	auto creator = [&expectedCb, &last, &lastPtr](void *ptr, RuntimeMetaObject::InvokeAction act) {
		lastPtr = ptr;
		last = act;
		return expectedCb;
	};
	
	meta.addMethod (MetaMethod::Static, "a", "A", { }, { }, { }, creator);
	
	// 
	meta.finalize ();
	
	MetaMethod m = meta.method (0);
	QCOMPARE(meta.methodCount (), 1);
	
	// Invoke
	QCOMPARE(m.callback (expectedPtr), expectedCb);
	QCOMPARE(last, RuntimeMetaObject::InvokeAction::Invoke);
	QCOMPARE(lastPtr, expectedPtr);
	lastPtr = nullptr;
	
	// UnsafeInvoke
	QCOMPARE(m.unsafeCallback (expectedPtr), expectedCb);
	QCOMPARE(last, RuntimeMetaObject::InvokeAction::UnsafeInvoke);
	QCOMPARE(lastPtr, expectedPtr);
	lastPtr = nullptr;
	
	// ArgumentTest
	QCOMPARE(m.testCallback (expectedPtr), expectedCb);
	QCOMPARE(last, RuntimeMetaObject::InvokeAction::ArgumentTest);
	QCOMPARE(lastPtr, expectedPtr);
	
}

void RuntimeMetaObjectTest::addMethodReplacesOldWithNewMethodIfNotUnique () {
	RuntimeMetaObject meta ("A");
	
	auto dummy = [](void *, RuntimeMetaObject::InvokeAction)
	{ return Callback (); };
	
	QVector< QByteArray > types { "QString", "int" };
	QVector< QByteArray > names1 { "a", "b" };
	QVector< QByteArray > names2 { "house", "mouse" };
	
	// 
	meta.addMethod (MetaMethod::Method, "foo", "int", names1, types, { }, dummy);
	meta.addMethod (MetaMethod::Static, "foo", "QByteArray", names2, types, { }, dummy);
	
	// 
	meta.finalize ();
	
	// 
	MetaMethod m = meta.method (0);
	QCOMPARE(meta.methodCount (), 1);
	QCOMPARE(m.type (), MetaMethod::Static);
	QCOMPARE(m.name (), QByteArray ("foo"));
	QCOMPARE(m.returnType (), QByteArray ("QByteArray"));
	QCOMPARE(m.argumentTypes (), types);
	QCOMPARE(m.argumentNames (), names2);
	
}

void RuntimeMetaObjectTest::verifyEnumOrdering () {
	RuntimeMetaObject meta ("A");
	
	meta.addEnum ("B", { }, { });
	meta.addEnum ("A", { }, { });
	
	// 
	meta.finalize ();
	QCOMPARE(meta.enumCount (), 2);
	QCOMPARE(meta.enumAt (0).name (), QByteArray ("A"));
	QCOMPARE(meta.enumAt (1).name (), QByteArray ("B"));
}

void RuntimeMetaObjectTest::verifyEnumAnnotations () {
	RuntimeMetaObject meta ("A");
	
	meta.addEnum ("Enum", { { "Second", 2 }, { "First", 1 } }, { });
	
	// 
	meta.finalize ();
	
	MetaEnum e = meta.enumAt (0);
	QCOMPARE(meta.enumCount (), 1);
	QCOMPARE(e.annotationCount (), 2);
	QCOMPARE(e.annotation (0).name (), QByteArray ("First"));
	QCOMPARE(e.annotation (0).value (), QVariant (1));
	QCOMPARE(e.annotation (1).name (), QByteArray ("Second"));
	QCOMPARE(e.annotation (1).value (), QVariant (2));
}

void RuntimeMetaObjectTest::verifyEnumValueOrdering () {
	RuntimeMetaObject meta ("A");
	
	meta.addEnum ("Enum", { }, { { "Second", 2 }, { "First", 1 } });
	
	// 
	meta.finalize ();
	
	MetaEnum e = meta.enumAt (0);
	QCOMPARE(meta.enumCount (), 1);
	QCOMPARE(e.elementCount (), 2);
	QCOMPARE(e.key (0), QByteArray ("First"));
	QCOMPARE(e.value (0), 1);
	QCOMPARE(e.key (1), QByteArray ("Second"));
	QCOMPARE(e.value (1), 2);
}

static QVariant noopGetter (void *) { return QVariant (); }
static bool noopSetter (void *, const QVariant &) { return false; }

void RuntimeMetaObjectTest::verifyFieldOrdering () {
	RuntimeMetaObject meta ("A");
	
	meta.addField ("b", "int", { }, noopGetter);
	meta.addField ("a", "int", { }, noopGetter, noopSetter);
	
	// 
	meta.finalize ();
	
	QCOMPARE(meta.fieldCount (), 2);
	QCOMPARE(meta.field (0).name (), QByteArray ("a"));
	QCOMPARE(meta.field (0).access (), MetaField::ReadWrite);
	QCOMPARE(meta.field (1).name (), QByteArray ("b"));
	QCOMPARE(meta.field (1).access (), MetaField::ReadOnly);
}

void RuntimeMetaObjectTest::verifyFieldAnnotations () {
	RuntimeMetaObject meta ("A");
	
	meta.addField ("a", "A", { { "Second", 2 }, { "First", 1 } }, noopGetter);
	
	// 
	meta.finalize ();
	
	MetaField f = meta.field (0);
	QCOMPARE(meta.fieldCount (), 1);
	QCOMPARE(f.annotationCount (), 2);
	QCOMPARE(f.annotation (0).name (), QByteArray ("First"));
	QCOMPARE(f.annotation (0).value (), QVariant (1));
	QCOMPARE(f.annotation (1).name (), QByteArray ("Second"));
	QCOMPARE(f.annotation (1).value (), QVariant (2));
}

void RuntimeMetaObjectTest::testFieldGetter () {
	RuntimeMetaObject meta ("A");
	
	void *expectedPtr = this;
	void *ptr = nullptr;
	
	auto getter = [&ptr](void *inst) { ptr = inst; return QVariant (123); };
	
	meta.addField ("a", "int", { }, getter, noopSetter);
	
	// 
	meta.finalize ();
	MetaField f = meta.field (0);
	QCOMPARE(f.access (), MetaField::ReadWrite);
	QCOMPARE(f.read (expectedPtr), QVariant (123));
	QCOMPARE(ptr, expectedPtr);
}

void RuntimeMetaObjectTest::testFieldSetter () {
	RuntimeMetaObject meta ("A");
	
	void *expectedPtr = this;
	void *ptr = nullptr;
	QVariant value;
	
	auto setter = [&ptr, &value](void *inst, const QVariant &v) {
		ptr = inst;
		value = v;
		return true;
	};
	
	meta.addField ("a", "int", { }, noopGetter, setter);
	
	// 
	meta.finalize ();
	MetaField f = meta.field (0);
	QCOMPARE(f.access (), MetaField::ReadWrite);
	QVERIFY(f.write (expectedPtr, 123));
	QCOMPARE(value, QVariant (123));
	QCOMPARE(ptr, expectedPtr);
}

void RuntimeMetaObjectTest::testReadOnlyField () {
	RuntimeMetaObject meta ("A");
	
	meta.addField ("a", "int", { }, noopGetter);
	
	// 
	meta.finalize ();
	MetaField f = meta.field (0);
	QCOMPARE(f.access (), MetaField::ReadOnly);
	QVERIFY(!f.write (this, 123));
}

QTEST_MAIN(RuntimeMetaObjectTest)
#include "tst_runtimemetaobject.moc"
