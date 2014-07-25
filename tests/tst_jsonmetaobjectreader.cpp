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

#include <nuria/jsonmetaobjectreader.hpp>

#include <QtTest/QtTest>
#include <QMetaType>
#include <QObject>
#include <QDir>

class JsonMetaObjectReaderTest : public QObject {
	Q_OBJECT
	QMetaEnum m_errorEnum;
public:
	
	JsonMetaObjectReaderTest ();
	
private slots:
	
	void testRun_data ();
	void testRun ();
	
	void verifyResultOfNoErrorTest ();
	
};

JsonMetaObjectReaderTest::JsonMetaObjectReaderTest () {
	const QMetaObject *meta = &Nuria::JsonMetaObjectReader::staticMetaObject;
	int idx = meta->indexOfEnumerator ("Error");
	this->m_errorEnum = meta->enumerator (idx);
	
}

void JsonMetaObjectReaderTest::testRun_data () {
	QTest::addColumn< QString > ("file");
	QTest::addColumn< QByteArray > ("error");
	
	QDir resourceDir (":/resources");
	QStringList tests = resourceDir.entryList (QDir::Files);
	
	for (int i = 0; i < tests.length (); i++) {
		QByteArray errorName = tests.at (i).split (".").first ().toLatin1 ();
		QTest::newRow (errorName.constData ()) << tests.at (i) << errorName;
	}
	
}

static QByteArray readResourceFile (const QString &name) {
	QFile handle (":/resources/" + name);
	handle.open (QIODevice::ReadOnly);
	
	return handle.readAll ();
}

void JsonMetaObjectReaderTest::testRun () {
	Nuria::JsonMetaObjectReader::Error resultError;
	
	QFETCH(QString, file);
	QFETCH(QByteArray, error);
	
	QByteArray testCaseData = readResourceFile (file);
	
	// 
	Nuria::JsonMetaObjectReader reader;
	resultError = reader.parse (testCaseData);
	
	// 
	const char *resultErrorName = this->m_errorEnum.valueToKey (int (resultError));
	QCOMPARE(resultErrorName, error.constData ());
	
}

void JsonMetaObjectReaderTest::verifyResultOfNoErrorTest () {
	static QString headerFile ("foo.h");
	static QByteArray typeName ("Foo");
	
	QByteArray testCaseData = readResourceFile ("NoError.json");
	Nuria::JsonMetaObjectReader reader;
	
	QVERIFY(!testCaseData.isEmpty ());
	QCOMPARE(Nuria::JsonMetaObjectReader::NoError, reader.parse (testCaseData));
	
	// Check file object
	QStringList files = reader.sourceFiles ();
	QCOMPARE(1, files.length ());
	QCOMPARE(headerFile, files.first ());
	
	// Check types
	Nuria::MetaObjectMap metaObjects = reader.metaObjects (headerFile);
	Nuria::MetaObject *meta = metaObjects.value (typeName);
	QCOMPARE(1, metaObjects.size ());
	QVERIFY(metaObjects.contains (typeName));
	QVERIFY(meta);
	
	// Check the type itself.
	// 1. Name and counts of different things
	QCOMPARE(typeName, meta->className ());
	QCOMPARE(2, meta->parents ().size ());
	QCOMPARE(2, meta->annotationCount ());
	QCOMPARE(3, meta->methodCount ());
	QCOMPARE(1, meta->fieldCount ());
	QCOMPARE(1, meta->enumCount ());
	
	// 2. Bases
	QVector< QByteArray > bases = meta->parents ();
	QCOMPARE(QByteArray ("first"), bases.at (0));
	QCOMPARE(QByteArray ("second"), bases.at (1));
	
	// 3. Annotations
	QCOMPARE(QByteArray ("a"), meta->annotation (0).name ());
	QCOMPARE(QByteArray ("b"), meta->annotation (1).name ());
	QCOMPARE(int (QMetaType::Bool), meta->annotation (0).value ().userType ());
	QCOMPARE(true, meta->annotation (0).value ().toBool ());
	QCOMPARE(false, meta->annotation (1).value ().isValid ());
	
	// 4. Methods and their annotations
	Nuria::MetaMethod memberMethod = meta->method ({ "memberMethod", "QString" });
	Nuria::MetaMethod staticMethod = meta->method ({ "staticMethod", "int" });
	Nuria::MetaMethod constructor = meta->method ({ "", "QString" });
	QVERIFY(memberMethod.isValid ());
	QVERIFY(staticMethod.isValid ());
	QVERIFY(constructor.isValid ());
	
	// 4.1 Member methods
	QCOMPARE(QByteArray ("memberMethod"), memberMethod.name ());
	QCOMPARE(QByteArray ("int"), memberMethod.returnType ());
	QCOMPARE(QVector< QByteArray > { "a" }, memberMethod.argumentNames ());
	QCOMPARE(QVector< QByteArray > { "QString" }, memberMethod.argumentTypes ());
	QCOMPARE(1, memberMethod.annotationCount ());
	QCOMPARE(QByteArray ("name"), memberMethod.annotation (0).name ());
	QCOMPARE(QString ("memberMethod"), memberMethod.annotation (0).value ().toString ());
	
	// 4.2 Static methods
	QCOMPARE(QByteArray ("staticMethod"), staticMethod.name ());
	QCOMPARE(QByteArray ("int"), staticMethod.returnType ());
	QCOMPARE(QVector< QByteArray > { "a" }, staticMethod.argumentNames ());
	QCOMPARE(QVector< QByteArray > { "int" }, staticMethod.argumentTypes ());
	QCOMPARE(1, staticMethod.annotationCount ());
	QCOMPARE(QByteArray ("name"), staticMethod.annotation (0).name ());
	QCOMPARE(QString ("staticMethod"), staticMethod.annotation (0).value ().toString ());
	
	// 4.3 Constructors
	QCOMPARE(QByteArray (""), constructor.name ());
	QCOMPARE(typeName, constructor.returnType ());
	QCOMPARE(QVector< QByteArray > { "a" }, constructor.argumentNames ());
	QCOMPARE(QVector< QByteArray > { "QString" }, constructor.argumentTypes ());
	QCOMPARE(1, constructor.annotationCount ());
	QCOMPARE(QByteArray ("name"), constructor.annotation (0).name ());
	QCOMPARE(QString (""), constructor.annotation (0).value ().toString ());
	
	// 5. Enums
	Nuria::MetaEnum myEnum = meta->enumAt (0);
	QVERIFY(myEnum.isValid ());
	QCOMPARE(QByteArray ("MyEnum"), myEnum.name ());
	QCOMPARE(2, myEnum.elementCount ());
	QCOMPARE(QByteArray ("First"), myEnum.key (0));
	QCOMPARE(1, myEnum.value (0));
	QCOMPARE(QByteArray ("Second"), myEnum.key (1));
	QCOMPARE(2, myEnum.value (1));
	QCOMPARE(1, myEnum.annotationCount ());
	QCOMPARE(QByteArray ("isAEnum"), myEnum.annotation (0).name ());
	QCOMPARE(int (QMetaType::Bool), myEnum.annotation (0).value ().userType ());
	QCOMPARE(true, myEnum.annotation (0).value ().toBool ());
	
	// 6. Fields
	Nuria::MetaField myField = meta->field (0);
	QVERIFY(myField.isValid ());
	QCOMPARE(QByteArray ("myField"), myField.name ());
	QCOMPARE(QByteArray ("bool"), myField.typeName ());
	QCOMPARE(Nuria::MetaField::ReadOnly, myField.access ());
	QCOMPARE(1, myField.annotationCount ());
	QCOMPARE(QByteArray ("isAField"), myField.annotation (0).name ());
	QCOMPARE(int (QMetaType::Bool), myField.annotation (0).value ().userType ());
	QCOMPARE(true, myField.annotation (0).value ().toBool ());
	
}

QTEST_MAIN(JsonMetaObjectReaderTest)
#include "tst_jsonmetaobjectreader.moc"
