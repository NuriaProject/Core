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

#include <QtTest>

#include <nuria/objectwrapperresource.hpp>

using namespace Nuria;

class TestWrapperObject : public QObject {
	Q_OBJECT
public:
	
	// Invokables aren't exported
	Q_INVOKABLE
	void notExportedInvokable () { }
	
private slots:
	
	// Private slot is not exported
	void notExportedPrivate () { }
	
protected slots:
	
	// Protected slot is not exported
	void notExportedProtected () { }
	
public slots:
	
	void voidSlot () { qDebug("voidSlot"); }
	void voidSlotArgument (const QString &arg) { qDebug("%s", qPrintable(arg)); }
	void overloaded (int a) { qDebug("int %i", a); }
	void overloaded (QString b) { qDebug("string %s", qPrintable(b)); }
	void overloaded (int a, QString b) { qDebug("int-string %i %s", a, qPrintable(b)); }
	
	int intSlot () { return 123; }
	int intSlotArgument (int a) { return a * 2; }
	QVariant variantSlot () { return QString ("foo"); }
	QVariant variantSlotArgument (QVariant a) { return a.toInt () + 1; }
	QVariantList manyArguments (int a, QString b, bool c, QVariant d) { return { a, b, c, d }; }
	
signals:
	
	void noArguments ();
	void intArgument (int count);
	void variantArgument (QVariant variant);
	void manyArguments (int a, QString b);
	
};

class ObjectWrapperResourceTest : public QObject {
	Q_OBJECT
public:
	
	ObjectWrapperResourceTest ();
	
private slots:
	
	void verifyInterfaceName ();
	void verifyPropertyList ();
	
	void exclude_data ();
	void exclude ();
	
	void excludeMultiple ();
	
	void testVoidSlots_data ();
	void testVoidSlots ();
	
	void testResultSlots_data ();
	void testResultSlots ();
	
	void testSignals_data ();
	void testSignals ();
	
	void signalCanBeCancelled ();
	void multipleConnectionsAreHandled ();
	
private:
	
	TestWrapperObject object;
	ObjectWrapperResource *wrapper;
	
};

ObjectWrapperResourceTest::ObjectWrapperResourceTest () {
	wrapper = new ObjectWrapperResource ("Foo", &object, object.metaObject (), this);
}

void ObjectWrapperResourceTest::verifyInterfaceName () {
	QCOMPARE(this->wrapper->interfaceName (), QString ("Foo"));
}

static bool operator== (const Resource::Property &a, const Resource::Property &b) {
	return (a.type () == b.type () &&
	        a.name () == b.name () &&
	        a.arguments () == b.arguments ());
}

static QDebug operator<< (QDebug &dbg, const Resource::Property &p) {
	dbg << "Property(" << p.type () << p.name () << p.arguments () << ")";
	return dbg;
}

void ObjectWrapperResourceTest::verifyPropertyList () {
	Resource::PropertyList list;
	int state = -1;
	
	auto cb = [&](Resource::InvokeResultState s, QVariant v) {
		state = s;
		list = v.value< Resource::PropertyList > ();
	};
	
	// Invoke
	this->wrapper->properties (cb);
	
	// List of expected properties
	Resource::PropertyList expected = {
	        { Resource::Property::Signal, "noArguments", { } },
                { Resource::Property::Signal, "intArgument", { { "count", QMetaType::Int } } },
                { Resource::Property::Signal, "variantArgument", { { "variant", QMetaType::QVariant } } },
                { Resource::Property::Signal, "manyArguments",
	          { { "a", QMetaType::Int }, { "b", QMetaType::QString } } },
	        
	        { Resource::Property::Slot, "voidSlot", { } },
	        { Resource::Property::Slot, "voidSlotArgument", { { "arg", QMetaType::QString } } },
	        { Resource::Property::Slot, "overloaded", { { "a", QMetaType::Int } } },
	        { Resource::Property::Slot, "overloaded", { { "b", QMetaType::QString } } },
	        { Resource::Property::Slot, "overloaded",
	          { { "a", QMetaType::Int }, { "b", QMetaType::QString } } },
	        
	        { Resource::Property::Slot, "intSlot", { }, QMetaType::Int },
	        { Resource::Property::Slot, "intSlotArgument", { { "a", QMetaType::Int } }, QMetaType::Int },
	        { Resource::Property::Slot, "variantSlot", { }, QMetaType::QVariant },
	        { Resource::Property::Slot, "variantSlotArgument",
	          { { "a", QMetaType::QVariant } }, QMetaType::QVariant },
	        { Resource::Property::Slot, "manyArguments", {
	                  { "a", QMetaType::Int }, { "b", QMetaType::QString },
	                  { "c", QMetaType::Bool }, { "d", QMetaType::QVariant }
	          }, QMetaType::QVariantList },
	};
	
	// Compare
	QCOMPARE(state, int (Resource::Success));
	QCOMPARE(list, expected);
}

void ObjectWrapperResourceTest::exclude_data () {
	QTest::addColumn< QString > ("fullName");
	QTest::addColumn< QString > ("name");
	
	QTest::newRow ("signal") << SIGNAL(intArgument(int)) << "intArgument";
	QTest::newRow ("slot") << SLOT(intSlotArgument(int)) << "intSlotArgument";
	
}

static bool propertyListContains (const Resource::PropertyList &list, const QString &name) {
	for (int i = 0; i < list.length (); i++) {
		if (list.at (i).name () == name) return true;
	}
	
	return false;
}

void ObjectWrapperResourceTest::exclude () {
	QFETCH(QString, fullName);
	QFETCH(QString, name);
	QByteArray fullNameData = fullName.toLatin1 ();
	
	// 
	ObjectWrapperResource wrapper ("Foo", &this->object, this->object.metaObject ());
	Resource::PropertyList propertyList;
	
	Resource::InvokeCallback cb = [&propertyList](Resource::InvokeResultState, const QVariant &v)
	{ propertyList = v.value< Resource::PropertyList > (); };
	
	// Check that we have a property with the name
	wrapper.properties (cb);
	QVERIFY(propertyListContains (propertyList, name));
	
	// Exclude it
	wrapper.exclude (fullNameData.constData ());
	
	// Check that it's now gone
	wrapper.properties (cb);
	QVERIFY(!propertyListContains (propertyList, name));
}

void ObjectWrapperResourceTest::excludeMultiple () {
	ObjectWrapperResource wrapper ("Foo", &this->object, this->object.metaObject ());
	Resource::PropertyList propertyList;
	
	Resource::InvokeCallback cb = [&propertyList](Resource::InvokeResultState, const QVariant &v)
	{ propertyList = v.value< Resource::PropertyList > (); };
	
	// 
	wrapper.properties (cb);
	QVERIFY(propertyListContains (propertyList, "noArguments"));
	QVERIFY(propertyListContains (propertyList, "intArgument"));
	
	// 'intArgument' comes after 'noArguments'
	wrapper.exclude (SIGNAL(noArguments()));
	wrapper.exclude (SIGNAL(intArgument(int)));
	
	// 
	wrapper.properties (cb);
	QVERIFY(!propertyListContains (propertyList, "noArguments"));
	QVERIFY(!propertyListContains (propertyList, "intArgument"));
	
}

void ObjectWrapperResourceTest::testVoidSlots_data () {
	QTest::addColumn< QString > ("name");
	QTest::addColumn< QVariantMap > ("arguments");
	QTest::addColumn< QString > ("output");
	
	QTest::newRow ("voidSlot") << "voidSlot" << QVariantMap { } << "voidSlot";
	QTest::newRow ("voidSlotArgument") << "voidSlotArgument" << QVariantMap { { "arg", "foo" } }
	                                   << "foo";
	QTest::newRow ("overloaded(int)") << "overloaded" << QVariantMap { {"a", 123 } } << "int 123";
	QTest::newRow ("overloaded(string)") << "overloaded" << QVariantMap { {"b", "foo" } } << "string foo";
	QTest::newRow ("overloaded(int,string)") << "overloaded" << QVariantMap { { "a", 456 }, { "b", "bar" } }
	                                         << "int-string 456 bar";
	
}

void ObjectWrapperResourceTest::testVoidSlots () {
	QFETCH(QString, name);
	QFETCH(QVariantMap, arguments);
	QFETCH(QString, output);
	
	int state = -1;
	
	auto cb = [&](Resource::InvokeResultState s, QVariant) {
		state = s;
	};
	
	// 
	QByteArray msg = output.toLatin1 ();
	QTest::ignoreMessage (QtDebugMsg, msg.constData ());
	this->wrapper->invoke (name, arguments, cb);
	QCOMPARE(state, int (Resource::Success));
}

void ObjectWrapperResourceTest::testResultSlots_data () {
	QTest::addColumn< QString > ("name");
	QTest::addColumn< QVariantMap > ("arguments");
	QTest::addColumn< QVariant > ("result");
	
	QTest::newRow ("intSlot") << "intSlot" << QVariantMap { } << QVariant (123);
	QTest::newRow ("intSlotArgument") << "intSlotArgument" << QVariantMap { { "a", 124 } } << QVariant (248);
	QTest::newRow ("variantSlot") << "variantSlot" << QVariantMap { } << QVariant ("foo");
	QTest::newRow ("variantSlotArgument") << "variantSlotArgument" << QVariantMap { { "a", 124 } }
	                                      << QVariant (125);
	QTest::newRow ("manyArguments") << "manyArguments"
	                                << QVariantMap { { "a", 42 }, { "b", "nuria" }, { "c", true }, { "d", 21 } }
	                                << QVariant (QVariantList { 42, "nuria", true, 21 });
	
}

void ObjectWrapperResourceTest::testResultSlots () {
	QFETCH(QString, name);
	QFETCH(QVariantMap, arguments);
	QFETCH(QVariant, result);
	
	QVariant returned;
	int state = -1;
	
	auto cb = [&](Resource::InvokeResultState s, QVariant v) {
		state = s;
		returned = v;
	};
	
	// 
	this->wrapper->invoke (name, arguments, cb);
	QCOMPARE(state, int (Resource::Success));
	QCOMPARE(returned, result);
}

typedef std::function< void() > TestSignalEmitter;
Q_DECLARE_METATYPE(TestSignalEmitter);

void ObjectWrapperResourceTest::testSignals_data () {
	QTest::addColumn< QString > ("name");
	QTest::addColumn< TestSignalEmitter > ("emitter");
	QTest::addColumn< QVariantMap > ("expected");
	
	TestSignalEmitter noArguments = [this] { this->object.noArguments (); };
	TestSignalEmitter intArgument = [this] { this->object.intArgument (42); };
	TestSignalEmitter variantArgument = [this] { this->object.variantArgument ("foo"); };
	TestSignalEmitter manyArguments = [this] { this->object.manyArguments (42, "foo"); };
	
	QTest::newRow ("noArguments") << "noArguments" << noArguments << QVariantMap { };
	QTest::newRow ("intArgument") << "intArgument" << intArgument << QVariantMap { { "count", 42 } };
	QTest::newRow ("variantArgument") << "variantArgument" << variantArgument
	                                  << QVariantMap { { "variant", "foo" } };
	QTest::newRow ("manyArguments") << "manyArguments" << manyArguments
	                                << QVariantMap { { "a", 42 }, { "b", "foo" } };
	
}

void ObjectWrapperResourceTest::testSignals () {
	QFETCH(QString, name);
	QFETCH(QVariantMap, expected);
	QFETCH(TestSignalEmitter, emitter);
	
	// Boilerplate
	QVariantMap map;
	int state = -1;
	
	auto cb = [&](Resource::InvokeResultState s, QVariant v) {
		state = s;
		map = v.toMap ();
	};
	
	// Connect
	Invocation conn = this->wrapper->listen (name, cb);
	
	emitter (); // Emit
	
	QCOMPARE(state, int (Resource::Success));
	QCOMPARE(map, expected);
	
	conn.cancel ();
	
}

void ObjectWrapperResourceTest::signalCanBeCancelled () {
	QVariantMap map;
	int state = -1;
	int calls = 0;
	
	auto cb = [&](Resource::InvokeResultState s, QVariant v) {
		state = s;
		map = v.toMap ();
		calls++;
	};
	
	// Connect
	Invocation conn = this->wrapper->listen ("intArgument", cb);
	emit this->object.intArgument (123); // Will go through
	conn.cancel (); // Cancel connection
	emit this->object.intArgument (456); // Won't be distributed
	
	// 
	QCOMPARE(calls, 1);
	QCOMPARE(state, int (Resource::Success));
	QCOMPARE(map, QVariantMap ({ { "count", 123 } }));
	
}

void ObjectWrapperResourceTest::multipleConnectionsAreHandled () {
	QVector< QVariantMap > mapInt1, mapInt2, mapVariant1, mapVariant2;
	QVector< int > stateInt1, stateInt2, stateVariant1, stateVariant2;
	
	auto intListener1 = [&](Resource::InvokeResultState s, QVariant v) {
		stateInt1.append (s);
		mapInt1.append (v.toMap ());
	};
	
	auto intListener2 = [&](Resource::InvokeResultState s, QVariant v) {
		stateInt2.append (s);
		mapInt2.append (v.toMap ());
	};
	
	auto variantListener1 = [&](Resource::InvokeResultState s, QVariant v) {
		stateVariant1.append (s);
		mapVariant1.append (v.toMap ());
	};
	
	auto variantListener2 = [&](Resource::InvokeResultState s, QVariant v) {
		stateVariant2.append (s);
		mapVariant2.append (v.toMap ());
	};
	
	// 
	Invocation connInt1 = this->wrapper->listen ("intArgument", intListener1);
	Invocation connInt2 = this->wrapper->listen ("intArgument", intListener2);
	Invocation connVariant1 = this->wrapper->listen ("variantArgument", variantListener1);
	
	emit this->object.intArgument (1);
	emit this->object.variantArgument ("a");
	
	Invocation connVariant2 = this->wrapper->listen ("variantArgument", variantListener2);
	
	emit this->object.intArgument (2);
	emit this->object.variantArgument ("b");
	
	connInt1.cancel ();
	
	emit this->object.intArgument (3);
	emit this->object.variantArgument ("c");
	
	connInt2.cancel ();
	
	emit this->object.intArgument (4);
	emit this->object.variantArgument ("d");
	
	connVariant2.cancel ();
	
	emit this->object.intArgument (5);
	emit this->object.variantArgument ("e");
	
	connVariant1.cancel ();
	
	emit this->object.intArgument (6);
	emit this->object.variantArgument ("f");
	
	// 
	QCOMPARE(stateInt1, QVector< int > ({ Resource::Success, Resource::Success  }));
	QCOMPARE(stateInt2, QVector< int > ({ Resource::Success, Resource::Success, Resource::Success }));
	QCOMPARE(stateVariant1, QVector< int > ({ Resource::Success, Resource::Success, Resource::Success,
	                                          Resource::Success, Resource::Success }));
	QCOMPARE(stateVariant2, QVector< int > ({ Resource::Success, Resource::Success, Resource::Success }));
	QCOMPARE(mapInt1, QVector< QVariantMap > ({ { { "count", 1 } }, { { "count", 2 } } }));
	QCOMPARE(mapInt2, QVector< QVariantMap > ({ { { "count", 1 } }, { { "count", 2 } }, { { "count", 3 } } }));
	QCOMPARE(mapVariant1, QVector< QVariantMap > ({ { { "variant", "a" } }, { { "variant", "b" } },
	                                                { { "variant", "c" } }, { { "variant", "d" } },
	                                                { { "variant", "e" } } }));
	QCOMPARE(mapVariant2, QVector< QVariantMap > ({ { { "variant", "b" } }, { { "variant", "c" } },
	                                                { { "variant", "d" } } }));
	
}

QTEST_MAIN(ObjectWrapperResourceTest)
#include "tst_objectwrapperresource.moc"
