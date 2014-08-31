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

using namespace Nuria;

class VariantTest : public QObject {
	Q_OBJECT
private slots:
	
	void verifyBuildList ();
	void verifyStealPointer ();
	void stealPointerReturnsNullptr ();
	
};

void VariantTest::verifyBuildList () {
	QVariantList expected { "test", 123, true };
	QVariantList result = Variant::buildList ("test", 123, true);
	
	QCOMPARE(result, expected);
}

void VariantTest::verifyStealPointer () {
	QVariant v = QVariant::fromValue< QObject * > (this);
	
	QCOMPARE(v.value< VariantTest * > (), this);
	QVERIFY(v.isValid ());
	QVERIFY(!v.isNull ());
	
	QCOMPARE(Variant::stealPointer (v), (void *)this);
	
	QCOMPARE(v.value< VariantTest * > (), (VariantTest *)nullptr);
	QVERIFY(!v.isValid ());
	QVERIFY(v.isNull ());
}

struct TestStruct { int a = 0; };
Q_DECLARE_METATYPE(TestStruct)

void VariantTest::stealPointerReturnsNullptr () {
	qRegisterMetaType< TestStruct > ();
	QVariant v = QVariant::fromValue (TestStruct ());
	
	QVERIFY(v.data ());
	QVERIFY(v.isValid ());
	QVERIFY(!v.isNull ());
	
	QVERIFY(!Variant::stealPointer (v));
	
	QVERIFY(v.data ());
	QVERIFY(v.isValid ());
	QVERIFY(!v.isNull ());
	
}

QTEST_MAIN(VariantTest)
#include "tst_variant.moc"
