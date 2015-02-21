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

#include <nuria/jsonstreamreader.hpp>
#include <QtTest/QtTest>

using namespace Nuria;

class JsonStreamReaderTest : public QObject {
	Q_OBJECT
private slots:
	
	// More streaming behavious is tested in tst_streamingjsonhelper
	void verifyInitialState ();
	void verifyOneElement ();
	void verifyTwoElements ();
	void verifyPartialTransmission ();
	
	void clearStreamBufferDoesNotDiscardElements ();
	void discardReinitializesReader ();
	void verifyErrorBehaviour ();
	
	void jsonParserErrorDoesNotAffectReader ();
	
};

void JsonStreamReaderTest::verifyInitialState () {
	JsonStreamReader reader;
	
	QCOMPARE(reader.openMode (), QIODevice::WriteOnly);
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), false);
	QCOMPARE(reader.nextPendingElement ().isNull (), true);
	
}

void JsonStreamReaderTest::verifyOneElement () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	reader.write ("[\"]\",\"\\\"\",true,false]");
	
	QVERIFY(!reader.hasError ());
	QCOMPARE(error.length (), 0);
	QVERIFY(reader.hasPendingElement ());
	QCOMPARE(newPendingElement.length (), 1);
	
	QJsonParseError err;
	QVariantList list = reader.nextPendingElement (&err).toVariant ().toList ();
	
	QCOMPARE(err.error, QJsonParseError::NoError);
	QVERIFY(!reader.hasPendingElement ());
	QVERIFY(!reader.hasError ());
	QCOMPARE(list, QVariantList ({ "]", "\"", true, false }));
	
}

void JsonStreamReaderTest::verifyTwoElements () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	reader.write ("[\"]\",\"\\\"\",true,false]{\"foo}\":\"bar\"}");
	
	QVERIFY(!reader.hasError ());
	QCOMPARE(error.length (), 0);
	QVERIFY(reader.hasPendingElement ());
	QCOMPARE(newPendingElement.length (), 2);
	
	QJsonParseError err;
	QVariantList list = reader.nextPendingElement (&err).toVariant ().toList ();
	
	QCOMPARE(err.error, QJsonParseError::NoError);
	QVERIFY(reader.hasPendingElement ());
	QVERIFY(!reader.hasError ());
	QCOMPARE(list, QVariantList ({ "]", "\"", true, false }));
	
	QVariantMap map = reader.nextPendingElement (&err).toVariant ().toMap ();
	
	QCOMPARE(err.error, QJsonParseError::NoError);
	QVERIFY(!reader.hasPendingElement ());
	QVERIFY(!reader.hasError ());
	QCOMPARE(map, QVariantMap ({ { "foo}", "bar" } }));
	
}

void JsonStreamReaderTest::verifyPartialTransmission () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	reader.write ("[");
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), false);
	
	reader.write ("1,2,3");
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), false);
	
	reader.write ("]");
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), true);
	QCOMPARE(error.length (), 0);
	QCOMPARE(newPendingElement.length (), 1);
	
	QVariantList list = reader.nextPendingElement ().toVariant ().toList ();
	QCOMPARE(list, QVariantList ({ 1, 2, 3 }));
	
}

void JsonStreamReaderTest::clearStreamBufferDoesNotDiscardElements () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	reader.write ("[1,2,3]");
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), true);
	QCOMPARE(error.length (), 0);
	QCOMPARE(newPendingElement.length (), 1);
	
	reader.write ("\"garbage"); // Unclosed string
	reader.clearStreamBuffer (); // Clear
	
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), true);
	QCOMPARE(newPendingElement.length (), 1);
	
	reader.write ("[4]");
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), true);
	
	// 
	QCOMPARE(error.length (), 0);
	QCOMPARE(newPendingElement.length (), 2);
	
	QCOMPARE(reader.nextPendingElement ().toVariant ().toList (), QVariantList ({ 1, 2, 3 }));
	QCOMPARE(reader.hasPendingElement (), true);
	QCOMPARE(reader.nextPendingElement ().toVariant ().toList (), QVariantList ({ 4 }));
	QCOMPARE(reader.hasPendingElement (), false);
	
}

void JsonStreamReaderTest::discardReinitializesReader () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	// Bracket error
	reader.write ("[1,2,3]}");
	QCOMPARE(error.length (), 1);
	QCOMPARE(reader.hasError (), true);
	QCOMPARE(newPendingElement.length (), 1);
	QCOMPARE(reader.hasPendingElement (), true);
	
	// Discard
	reader.discard ();
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), false);
	QCOMPARE(reader.nextPendingElement ().isNull (), true);
	
}

void JsonStreamReaderTest::verifyErrorBehaviour () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	// Bracket error
	reader.write ("}");
	QCOMPARE(error.length (), 1);
	QCOMPARE(reader.hasError (), true);
	QCOMPARE(reader.hasPendingElement (), false);
	
	// Reset and try again
	reader.clearStreamBuffer ();
	QCOMPARE(reader.hasError (), false);
	
	reader.write ("[1,2,3]");
	QCOMPARE(error.length (), 1);
	QCOMPARE(newPendingElement.length (), 1);
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), true);
	
	// 
	QCOMPARE(reader.nextPendingElement ().toVariant ().toList (), QVariantList ({ 1, 2, 3 }));
	QCOMPARE(reader.hasPendingElement (), false);
	
}

void JsonStreamReaderTest::jsonParserErrorDoesNotAffectReader () {
	JsonStreamReader reader;
	QSignalSpy error (&reader, SIGNAL(error()));
	QSignalSpy newPendingElement (&reader, SIGNAL(newPendingElement()));
	
	// Json error
	reader.write ("{ 123: true }");
	QCOMPARE(error.length (), 0);
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(newPendingElement.length (), 1);
	QCOMPARE(reader.hasPendingElement (), true);
	
	QJsonParseError err;
	QVERIFY(reader.nextPendingElement (&err).isNull ());
	QVERIFY(err.error != QJsonParseError::NoError);
	
	QCOMPARE(error.length (), 0);
	QCOMPARE(reader.hasError (), false);
	QCOMPARE(reader.hasPendingElement (), false);
	
}

QTEST_MAIN(JsonStreamReaderTest)
#include "tst_jsonstreamreader.moc"
