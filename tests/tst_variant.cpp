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

#include <nuria/variant.hpp>

#include <QtTest/QtTest>
#include <QObject>
#include <QDate>

class VariantTest : public QObject {
	Q_OBJECT
public:
	VariantTest ();
	
private slots:
	
	void initTestCase ();
	
	// Comparison
	void compareFullyDeclaredType ();
	void compareType ();
	
	// Conversion
	void constructorConversion ();
	void customConversion ();
	
	// Iterator
	void invalidItemCount ();
	void genericItemCount ();
	void endEqualsEnd ();
	void iterateOverGeneric ();
	void iterateOverList ();
	void iterateOverMap ();
	void toList ();
	void toMap ();
	void variantForeachGeneric ();
	void variantForeachList ();
	void variantForeachMap ();
	
};

// 
VariantTest::VariantTest () {
	
}

void VariantTest::initTestCase () {
	typedef Nuria::Variant::Comparison C;
	typedef bool(QString::*Comp)(const QByteArray &) const;
	
	// === Comparisons ===
	// Register QString
	Nuria::Variant::registerType (qMetaTypeId< QString > (), qMetaTypeId< QByteArray > (),
				      (C)((Comp)&QString::operator==),
				      (C)((Comp)&QString::operator!=),
				      (C)((Comp)&QString::operator<),
				      (C)((Comp)&QString::operator>),
				      (C)((Comp)&QString::operator<=),
				      (C)((Comp)&QString::operator>=));
	
	// Register QDate
	Nuria::Variant::registerType< QDate > ();
	
}

void VariantTest::compareFullyDeclaredType () {
	using namespace Nuria;
	QString a ("a");
	QByteArray b ("b");
	
	QVariant va (a);
	QVariant vb (b);
	
	QVERIFY(Variant::canCompare (va, vb));
	QCOMPARE(Variant::equal (va, vb), a == b);
	QCOMPARE(Variant::notEqual (va, vb), a != b);
	QCOMPARE(Variant::lessThan (va, vb), a < b);
	QCOMPARE(Variant::greaterThan (va, vb), a > b);
	QCOMPARE(Variant::lessEqualThan (va, vb), a <= b);
	QCOMPARE(Variant::greaterEqualThan (va, vb), a >= b);
	
}

void VariantTest::compareType () {
	using namespace Nuria;
	QDate a (2000, 1, 1);
	QDate b (2010, 1, 1);
	
	QVariant va (a);
	QVariant vb (b);
	
	QVERIFY(Variant::canCompare (va, vb));
	QCOMPARE(Variant::equal (va, vb), a == b);
	QCOMPARE(Variant::notEqual (va, vb), a != b);
	QCOMPARE(Variant::lessThan (va, vb), a < b);
	QCOMPARE(Variant::greaterThan (va, vb), a > b);
	QCOMPARE(Variant::lessEqualThan (va, vb), a <= b);
	QCOMPARE(Variant::greaterEqualThan (va, vb), a >= b);
	
}

void VariantTest::constructorConversion () {
	Nuria::Variant::registerConversion< QByteArray, QString > ();
	
	QVariant variant (QByteArray ("Nuria"));
	QCOMPARE(Nuria::Variant::convert< QString > (variant), QVariant (QString ("Nuria")));
}

void VariantTest::customConversion () {
	Nuria::Variant::registerConversion< QDate, QString >
			([](const QDate &date) { return new QString (date.toString (Qt::ISODate)); });
	
	QVariant variant (QDate (2010, 1, 2));
	QVariant converted = Nuria::Variant::convert< QString > (variant);
	QCOMPARE(converted, QVariant (QString ("2010-01-02")));
	
}

void VariantTest::invalidItemCount () {
	QVariant variant;
	QCOMPARE(Nuria::Variant::itemCount (variant), 0);
}

void VariantTest::genericItemCount () {
	QVariant variant (123);
	QCOMPARE(Nuria::Variant::itemCount (variant), 1);
}

Q_DECLARE_METATYPE(QList<int>)
void VariantTest::endEqualsEnd () {
	Nuria::Variant::registerIterators< QList< int > > ();
	QList< int > list { 0, 1, 2, 3 };
	
	QVariant variant = QVariant::fromValue (list);
	Nuria::Variant::Iterator a = Nuria::Variant::end (variant);
	Nuria::Variant::Iterator b = Nuria::Variant::end (variant);
	
	QVERIFY(a.isValid ());
	QVERIFY(b.isValid ());
	QCOMPARE(a, b);
}

void VariantTest::iterateOverGeneric () {
	QVariant generic (123);
	
	QVERIFY(Nuria::Variant::isGeneric (generic));
	QCOMPARE(Nuria::Variant::itemCount (generic), 1);
	
	Nuria::Variant::Iterator it = Nuria::Variant::begin (generic);
	Nuria::Variant::Iterator end = Nuria::Variant::end (generic);
	
	int i;
	for (i = 0; it != end; ++it, i++) {
		if (i == 1) {
			QFAIL("Iterated over generic end");
		}
		
		QVariant cur = *it;
		QCOMPARE(cur.toInt (), generic.toInt ());
	}
	
	QCOMPARE(i, 1);
	
}

void VariantTest::iterateOverList () {
	Nuria::Variant::registerIterators< QList< int > > ();
	QList< int > list { 0, 1, 2, 3 };
	
	// 
	QVariant variant = QVariant::fromValue (list);
	QVERIFY(Nuria::Variant::isList (variant));
	QCOMPARE(Nuria::Variant::itemCount (variant), list.size ());
	
	// 
	Nuria::Variant::Iterator it = Nuria::Variant::begin (variant);
	Nuria::Variant::Iterator end = Nuria::Variant::end (variant);
	
	int i;
	for (i = 0; it != end; ++it, i++) {
		if (i == list.size ()) {
			QFAIL("Iterated over list end");
		}
		
		QVariant cur = *it;
		QCOMPARE(cur.toInt (), i);
	}
	
	QCOMPARE(i, list.size ());
}

typedef QMap< int, QString > MyMap;
Q_DECLARE_METATYPE(MyMap)
void VariantTest::iterateOverMap () {
	Nuria::Variant::registerIterators< MyMap > ();
	MyMap map {{ 1, "1" }, { 2, "2" }, { 3, "3" }};
	
	QVariant variant = QVariant::fromValue (map);
	QVERIFY(Nuria::Variant::isMap (variant));
	QCOMPARE(Nuria::Variant::itemCount (variant), map.count ());
	
	Nuria::Variant::Iterator it = Nuria::Variant::begin (variant);
	Nuria::Variant::Iterator end = Nuria::Variant::end (variant);
	
	int i;
	for (i = 0; it != end; ++it, i++) {
		if (i == map.count ()) {
			QFAIL("Iterated over list end");
		}
		
		QCOMPARE(it.key ().toString (), it.value ().toString ());
	}
	
	QCOMPARE(i, map.count ());
	
}

void VariantTest::toList () {
	Nuria::Variant::registerIterators< QList< int > > ();
	QList< int > list { 0, 1, 2, 3 };
	QVariantList expected { 0, 1, 2, 3 };
	
	// 
	QVariant variant = QVariant::fromValue (list);
	QVERIFY(Nuria::Variant::isList (variant));
	QCOMPARE(Nuria::Variant::itemCount (variant), list.size ());
	QCOMPARE(Nuria::Variant::toList (variant), expected);
	
}

void VariantTest::toMap () {
	Nuria::Variant::registerIterators< MyMap > ();
	MyMap map {{ 1, "one" }, { 2, "two" }, { 3, "three" }};
	QVariantMap expected {{ "1", "one" }, { "2", "two" }, { "3", "three" }};
	
	QVariant variant = QVariant::fromValue (map);
	QVERIFY(Nuria::Variant::isMap (variant));
	QCOMPARE(Nuria::Variant::itemCount (variant), map.count ());
	QCOMPARE(Nuria::Variant::toMap (variant), expected);
}

void VariantTest::variantForeachGeneric () {
	QVariant variant (123);
	
	int i = 0;
	QVariant cur;
	VARIANT_FOREACH(cur, variant) {
		QCOMPARE(cur, variant);
		i++;
	}
	
	QCOMPARE(i, 1);
}

void VariantTest::variantForeachList () {
	Nuria::Variant::registerIterators< QList< int > > ();
	
	QList< int > list { 0, 1, 2, 3 };
	QVariant variant = QVariant::fromValue (list);
	
	QVariant cur;
	int i = 0;
	VARIANT_FOREACH(cur, variant) {
		QCOMPARE(cur.toInt (), i);
		i++;
	}
	
	QCOMPARE(i, list.size ());
}

void VariantTest::variantForeachMap () {
	Nuria::Variant::registerIterators< MyMap > ();
	MyMap map {{ 0, "0" }, { 1, "1" }, { 2, "2" }};
	
	QVariant variant = QVariant::fromValue (map);
	
	QVariant cur;
	int i = 0;
	VARIANT_FOREACH(cur, variant) {
		QCOMPARE(cur.toInt (), i);
		i++;
	}
	
	QCOMPARE(i, map.size ());
}

QTEST_MAIN(VariantTest)
#include "tst_variant.moc"
