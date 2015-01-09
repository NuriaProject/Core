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

#include <nuria/serializer.hpp>

#include <QtTest/QtTest>
#include <QObject>

#include "structures.hpp"

using namespace Nuria;

class SerializerTest : public QObject {
	Q_OBJECT
public:
	
private slots:
	void serializeSimple ();
	void serializeComplex ();
	void serializeRecursive ();
	void serializeWithExclude ();
	void serializeWithCustomAllowedTypes ();
	void serializeWithFailedField ();
	void serializeWithNuriaConversion ();
	void serializeWithQtConversion ();
	void serializeWithCustomConverter ();
	
	void deserializeSimple ();
	void deserializeComplex ();
	void deserializeRecursive ();
	void deserializeWithExclude ();
	void deserializeWithFailedField ();
	void deserializeWithNuriaConversion ();
	void deserializeWithQtConversion ();
	void deserializeWithCustomConverter ();
	void deserializeUsingConstructor ();
	
};

void SerializerTest::serializeSimple () {
	QVariantMap expected { { "digit", 123 }, { "string", "hello" },
			       { "number", 12.34f }, { "boolean", true } };
	Simple simple;
	
	simple.digit = 123;
	simple.string = "hello";
	simple.number = 12.34f;
	simple.boolean = true;
	
	// 
	Serializer serializer;
	QVariantMap result = serializer.serialize (&simple, "Simple");
	
	QCOMPARE(result, expected);
}

void SerializerTest::serializeComplex () {
	QVariantMap expected {
		{ "simple",
			QVariantMap { { "digit", 123 }, { "string", "hello" },
				      { "number", 12.34f }, { "boolean", true } } },
		{ "outer", 42 }
	};
	
	Complex complex;
	complex.simple.digit = 123;
	complex.simple.string = "hello";
	complex.simple.number = 12.34f;
	complex.simple.boolean = true;
	complex.outer = 42;
	
	// 
	Serializer serializer;
	serializer.setRecursionDepth (Serializer::InfiniteRecursion);
	QVariantMap result = serializer.serialize (&complex, "Complex");
	
	QCOMPARE(result, expected);
}

void SerializerTest::serializeRecursive () {
	QVariantMap expected {
			{ "depth", 1 }, { "recurse", QVariantMap {
					{ "depth", 2 }, { "recurse", QVariantMap {
						{ "depth", 3 } 
					}
				}
			}
		}
	};
	
	// 
	Recursive recurse;
	recurse.recurse = new Recursive;
	recurse.recurse->recurse = new Recursive;
	recurse.recurse->recurse->recurse = new Recursive;
	recurse.depth = 1;
	recurse.recurse->depth = 2;
	recurse.recurse->recurse->depth = 3;
	recurse.recurse->recurse->recurse->depth = 4;
	
	// 
	Serializer serializer;
	serializer.setRecursionDepth (2);
	QVariantMap result = serializer.serialize (&recurse, "Recursive");
	
	QCOMPARE(result, expected);
}

void SerializerTest::serializeWithExclude () {
	QVariantMap expected { { "digit", 123 }, { "string", "hello" } };
	Simple simple;
	
	simple.digit = 123;
	simple.string = "hello";
	
	// 
	Serializer serializer;
	serializer.setExclude ({ "number", "boolean" });
	QVariantMap result = serializer.serialize (&simple, "Simple");
	
	QCOMPARE(result, expected);
}

void SerializerTest::serializeWithCustomAllowedTypes () {
	QDateTime theDateTime = QDateTime::currentDateTime ();
	QVariantMap expected { { "foo", 321 }, { "dateTime", theDateTime } };
	
	Custom custom;
	custom.foo = 321;
	custom.dateTime = theDateTime;
	
	// 
	Serializer serializer;
	serializer.setAllowedTypes (QVector< QByteArray > { "QDateTime" });
	QVariantMap result = serializer.serialize (&custom, "Custom");
	
	QCOMPARE(result["dateTime"].userType (), int (QMetaType::QDateTime));
	QCOMPARE(result, expected);
}

void SerializerTest::serializeWithFailedField () {
	QVariantMap expected { { "works", true } };
	
	Fail fail;
	fail.works = true;
	fail.someList.append (123);
	
	// 
	Serializer serializer;
	QVariantMap result = serializer.serialize (&fail, "Fail");
	
	QCOMPARE(result, expected);
	QCOMPARE(serializer.failedFields (), QStringList { "someList" });
}

void SerializerTest::serializeWithNuriaConversion () {
	QVariantMap expected { { "custom", "foo" } };
	
	WithCustomType withCustomType;
	QMetaType::registerConverter< SomeCustomType, QString > (&SomeCustomType::toString);
	
	// 
	Serializer serializer;
	QVariantMap result = serializer.serialize (&withCustomType, "WithCustomType");
	
	QCOMPARE(result, expected);
}

void SerializerTest::serializeWithQtConversion () {
	QDateTime theDateTime = QDateTime::currentDateTime ();
	
	QVariantMap expected { { "foo", 123 }, { "dateTime", theDateTime.toString (Qt::ISODate) } };
	
	Custom custom;
	custom.foo = 123;
	custom.dateTime = theDateTime;
	
	// 
	Serializer serializer;
	QVariantMap result = serializer.serialize (&custom, "Custom");
	
	QCOMPARE(result, expected);
}

static bool dateTimeToString (QVariant &v, int toType) {
	if (v.userType () != QMetaType::QDateTime) return Serializer::defaultValueConverter (v, toType);
	
	v = v.toDateTime ().toString (Qt::ISODate);
	qDebug("dateTimeToString %i %s", toType, qPrintable(v.toString ()));
	return true;
}

void SerializerTest::serializeWithCustomConverter () {
	QDateTime theDateTime (QDate (2014, 12, 20), QTime (21, 27, 39));
	QVariantMap expected { { "foo", 123 }, { "dateTime", theDateTime.toString (Qt::ISODate) } };
	
	Custom custom;
	custom.foo = 123;
	custom.dateTime = theDateTime;
	
	// 
	Serializer serializer (Serializer::defaultMetaObjectFinder, Serializer::defaultInstanceCreator,
	                       dateTimeToString);
	QTest::ignoreMessage (QtDebugMsg, "dateTimeToString 10 2014-12-20T21:27:39");
	QVariantMap result = serializer.serialize (&custom, "Custom");
	
	QCOMPARE(result, expected);
}

void SerializerTest::deserializeSimple () {
	QVariantMap data { { "digit", 123 }, { "string", "hello" },
			   { "number", 12.34f }, { "boolean", true } };
	
	// 
	Serializer serializer;
	Simple *result = serializer.deserialize< Simple > (data);
	
	QVERIFY(result);
	QCOMPARE(result->digit, 123);
	QCOMPARE(result->string, QString ("hello"));
	QCOMPARE(result->number, 12.34f);
	QCOMPARE(result->boolean, true);
	
	delete result;
}

void SerializerTest::deserializeComplex () {
	QVariantMap data { { "outer", 42 },
			   { "simple", QVariantMap {
				{ "digit", 123 }, { "string", "hello" },
				{ "number", 12.34f }, { "boolean", true } }
			   } };
	
	// 
	Serializer serializer;
	serializer.setRecursionDepth (Serializer::InfiniteRecursion);
	Complex *result = (Complex *)serializer.deserialize (data, "Complex");
	
	QVERIFY(result);
	QCOMPARE(result->outer, 42);
	QCOMPARE(result->simple.digit, 123);
	QCOMPARE(result->simple.string, QString ("hello"));
	QCOMPARE(result->simple.number, 12.34f);
	QCOMPARE(result->simple.boolean, true);
	
	delete result;
}

void SerializerTest::deserializeRecursive () {
	QVariantMap data {
		{ "depth", 1 }, { "recurse", QVariantMap {
				{ "depth", 2 }, { "recurse", QVariantMap {
						{ "depth", 3 }, { "recurse", QVariantMap {
								{ "depth", 4 } 
							}
						} 
					}
				}
			}
		}
	};
	
	// 
	Serializer serializer;
	serializer.setRecursionDepth (2);
	Recursive *recurse = (Recursive *)serializer.deserialize (data, "Recursive");
	
	QVERIFY(recurse);
	QVERIFY(recurse->recurse);
	QVERIFY(recurse->recurse->recurse);
	QVERIFY(!recurse->recurse->recurse->recurse);
	QCOMPARE(recurse->depth, 1);
	QCOMPARE(recurse->recurse->depth, 2);
	QCOMPARE(recurse->recurse->recurse->depth, 3);
	
	delete recurse;
}

void SerializerTest::deserializeWithExclude () {
	QVariantMap data { { "digit", 123 }, { "string", "hello" },
			   { "number", 12.34f }, { "boolean", true } };
	
	// 
	Serializer serializer;
	serializer.setExclude ({ "number", "boolean" });
	Simple *simple = (Simple *)serializer.deserialize (data, "Simple");
	
	QVERIFY(simple);
	QCOMPARE(simple->digit, 123);
	QCOMPARE(simple->string, QString ("hello"));
	QCOMPARE(simple->number, 0.f);
	QCOMPARE(simple->boolean, false);
	
	delete simple;
}

void SerializerTest::deserializeWithFailedField () {
	QVariantMap data { { "works", true }, { "someList", QVariantList { 1, 2, 3 } } };
	
	// 
	Serializer serializer;
	Fail *fail = (Fail *)serializer.deserialize (data, "Fail");
	
	QVERIFY(fail);
	QCOMPARE(fail->works, true);
	QVERIFY(fail->someList.isEmpty ());
	QCOMPARE(serializer.failedFields (), QStringList { "someList" });
	
	delete fail;
}

void SerializerTest::deserializeWithNuriaConversion () {
	QVariantMap data { { "custom", "foo" } };
	
	QMetaType::registerConverter< QString, SomeCustomType > (&SomeCustomType::fromString);
	QTest::ignoreMessage (QtDebugMsg, "fromString: foo");
	
	// 
	Serializer serializer;
	WithCustomType *custom = (WithCustomType *)serializer.deserialize (data, "WithCustomType");
	
	QVERIFY(custom);
	QVERIFY(custom->custom.works);
	
	delete custom;
}

void SerializerTest::deserializeWithQtConversion () {
	QDateTime theDateTime (QDate (2014, 04, 14), QTime (17, 03, 14));
	QVariantMap data { { "foo", 123 }, { "dateTime", theDateTime.toString (Qt::ISODate) } };
	
	// 
	Serializer serializer;
	Custom *custom = (Custom *)serializer.deserialize (data, "Custom");
	
	QVERIFY(custom);
	QCOMPARE(custom->foo, 123);
	QCOMPARE(custom->dateTime, theDateTime);
	
	delete custom;
}

static bool stringToBool (QVariant &v, int toType) {
	if (toType != QMetaType::Bool) return Serializer::defaultValueConverter (v, toType);
	
	qDebug("stringToBool: %s", qPrintable(v.toString ()));
	v = (v.toString () == "true");
	return true;
}

void SerializerTest::deserializeWithCustomConverter () {
	QVariantMap data { { "integer", "123" }, { "a", true }, { "b", "true" } };
	
	Serializer serializer (Serializer::defaultMetaObjectFinder, Serializer::defaultInstanceCreator, stringToBool);
	
	QTest::ignoreMessage (QtDebugMsg, "stringToBool: true");
	CustomConverter *conv = (CustomConverter *)serializer.deserialize (data, "CustomConverter");
	
	QVERIFY(conv);
	QCOMPARE(conv->integer, 123);
	QCOMPARE(conv->a, true);
	QCOMPARE(conv->b, true);
	
	delete conv;
}

void SerializerTest::deserializeUsingConstructor () {
	QVariantMap data { { "integer", 123 }, { "string", "foo" } };
	Serializer serializer;
	
	QTest::ignoreMessage (QtDebugMsg, "int 123");
	WithConstructor *constr = (WithConstructor *)serializer.deserialize (data, "WithConstructor");
	
	QVERIFY(constr);
	QCOMPARE(constr->integer, 123);
	QCOMPARE(constr->string, QString ("foo"));
	
	delete constr;
}

QTEST_MAIN(SerializerTest)
#include "tst_serializer.moc"
